// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgEventRegionRepresentation.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkIdTypeArray.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataCollection.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkUnsignedCharArray.h>

#include "vgEventType.h"

#include "vtkVgAnnotationActor.h"
#include "vtkVgCellPicker.h"
#include "vtkVgEvent.h"
#include "vtkVgEventFilter.h"
#include "vtkVgEventModel.h"
#include "vtkVgEventTypeRegistry.h"
#include "vtkVgPickData.h"
#include "vtkVgPicker.h"

#include <set>

vtkStandardNewMacro(vtkVgEventRegionRepresentation);

//----------------------------------------------------------------------------
struct vtkVgEventRegionRepresentation::vtkInternal
{
public:
  vtkActor* RegionActor;
  vtkPolyData* PolyData;
  vtkCellArray* Lines;
  vtkIdTypeArray* EventIds;
  vtkUnsignedCharArray* EventColorArray;
  vtkTransform* RegionTransform;

  vtkInternal()
    {
    this->PolyData = vtkPolyData::New();
    this->Lines = vtkCellArray::New();
    this->PolyData->SetLines(this->Lines);
    this->Lines->FastDelete();
    this->EventIds = vtkIdTypeArray::New();
    this->EventIds->SetName("Event Ids");
    this->PolyData->GetCellData()->AddArray(this->EventIds);
    this->EventIds->FastDelete();

    this->EventColorArray = vtkUnsignedCharArray::New();
    this->EventColorArray->SetNumberOfComponents(4);
    this->EventColorArray->SetName("Event Color");
    this->PolyData->GetCellData()->SetScalars(this->EventColorArray);
    this->EventColorArray->FastDelete();

    vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
    mapper->SetInputData(this->PolyData);
    this->RegionActor = vtkActor::New();
    this->RegionActor->SetMapper(mapper);
    this->RegionActor->GetProperty()->SetColor(1, 0, 0);  // temporary hardcode to a color
    this->RegionActor->GetProperty()->SetLineWidth(3);

    this->RegionTransform = vtkTransform::New();
    this->RegionActor->SetUserTransform(this->RegionTransform);
    this->RegionTransform->FastDelete();

    mapper->FastDelete();
    }

  ~vtkInternal()
    {
    this->PolyData->Delete();
    this->RegionActor->Delete();
    }
};

//-----------------------------------------------------------------------------
vtkVgEventRegionRepresentation::vtkVgEventRegionRepresentation()
{
  this->Visible = 1;
  this->Internal = new vtkInternal;

  this->NewPropCollection    = vtkPropCollectionRef::New();
  this->ActivePropCollection = vtkPropCollectionRef::New();
  this->ExpirePropCollection = vtkPropCollectionRef::New();

  this->NewPropCollection->AddItem(this->Internal->RegionActor);
  this->ActivePropCollection->AddItem(this->Internal->RegionActor);

  this->Picker = vtkSmartPointer<vtkVgCellPicker>::New();
  this->Picker->PickFromListOn();
  this->Picker->SetTolerance(0.001);

  this->RegionZOffset = 1.0;
}

//-----------------------------------------------------------------------------
vtkVgEventRegionRepresentation::~vtkVgEventRegionRepresentation()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgEventRegionRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgEventRegionRepresentation::GetNewRenderObjects() const
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgEventRegionRepresentation::GetActiveRenderObjects() const
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgEventRegionRepresentation::GetExpiredRenderObjects() const
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgEventRegionRepresentation::GetNewRenderObjects()
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgEventRegionRepresentation::GetActiveRenderObjects()
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgEventRegionRepresentation::GetExpiredRenderObjects()
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
void vtkVgEventRegionRepresentation::ResetTemporaryRenderObjects()
{
  this->NewPropCollection->RemoveAllItems();
  this->ExpirePropCollection->RemoveAllItems();
}

//-----------------------------------------------------------------------------
float vtkVgEventRegionRepresentation::GetLineWidth() const
{
  return this->Internal->RegionActor->GetProperty()->GetLineWidth();
}

//-----------------------------------------------------------------------------
void vtkVgEventRegionRepresentation::SetLineWidth(float value)
{
  this->Internal->RegionActor->GetProperty()->SetLineWidth(value);
}

//-----------------------------------------------------------------------------
void vtkVgEventRegionRepresentation::SetVisible(int flag)
{
  if (flag == this->Visible)
    {
    return;
    }
  this->Internal->RegionActor->SetVisibility(
    flag && this->Internal->PolyData->GetNumberOfCells());
  this->Visible = flag;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventRegionRepresentation::Update()
{
  if (!this->GetVisible())
    {
    return;
    }

  this->Internal->Lines->Reset();
  this->Internal->EventIds->Reset();
  this->Internal->EventColorArray->Reset();

  // not the best place for the allocate, but it doesn't do anything unless
  // the number of events has increased.
  this->Internal->EventIds->Allocate(this->EventModel->GetNumberOfEvents());

  // should do this somehwere else once...
  this->Internal->PolyData->SetPoints(this->EventModel->GetSharedRegionPoints());

  this->EventModel->InitEventTraversal();
  while (vtkVgEvent* theEvent =
           this->EventModel->GetNextDisplayedEvent().GetEvent())
    {
    // is event masked in this representation?
    unsigned int mask = this->GetDisplayMask();
    if ((mask & theEvent->GetDisplayFlags()) != mask)
      {
      continue;
      }

    // don't display if this event is being filtered out
    int eventType = theEvent->GetActiveClassifierType();
    if (this->EventFilter &&
        this->EventFilter->GetBestClassifier(theEvent) == -1)
      {
      continue;
      }

    // if we don't recognize the active type of the event, ignore it
    //int eventType = theEvent->GetActiveClassifierType();
    //if (this->Internal->EventActor.find(eventType) ==
    //    this->Internal->EventActor.end())
    //  {
    //  continue;
    //  }

    // When "display full volume" is used for a region-based event, show the
    // first, last, or current region as appropriate. Don't attempt to show
    // every region.
    vtkIdType npts, *pts;
    if (this->EventModel->GetShowEventsBeforeStart() &&
        this->EventModel->GetCurrentTimeStamp() < theEvent->GetStartFrame())
      {
      vtkVgTimeStamp startTime =  theEvent->GetStartFrame();
      theEvent->GetRegionAtOrAfter(startTime, npts, pts);
      }
    else if (this->EventModel->GetShowEventsAfterExpiration() &&
             this->EventModel->GetCurrentTimeStamp() > theEvent->GetEndFrame())
      {
      vtkVgTimeStamp endTime =  theEvent->GetStartFrame();
      theEvent->GetRegionAtOrAfter(endTime, npts, pts);
      }
    else
      {
      vtkVgTimeStamp now = this->EventModel->GetCurrentTimeStamp();
      if (theEvent->IsTimeWithinExtents(now))
        {
        // TODO: Make the region translucent if it's not defined at the
        // current time.
        theEvent->GetClosestDisplayRegion(now, npts, pts);
        }
      else
        {
        npts = 0;
        }
      }

    if (npts > 0)
      {
      this->Internal->Lines->InsertNextCell(npts, pts);
      this->Internal->EventIds->InsertNextValue(theEvent->GetId());

      double dcolor[3];

      // Use the event type color if one is available.
      int index = this->EventTypeRegistry ?
                  this->EventTypeRegistry->GetTypeIndex(eventType) : -1;
      if (index >= 0)
        {
        const vgEventType& type = this->EventTypeRegistry->GetType(index);
        type.GetColor(dcolor[0], dcolor[1], dcolor[2]);
        }
      else
        {
        this->Internal->RegionActor->GetProperty()->GetColor(dcolor);
        }

      unsigned char color[4];
      if (theEvent->GetUseCustomColor())
        {
        double customColor[3];
        theEvent->GetCustomColor(customColor);
        color[0] = 255 * customColor[0] * this->ColorMultiplier;
        color[1] = 255 * customColor[1] * this->ColorMultiplier;
        color[2] = 255 * customColor[2] * this->ColorMultiplier;
        color[3] = 255;
        }
      else
        {
        color[0] = 255 * dcolor[0] * this->ColorMultiplier;
        color[1] = 255 * dcolor[1] * this->ColorMultiplier;
        color[2] = 255 * dcolor[2] * this->ColorMultiplier;
        color[3] = 255 *
                   this->Internal->RegionActor->GetProperty()->GetOpacity();
        }

      this->Internal->EventColorArray->InsertNextTypedTuple(color);
      }
    }

  // An actor should only be visible if there is at least one cell to
  // display because the bounds of an "empty" polydata actor are determined
  // from its points; we may be storing points in image and points world
  // coordinates in the same vtkPoints but using only one or the other -
  // if there is nothing to display for a frame, the bounds might then be
  // computed incorrectly.
  this->Internal->RegionActor->SetVisibility(
    this->Internal->PolyData->GetNumberOfCells());

  this->Internal->PolyData->Modified();
  this->Internal->PolyData->DeleteCells();

  // Finally, setup the transform.  Test the RepresentationMatrix against
  // a single point in our result if (any), to determine whether or not to
  // multiply through by -1 to handle OpenGL issue with -1 homogeneous
  // coordinate
  vtkSmartPointer<vtkPolyDataCollection> polyDataCollection =
    vtkSmartPointer<vtkPolyDataCollection>::New();
  polyDataCollection->AddItem(this->Internal->PolyData);
  this->SetupActorTransform(polyDataCollection, this->RepresentationMatrix,
                            this->Internal->RegionTransform);

  // finish setting up the transform
  this->Internal->RegionTransform->Translate(0.0, 0.0, this->RegionZOffset
    + vtkVgRepresentationBase::GetZOffset(this->Internal->RegionActor));

  this->UpdateTime.Modified();
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgEventRegionRepresentation::Pick(double renX, double renY,
                                               vtkRenderer* ren,
                                               vtkIdType& pickType)
{
  pickType = vtkVgPickData::EmptyPick;

  if (!this->GetVisible())
    {
    return -1;
    }

  // Set up the pick list and add the actors to it
  this->Picker->InitializePickList();

  this->ActivePropCollection->InitTraversal();
  while (vtkProp* prop = this->ActivePropCollection->GetNextProp())
    {
    this->Picker->AddPickList(prop);
    }

  if (this->Picker->Pick(renX, renY, 0.0, ren))
    {
    vtkIdTypeArray* da =
      dynamic_cast<vtkIdTypeArray*>(
        this->Picker->GetDataSet()->GetCellData()->GetArray("Event Ids"));
    if (!da)
      {
      return -1;
      }

    vtkIdType cellId = this->Picker->GetCellId();
    vtkIdType eventId = da->GetValue(cellId);
    pickType = vtkVgPickData::PickedEvent;
    return eventId;
    }

  return -1;
}

//-----------------------------------------------------------------------------
void vtkVgEventRegionRepresentation::SetColor(double r, double g, double b)
{
  this->Internal->RegionActor->GetProperty()->SetColor(r, g, b);
}
