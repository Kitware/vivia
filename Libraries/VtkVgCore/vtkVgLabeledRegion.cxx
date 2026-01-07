// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgLabeledRegion.h"

#include "vtkActor2D.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkInteractorObserver.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkTextActor.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"
#include "vtkTexture.h"
#include "vtkTexturedActor2D.h"
#include "vtkWindow.h"

#include <algorithm>

vtkStandardNewMacro(vtkVgLabeledRegion);

vtkCxxSetObjectMacro(vtkVgLabeledRegion, TextProperty, vtkTextProperty);
vtkCxxSetObjectMacro(vtkVgLabeledRegion, FrameProperty, vtkProperty);
vtkCxxSetObjectMacro(vtkVgLabeledRegion, Image, vtkImageData);
vtkCxxSetObjectMacro(vtkVgLabeledRegion, ImageProperty, vtkProperty2D);

//----------------------------------------------------------------------
vtkProp* vtkVgLabeledRegion::GetFrameActor() { return this->FrameActor; }
vtkProp* vtkVgLabeledRegion::GetImageActor() { return this->ImageActor; }
vtkProp* vtkVgLabeledRegion::GetTextActor()  { return this->TextActor; }

//----------------------------------------------------------------------
vtkVgLabeledRegion::vtkVgLabeledRegion()
{
  this->Visibility = 1;
  this->TextVisible = 0;
  this->ImageVisible = 0;

  this->Text = 0;
  this->Image = 0;

  // controlling layout
  this->Layout = LayoutTextToRightOfImage;
  this->Padding = 5;
  this->NormalizedLabelPosition[0] = 0.0;
  this->NormalizedLabelPosition[1] = 0.0;

  this->ImageSize[0] = 50;
  this->ImageSize[1] = 50;

  this->Texture = vtkTexture::New();
  this->TexturePoints = vtkPoints::New();
  this->TexturePolyData = vtkPolyData::New();

  this->TexturePoints->SetNumberOfPoints(4);
  this->TexturePolyData->SetPoints(this->TexturePoints);

  // create the quad for the image
  vtkCellArray* polys = vtkCellArray::New();
  vtkIdType ids[] = { 0, 1, 2, 3 };
  polys->InsertNextCell(4, ids);
  this->TexturePolyData->SetPolys(polys);
  polys->FastDelete();

  // setup texture coordinates
  vtkFloatArray* tc = vtkFloatArray::New();
  tc->SetNumberOfComponents(2);
  tc->SetNumberOfTuples(4);
  tc->InsertComponent(0, 0, 0.0);  tc->InsertComponent(0, 1, 0.0);
  tc->InsertComponent(1, 0, 1.0);  tc->InsertComponent(1, 1, 0.0);
  tc->InsertComponent(2, 0, 1.0);  tc->InsertComponent(2, 1, 1.0);
  tc->InsertComponent(3, 0, 0.0);  tc->InsertComponent(3, 1, 1.0);
  this->TexturePolyData->GetPointData()->SetTCoords(tc);
  tc->FastDelete();

  this->TextureMapper = vtkPolyDataMapper2D::New();
  this->TextureMapper->SetInputData(this->TexturePolyData);
  this->ImageProperty = vtkProperty2D::New();
  this->ImageProperty->SetOpacity(1.0);

  // image actor
  this->ImageActor = vtkTexturedActor2D::New();
  this->ImageActor->SetTexture(this->Texture);
  this->ImageActor->SetMapper(this->TextureMapper);
  this->ImageActor->SetProperty(this->ImageProperty);
  this->ImageActor->GetPositionCoordinate()->SetCoordinateSystemToWorld();

  // text actor
  this->TextProperty = vtkTextProperty::New();
  this->TextProperty->SetColor(0.8, 0.8, 0.8);
  this->TextProperty->SetFontSize(14);
  this->TextProperty->BoldOn();

  this->TextMapper = vtkTextMapper::New();
  this->TextMapper->SetTextProperty(this->TextProperty);

  this->TextActor = vtkActor2D::New();
  this->TextActor->SetMapper(this->TextMapper);

  // use a reference coordinate so that the text can be anchored
  // to a world position, but still use display positioning for layout
  vtkCoordinate* ref = vtkCoordinate::New();
  ref->SetCoordinateSystemToWorld();
  this->TextActor->GetPositionCoordinate()->SetReferenceCoordinate(ref);
  ref->FastDelete();

  // frame actor
  this->FrameProperty = vtkProperty::New();
  this->FrameProperty->SetColor(1, 1, .882);
  this->FrameProperty->SetOpacity(0.5);

  this->FramePolyData = vtkPolyData::New();

  this->FrameMapper = vtkPolyDataMapper::New();
  this->FrameMapper->SetInputData(this->FramePolyData);

  this->FrameActor = vtkActor::New();
  this->FrameActor->SetMapper(this->FrameMapper);
  this->FrameActor->SetProperty(this->FrameProperty);
}

//----------------------------------------------------------------------
vtkVgLabeledRegion::~vtkVgLabeledRegion()
{
  delete [] this->Text;

  if (this->Image)
    {
    this->Image->Delete();
    }

  this->FramePolyData->Delete();
  this->FrameMapper->Delete();
  this->FrameActor->Delete();
  this->FrameProperty->Delete();

  this->TextMapper->Delete();
  this->TextActor->Delete();
  this->TextProperty->Delete();

  this->Texture->Delete();
  this->TexturePolyData->Delete();
  this->TexturePoints->Delete();
  this->TextureMapper->Delete();
  this->ImageActor->Delete();
  this->ImageProperty->Delete();
}

//----------------------------------------------------------------------
void vtkVgLabeledRegion::SetFramePolyData(vtkPolyData* pd)
{
  if (pd == NULL || pd == this->FramePolyData)
    {
    return;
    }

  if (this->FramePolyData)
    {
    this->FramePolyData->UnRegister(this);
    }

  this->FramePolyData = pd;
  this->FrameMapper->SetInputData(pd);

  this->FramePolyData->Register(this);
  this->Modified();
}

//----------------------------------------------------------------------
inline void vtkVgLabeledRegion::AdjustImageSize(double imageSize[2])
{
  double r0 = this->ImageSize[0] / imageSize[0];
  double r1 = this->ImageSize[1] / imageSize[1];
  if (r0 > r1)
    {
    imageSize[0] *= r1;
    imageSize[1] *= r1;
    }
  else
    {
    imageSize[0] *= r0;
    imageSize[1] *= r0;
    }
}

//----------------------------------------------------------------------
void vtkVgLabeledRegion::Update(vtkViewport* v)
{
  this->UpdateTime.Modified();

  this->TextVisible = 0;
  this->ImageVisible = 0;

  // determine the size of the image
  double imageSize[] = { 0.0, 0.0 };
  if (this->Image)
    {
    if (this->Image->GetDataDimension() == 2)
      {
      imageSize[0] = this->Image->GetDimensions()[0];
      imageSize[1] = this->Image->GetDimensions()[1];
      this->ImageVisible = imageSize[0] > 0.0 && imageSize[1] > 0.0;
      this->AdjustImageSize(imageSize);
      }
    }

  // determine the size of the text
  int textSize[] = { 0, 0 };
  if (this->Text)
    {
    this->TextMapper->SetInput(this->Text);
    this->TextMapper->GetSize(v, textSize);
    this->TextVisible = textSize[0] > 0 && textSize[1] > 0;
    }

  // if there is no image or text, we are done
  if (!(this->TextVisible || this->ImageVisible))
    {
    return;
    }

  double bounds[6];
  this->FramePolyData->GetBounds(bounds);

  // get the min point of the bounding rectangle
  v->SetWorldPoint(bounds[0], bounds[2], 0.0, 1.0);
  v->WorldToDisplay();
  bounds[0] = v->GetDisplayPoint()[0];
  bounds[2] = v->GetDisplayPoint()[1];

  // get the max point of the bounding rectangle
  v->SetWorldPoint(bounds[1], bounds[3], 0.0, 1.0);
  v->WorldToDisplay();
  bounds[1] = v->GetDisplayPoint()[0];
  bounds[3] = v->GetDisplayPoint()[1];

  // compute layout
  double labelSize[2];
  double textOffset[2];
  double imageOffset[2];
  switch (this->Layout)
    {
    case LayoutTextToRightOfImage:
      labelSize[0] = this->TextVisible ? imageSize[0] + this->Padding + textSize[0]
                     : imageSize[0];
      labelSize[1] = std::max(imageSize[1], (double) textSize[1]);

      imageOffset[0] = -0.5 * labelSize[0];
      imageOffset[1] = -0.5 * imageSize[1];

      textOffset[0] = imageOffset[0] + imageSize[0] + this->Padding;
      textOffset[1] = -0.5 * textSize[1];
      break;

    case LayoutTextBelowImage:
    default:
      labelSize[0] = std::max(imageSize[0], (double) textSize[0]);
      labelSize[1] = this->TextVisible ? imageSize[1] + this->Padding + textSize[1]
                     : imageSize[1];

      textOffset[0] = -0.5 * textSize[0];
      textOffset[1] = -0.5 * labelSize[1];

      imageOffset[0] = -0.5 * imageSize[0];
      imageOffset[1] = this->TextVisible ? textOffset[1] + textSize[1] + this->Padding
                       : -0.5 * imageSize[1];
      break;
    }

  // compute the anchor position
  double pos[3];
  pos[0] = bounds[0] + this->NormalizedLabelPosition[0] * (bounds[1] - bounds[0]);
  pos[1] = bounds[2] + this->NormalizedLabelPosition[1] * (bounds[3] - bounds[2]);
  pos[2] = 0.0;

  // get anchor in world coords
  v->SetDisplayPoint(pos);
  v->DisplayToWorld();
  pos[0] = v->GetWorldPoint()[0];
  pos[1] = v->GetWorldPoint()[1];
  pos[2] = 0.5 * (bounds[4] + bounds[5]);

  this->TextActor->GetPositionCoordinate()->GetReferenceCoordinate()->SetValue(pos);
  this->ImageActor->GetPositionCoordinate()->SetValue(pos);

  // offset the text
  this->TextActor->SetPosition(textOffset);

  // offset the image
  if (this->ImageVisible)
    {
    double left = imageOffset[0];
    double right = left + imageSize[0];
    double bottom = imageOffset[1];
    double top = bottom + imageSize[1];
    this->Texture->SetInputData(this->Image);
    this->TexturePoints->SetPoint(0, left, bottom, 0.0);
    this->TexturePoints->SetPoint(1, right, bottom, 0.0);
    this->TexturePoints->SetPoint(2, right, top, 0.0);
    this->TexturePoints->SetPoint(3, left, top, 0.0);
    }
}

//----------------------------------------------------------------------
void vtkVgLabeledRegion::ReleaseGraphicsResources(vtkWindow* w)
{
  this->vtkProp::ReleaseGraphicsResources(w);
  this->FrameActor->ReleaseGraphicsResources(w);
  this->TextActor->ReleaseGraphicsResources(w);
  this->ImageActor->ReleaseGraphicsResources(w);
}

//----------------------------------------------------------------------
int vtkVgLabeledRegion::RenderOpaqueGeometry(vtkViewport* v)
{
  int rendered = 0;

  // update the frame polydata if necessary
  if (this->FramePolyData->GetMTime() > this->UpdateTime)
    {
    this->Modified();
    }

  // update the representation
  if (this->GetMTime() > this->UpdateTime)
    {
    this->Update(v);
    }

  if (this->FrameActor->RenderOpaqueGeometry(v))
    {
    rendered = 1;
    }

  if (this->ImageVisible && this->ImageActor->RenderOpaqueGeometry(v))
    {
    rendered = 1;
    }

  if (this->TextVisible && this->TextActor->RenderOpaqueGeometry(v))
    {
    rendered = 1;
    }

  return rendered;
}

//----------------------------------------------------------------------
int vtkVgLabeledRegion::RenderTranslucentPolygonalGeometry(vtkViewport* v)
{
  int rendered = 0;

  if (this->FrameActor->RenderTranslucentPolygonalGeometry(v))
    {
    rendered = 1;
    }

  if (this->ImageVisible && this->ImageActor->RenderTranslucentPolygonalGeometry(v))
    {
    rendered = 1;
    }

  if (this->TextVisible && this->TextActor->RenderTranslucentPolygonalGeometry(v))
    {
    rendered = 1;
    }

  return rendered;
}

//----------------------------------------------------------------------
int vtkVgLabeledRegion::RenderOverlay(vtkViewport* v)
{
  int rendered = 0;

  if (this->FrameActor->RenderOverlay(v))
    {
    rendered = 1;
    }

  if (this->ImageVisible && this->ImageActor->RenderOverlay(v))
    {
    rendered = 1;
    }

  if (this->TextVisible && this->TextActor->RenderOverlay(v))
    {
    rendered = 1;
    }

  return rendered;
}

//-----------------------------------------------------------------------------
int vtkVgLabeledRegion::HasTranslucentPolygonalGeometry()
{
  return this->FrameProperty->GetOpacity() < 1.0;
}

//----------------------------------------------------------------------
void vtkVgLabeledRegion::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "LabeledRegion Text: ";
  if (this->Text)
    {
    os << this->Text << "\n";
    }
  else
    {
    os << "(none)\n";
    }

  os << indent << "LabeledRegion Image: ";
  if (this->Image)
    {
    os << this->Image << "\n";
    }
  else
    {
    os << "(none)\n";
    }

  os << indent << "Image Size: (" << this->ImageSize[0] << ","
     << this->ImageSize[1] << ")\n";
  os << indent << "Padding: " << this->Padding << "\n";

  if (this->FrameProperty)
    {
    os << indent << "Frame Property:\n";
    this->FrameProperty->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent << "Frame Property: (none)\n";
    }

  if (this->ImageProperty)
    {
    os << indent << "Image Property:\n";
    this->ImageProperty->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent << "Image Property: (none)\n";
    }

  if (this->TextProperty)
    {
    os << indent << "Text Property:\n";
    this->TextProperty->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent << "Text Property: (none)\n";
    }
}
