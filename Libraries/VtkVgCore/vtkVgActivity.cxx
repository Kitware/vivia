/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgActivity.h"

// VTK includes.
#include <vtkAppendPolyData.h>
#include <vtkBitArray.h>
#include <vtkBoundingBox.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkExtractVOI.h>
#include <vtkIdList.h>
#include <vtkIdListCollection.h>
#include <vtkImageData.h>
#include <vtkImplicitModeller.h>
#include <vtkMarchingSquares.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkStripper.h>
#include <vtkTextProperty.h>

#include "vtkVgHull2D.h"
#include "vtkVgLabeledRegion.h"
#include "vtkVgTriangulateConcavePolysFilter.h"
#include "vtkVgEvent.h"
#include "vtkVgIconManager.h"

#include <algorithm>
#include <assert.h>

vtkStandardNewMacro(vtkVgActivity);

vtkCxxSetObjectMacro(vtkVgActivity, IconManager, vtkVgIconManager);

//-----------------------------------------------------------------------------
class vtkVgActivity::vtkInternal
{
public:
  std::vector<vtkVgEvent*> Events;
  vtkBoundingBox BoundingBox;
  vtkPolyData* RenderPolyData;

  vtkSmartPointer<vtkRenderer> Renderer;

  vtkInternal()
    {
    this->RenderPolyData = 0;
    }

  ~vtkInternal()
    {
    if (this->RenderPolyData)
      {
      this->RenderPolyData->Delete();
      }
    }
};

//-----------------------------------------------------------------------------
vtkVgActivity::vtkVgActivity()
{
  this->Name            =  0;
  this->Type            = -1;
  this->Id              = -1;
  this->IconIndex       = -1;
  this->Status = vgObjectStatus::None;
  this->Saliency = -1.0;
  this->Probability = -1.0;

  this->Actor = 0;
  this->IconManager = 0;
  this->ActivityFrameExtents[0].SetToMaxTime();
  this->ActivityFrameExtents[1].SetToMinTime();

  // initalize SpatialBounds to invalid bounds
  this->SpatialBounds[0] = this->SpatialBounds[2] = 1;
  this->SpatialBounds[1] = this->SpatialBounds[3] = -1;

  this->BoundsMode = false;

  this->Internal = new vtkInternal;

  this->Color[0] = this->Color[1] = this->Color[2] = 1.0;

  this->SecondaryColor[0] = this->SecondaryColor[1]
                            = this->SecondaryColor[2] = 0.0;

  this->MultiColorEnabled = false;
  this->ShowAlways = false;

  this->ExpirationOffset.SetTime(0);
  this->ExpirationOffset.SetFrameNumber(0);
}

//-----------------------------------------------------------------------------
vtkVgActivity::~vtkVgActivity()
{
  if (this->Actor)
    {
    this->GetRenderer()->RemoveViewProp(this->Actor);
    this->Actor->Delete();
    }
  if (this->IconManager)
    {
    this->IconManager->Delete();
    }

  delete[] this->Name;
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgActivity::SetRenderer(vtkRenderer* ren)
{
  this->Internal->Renderer = ren;
}

//-----------------------------------------------------------------------------
vtkRenderer* vtkVgActivity::GetRenderer()
{
  return this->Internal->Renderer;
}

//-----------------------------------------------------------------------------
void vtkVgActivity::SetOverlayOpacity(double opacity)
{
  this->OverlayOpacity = opacity;

  if (this->Actor)
    {
    this->Actor->GetFrameProperty()->SetOpacity(opacity);
    }
}

//-----------------------------------------------------------------------------
void vtkVgActivity::SetVisibility(bool visible)
{
  if (this->Actor)
    {
    this->Actor->SetVisibility(visible);
    }
}

//-----------------------------------------------------------------------------
bool vtkVgActivity::SetCurrentDisplayFrame(const vtkVgTimeStamp& timeStamp)
{
  this->CurrentDisplayFrame = timeStamp;

  vtkVgTimeStamp endFrame = this->ActivityFrameExtents[1];
  endFrame.ShiftForward(this->ExpirationOffset);

  if (!this->ShowAlways &&
      (this->ActivityFrameExtents[0] > this->CurrentDisplayFrame ||
       endFrame < this->CurrentDisplayFrame))
    {
    if (this->Actor)
      {
      this->GetRenderer()->RemoveViewProp(this->Actor);
      this->Actor->Delete();
      this->Actor = NULL;
      }
    return false;
    }

  if (!this->Actor)
    {
    if (!this->Internal->RenderPolyData)
      {
      this->PrepareRepresentation();
      }

    this->Actor = vtkVgLabeledRegion::New();

    this->Actor->SetFramePolyData(this->Internal->RenderPolyData);
    this->Actor->SetImageSize(32, 32);
    this->Actor->SetNormalizedLabelPosition(0.5, 0.5);
    this->Actor->GetFrameProperty()->SetOpacity(this->OverlayOpacity);
    this->Actor->GetFrameProperty()->SetLineWidth(3);
    this->Actor->GetFrameProperty()->SetColor(this->Color);
    this->Actor->GetTextProperty()->SetColor(1.0, 1.0, 1.0);

    // @TODO: Adding an actor to the renderer here is a quick hack
    //        to get things working - revisit this.
    this->GetRenderer()->AddViewProp(this->Actor);
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
void vtkVgActivity::PrepareRepresentation()
{
  if (!this->Internal->RenderPolyData)
    {
    this->Internal->RenderPolyData = vtkPolyData::New();
    }

  if (this->BoundsMode)
    {
    // do nothing if SpatialBounds aren't valid
    if (this->SpatialBounds[1] < this->SpatialBounds[0] ||
        this->SpatialBounds[3] < this->SpatialBounds[2])
      {
      return;
      }

    // points
    vtkPoints* points = vtkPoints::New();
    this->Internal->RenderPolyData->SetPoints(points);
    points->FastDelete();
    points->SetNumberOfPoints(4);
    points->SetPoint(0, this->SpatialBounds[0], this->SpatialBounds[2], 0.0);
    points->SetPoint(1, this->SpatialBounds[1], this->SpatialBounds[2], 0.0);
    points->SetPoint(2, this->SpatialBounds[1], this->SpatialBounds[3], 0.0);
    points->SetPoint(3, this->SpatialBounds[0], this->SpatialBounds[3], 0.0);

    // and then the poly
    vtkCellArray* polys = vtkCellArray::New();
    this->Internal->RenderPolyData->SetPolys(polys);
    polys->FastDelete();
    polys->Allocate(5);
    polys->InsertNextCell(4);
    polys->InsertCellPoint(0);
    polys->InsertCellPoint(1);
    polys->InsertCellPoint(2);
    polys->InsertCellPoint(3);

    // now add the required color info; ugly for now (rather not duplicate code
    // from below)
    vtkUnsignedCharArray* uca = vtkUnsignedCharArray::New();
    uca->SetNumberOfComponents(3);
    uca->SetName("Activity Color");

    vtkBitArray* ba = vtkBitArray::New();
    ba->SetName("Use Secondary Color");

    this->Internal->RenderPolyData->GetCellData()->SetScalars(uca);
    this->Internal->RenderPolyData->GetCellData()->SetPedigreeIds(ba);
    uca->FastDelete();
    ba->FastDelete();

    uca->InsertNextTuple3(this->Color[0] * 255,
                          this->Color[1] * 255,
                          this->Color[2] * 255);
    ba->InsertNextValue(0);
    return;
    }

  // else
  vtkPolyData* pd[] = { vtkPolyData::New(), vtkPolyData::New() };

  // When using multi-colored activities, it is necessary to build the polydata
  // in two phases - first for the primary event, then for all other events.
  // Since vtkImplicitModeller does not preserve scalar data, there is no other
  // way to assign different colors to different events within the activity.
  for (int phase = 0; phase < 2; ++phase)
    {
    // add verts to supprt single pt events
    vtkCellArray* verts = vtkCellArray::New();
    pd[phase]->SetVerts(verts);
    verts->FastDelete();

    vtkCellArray* lines = vtkCellArray::New();
    pd[phase]->SetLines(lines);
    lines->FastDelete();

    std::vector<vtkVgEvent*>::const_iterator eventIter;
    for (eventIter = this->Internal->Events.begin();
         eventIter != this->Internal->Events.end(); eventIter++)
      {
      if (eventIter == this->Internal->Events.begin())
        {
        pd[phase]->SetPoints((*eventIter)->GetPoints());

        // skip first event if we're building data for secondary events
        if (phase == 1)
          {
          continue;
          }
        }

      vtkIdListCollection* idCollection =
        (*eventIter)->GetFullEventIdCollection();
      for (vtkIdType i = 0; i < idCollection->GetNumberOfItems(); i++)
        {
        if (idCollection->GetItem(i)->GetNumberOfIds() == 1)
          {
          verts->InsertNextCell(idCollection->GetItem(i));
          }
        else
          {
          lines->InsertNextCell(idCollection->GetItem(i));
          }
        }

      // only build data for primary event in phase 0
      if (phase == 0)
        {
        break;
        }
      }

    pd[phase]->Modified();
    }

  // need to clean these calculations some, but will do for now
  double maxDistance = 20.0;
  vtkBoundingBox bb;
  for (int i = 0; i < 2; ++i)
    {
    bb.AddBounds(pd[i]->GetBounds());
    }
  double bounds[6];
  bb.GetBounds(bounds);
  // make sure min/max bounds are not equal, as calulcations break down ankd
  // error generated by the implicit modeller if they are
  if (bounds[0] == bounds[1])
    {
    bounds[0] -= 0.5;
    bounds[1] += 0.5;
    }
  if (bounds[2] == bounds[3])
    {
    bounds[2] -= 0.5;
    bounds[3] += 0.5;
    }
  double modelBounds[6] =
    {
    bounds[0], bounds[1], bounds[2], bounds[3],
    bounds[4] - maxDistance * 2.0,
    bounds[5] + maxDistance * 2.0
    };

  int sampleDimensions[3] = { 0, 0, 3 };
  double maxSpacing = 0;
  for (int i = 0; i < 2; i++)
    {
    sampleDimensions[i] =
      2 * ceil((bounds[i * 2 + 1] - bounds[i * 2]) / maxDistance);
    if (sampleDimensions[i] < 3)
      {
      sampleDimensions[i] = 3;
      }

    // to make sure we "represent" the max distance at the extents, extend
    // bounds and add samples
    double sampleSpacing = (bounds[i * 2 + 1] - bounds[i * 2]) /
                           (sampleDimensions[i] - 1);
    if (sampleSpacing > maxSpacing)
      {
      maxSpacing = sampleSpacing;
      }
    modelBounds[i * 2] -= sampleSpacing * 2;
    modelBounds[i * 2 + 1] += sampleSpacing * 2;
    sampleDimensions[i] += 4 * 2;
    }

  vtkAppendPolyData* append = vtkAppendPolyData::New();

  for (int phase = 0; phase < 2; ++phase)
    {
    // compute a distance map of the component events
    vtkImplicitModeller* implicitModeller = vtkImplicitModeller::New();
    implicitModeller->SetInputData(pd[phase]);
    implicitModeller->SetSampleDimensions(sampleDimensions);
    implicitModeller->SetMaximumDistance(maxDistance * 1.05);
    implicitModeller->SetModelBounds(modelBounds);
    implicitModeller->SetOutputScalarTypeToFloat();
    implicitModeller->SetProcessModeToPerCell();
    implicitModeller->CappingOn();
    implicitModeller->SetCapValue(maxDistance + maxSpacing);
    implicitModeller->Update();

    pd[phase]->FastDelete();

    // extract our z-slice of interest (the middle one); implicit modeller requires
    // a volume, which is why we didn't make z sample dimension 1.  We made it 3 so
    // that the middle slice would be exactly what we want.
    vtkExtractVOI* extractSlice = vtkExtractVOI::New();
    extractSlice->SetInputConnection(implicitModeller->GetOutputPort());
    extractSlice->SetVOI(0, sampleDimensions[0] - 1, 0, sampleDimensions[1] - 1, 1, 1);
    implicitModeller->FastDelete();

    // finally, contour
    vtkMarchingSquares* contour = vtkMarchingSquares::New();
    contour->SetInputConnection(extractSlice->GetOutputPort());
    extractSlice->FastDelete();
    contour->SetValue(0, maxDistance);

    // largest allowed value by the stripper, but shouldn't be a problem
    vtkStripper* stripper = vtkStripper::New();
    stripper->SetMaximumLength(100000);
    stripper->SetInputConnection(contour->GetOutputPort());
    contour->FastDelete();

    stripper->Update();
    vtkPolyData* pd = stripper->GetOutput();

    vtkUnsignedCharArray* uca = vtkUnsignedCharArray::New();
    uca->SetNumberOfComponents(3);
    uca->SetName("Activity Color");

    vtkBitArray* ba = vtkBitArray::New();
    ba->SetName("Use Secondary Color");

    pd->GetCellData()->SetScalars(uca);
    pd->GetCellData()->SetPedigreeIds(ba);
    uca->FastDelete();
    ba->FastDelete();

    bool useSecondaryColor = this->GetMultiColorEnabled();

    // add scalar data for colors
    for (vtkIdType i = 0; i < pd->GetNumberOfCells(); ++i)
      {
      unsigned char color[3];
      if (useSecondaryColor && phase > 0)
        {
        color[0] = this->SecondaryColor[0] * 255;
        color[1] = this->SecondaryColor[1] * 255;
        color[2] = this->SecondaryColor[2] * 255;
        }
      else
        {
        color[0] = this->Color[0] * 255;
        color[1] = this->Color[1] * 255;
        color[2] = this->Color[2] * 255;
        }
      uca->InsertNextTupleValue(color);
      ba->InsertNextValue(phase > 0);
      }

    append->AddInputConnection(stripper->GetOutputPort());
    stripper->FastDelete();
    }

  append->Update();
  vtkPolyData* appended = append->GetOutput();

  vtkPolyData* polygons = vtkPolyData::New();
  polygons->SetPoints(appended->GetPoints());

  vtkCellArray* polys = vtkCellArray::New();
  polygons->SetPolys(polys);
  polys->FastDelete();

  // should be sufficient as long as points aren't reused (and we don't expect
  // them to be reused)
  polys->Allocate(appended->GetNumberOfPoints() + appended->GetNumberOfLines());

  vtkCellArray* lines = appended->GetLines();
  lines->InitTraversal();
  vtkIdType npts, *pts;
  while (lines->GetNextCell(npts, pts))
    {
    // make sure it is closed
    if (pts[0] != pts[npts - 1])
      {
      continue;
      }
    polys->InsertNextCell(npts - 1, pts);
    }

  // copy color info
  polygons->GetCellData()->SetScalars(appended->GetCellData()->GetScalars());
  polygons->GetCellData()->SetPedigreeIds(appended->GetCellData()->GetPedigreeIds());

  append->Delete();

  vtkVgTriangulateConcavePolysFilter* concavePolys =
    vtkVgTriangulateConcavePolysFilter::New();
  concavePolys->SetInputData(polygons);
  polygons->FastDelete();
  concavePolys->Update();

  this->Internal->RenderPolyData->DeepCopy(concavePolys->GetOutput());
  concavePolys->Delete();
}

//-----------------------------------------------------------------------------
void vtkVgActivity::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgActivity::AddEvent(vtkVgEvent* theEvent)
{
  if (!theEvent)
    {
    vtkErrorMacro("Invalid event.\n");
    return;
    }

  this->Internal->Events.push_back(theEvent);

  if (theEvent->GetStartFrame() < this->ActivityFrameExtents[0])
    {
    this->ActivityFrameExtents[0] = theEvent->GetStartFrame();
    }
  if (this->ActivityFrameExtents[1] < theEvent->GetEndFrame())
    {
    this->ActivityFrameExtents[1] = theEvent->GetEndFrame();
    }

  double bounds[6] = {0, 0, 0, 0, 0, 0};
  theEvent->GetFullBounds(bounds);
  this->Internal->BoundingBox.AddBounds(bounds);
  this->Internal->BoundingBox.GetBounds(bounds);

  // rebuild the representation
  if (this->Actor)
    {
    this->PrepareRepresentation();
    }
  else if (this->Internal->RenderPolyData)
    {
    this->Internal->RenderPolyData->Delete();
    this->Internal->RenderPolyData = 0;
    }
}

//-----------------------------------------------------------------------------
void vtkVgActivity::RemoveEvent(unsigned int index)
{
  this->Internal->Events.erase(this->Internal->Events.begin() + index);
  this->UpdateFrameExtentsAndBounds();

  // rebuild the representation
  if (this->Actor)
    {
    this->PrepareRepresentation();
    }
  else if (this->Internal->RenderPolyData)
    {
    this->Internal->RenderPolyData->Delete();
    this->Internal->RenderPolyData = 0;
    }
}

//-----------------------------------------------------------------------------
unsigned int vtkVgActivity::GetNumberOfEvents()
{
  return static_cast<unsigned int>(this->Internal->Events.size());
}

//-----------------------------------------------------------------------------
void vtkVgActivity::UpdateFrameExtentsAndBounds()
{
  if (this->GetNumberOfEvents() == 0)
    {
    return;
    }

  vtkVgEvent* event0 = this->Internal->Events[0];

  vtkVgTimeStamp minStartFrame = event0->GetStartFrame();
  vtkVgTimeStamp maxEndFrame = event0->GetEndFrame();

  double bounds[6] = { 0.0 };
  event0->GetFullBounds(bounds);

  this->Internal->BoundingBox.Reset();
  this->Internal->BoundingBox.AddBounds(bounds);

  for (unsigned int i = 1, end = this->GetNumberOfEvents(); i < end; ++i)
    {
    vtkVgEvent* theEvent = this->Internal->Events[i];
    minStartFrame = std::min(minStartFrame, theEvent->GetStartFrame());
    maxEndFrame = std::max(maxEndFrame, theEvent->GetEndFrame());
    theEvent->GetFullBounds(bounds);
    this->Internal->BoundingBox.AddBounds(bounds);
    }

  this->ActivityFrameExtents[0] = minStartFrame;
  this->ActivityFrameExtents[1] = maxEndFrame;

  this->Internal->BoundingBox.GetBounds(bounds);
}

//-----------------------------------------------------------------------------
vtkVgEvent* vtkVgActivity::GetEvent(unsigned int index)
{
  if (index >= this->Internal->Events.size())
    {
    vtkErrorMacro("Index out of range.\n");
    return 0;
    }

  return this->Internal->Events[index];
}

//-----------------------------------------------------------------------------
void vtkVgActivity::SetShowLabel(bool enable)
{
  if (!this->Actor)
    {
    return;
    }

  if (enable)
    {
    if (this->IconManager->GetIconSheetFileName())
      {
      this->Actor->SetImage(this->IconManager->GetIconImage(this->IconIndex));
      }
    this->Actor->SetText(this->Name);
    }
  else
    {
    this->Actor->SetImage(0);
    this->Actor->SetText(0);
    }
}

//-----------------------------------------------------------------------------
void vtkVgActivity::ApplyColors()
{
  if (!this->Internal->RenderPolyData)
    {
    return;
    }

  vtkBitArray* ba =
    vtkBitArray::SafeDownCast(
      this->Internal->RenderPolyData->GetCellData()->GetPedigreeIds());

  vtkUnsignedCharArray* uca =
    vtkUnsignedCharArray::SafeDownCast(
      this->Internal->RenderPolyData->GetCellData()->GetScalars());

  assert(ba->GetNumberOfTuples() == uca->GetNumberOfTuples());
  bool useSecondary = this->GetMultiColorEnabled();

  // Update the color scalars in the render polydata.
  for (vtkIdType i = 0, end = ba->GetNumberOfTuples(); i < end; ++i)
    {
    unsigned char color[3];
    if (useSecondary && ba->GetValue(i))
      {
      color[0] = this->SecondaryColor[0] * 255;
      color[1] = this->SecondaryColor[1] * 255;
      color[2] = this->SecondaryColor[2] * 255;
      }
    else
      {
      color[0] = this->Color[0] * 255;
      color[1] = this->Color[1] * 255;
      color[2] = this->Color[2] * 255;
      }
    uca->SetTupleValue(i, color);
    }

  this->Internal->RenderPolyData->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgActivity::SetExpirationOffset(const vtkVgTimeStamp& timeStamp)
{
  this->ExpirationOffset = timeStamp;
  this->Modified();
}
