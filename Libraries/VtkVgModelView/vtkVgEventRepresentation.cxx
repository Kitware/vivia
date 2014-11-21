/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgEventRepresentation.h"

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCollection.h>
#include <vtkIdList.h>
#include <vtkIdListCollection.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataCollection.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>
#include <vtkUnsignedCharArray.h>

#include "vgEventType.h"

#include <vtkVgCellPicker.h>

#include "vtkVgEvent.h"
#include "vtkVgEventFilter.h"
#include "vtkVgEventModel.h"
#include "vtkVgEventTypeRegistry.h"
#include "vtkVgPickData.h"
#include "vtkVgPicker.h"
#include "vtkVgTrackModel.h"
#include "vtkVgTrackRepresentation.h"
#include "vtkVgTypeDefs.h"

#include <map>

vtkStandardNewMacro(vtkVgEventRepresentation);

vtkCxxSetObjectMacro(vtkVgEventRepresentation, TrackRepresentation, vtkVgTrackRepresentation);

namespace
{

// perhaps checkout http://colorschemedesigner.com/ to come up with a "better"
unsigned char RandomColors[][3] =
{
    {   0, 255, 255 }, // Cyan
    { 240, 230, 140 }, // Khaki
    { 178,  34,  34 }, // Firebrick
    { 106,  90, 205 }, // Slate Blue
    { 255,   0,   0 }, // Red
    {   0, 100,   0 }, // Dark Green
    { 188, 143, 143 }, // Rosy Brown
    {   0,   0, 255 }, // Blue
    { 127, 255,   0 }, // Chartreuse
    { 208,  32, 144 }, // Violet Red
    { 255,  69,   0 }  // Orange Red
};

enum { NumRandomColors = sizeof(RandomColors) / sizeof(RandomColors[0]) };

//----------------------------------------------------------------------------
struct EventBin
{
  EventBin();
  ~EventBin();

  vtkPolyData*          PolyData;
  vtkCellArray*         Verts;
  vtkCellArray*         Lines;
  vtkActor*             EventActor;
  vtkIdTypeArray*       EventIds;
  vtkUnsignedCharArray* EventColorArray;
  vtkIdTypeArray*       TemporaryEventIds;
  vtkUnsignedCharArray* TemporaryEventColorArray;
};

//----------------------------------------------------------------------------
EventBin::EventBin()
{
}

//----------------------------------------------------------------------------
EventBin::~EventBin()
{
  this->PolyData->Delete();
  this->EventActor->Delete();
  this->TemporaryEventIds->Delete();
  this->TemporaryEventColorArray->Delete();
}

//----------------------------------------------------------------------------
struct EventTypeInfo
{
  explicit EventTypeInfo(const int type) : Type(type) {}
  ~EventTypeInfo();

  int Type;

  // Mapping event type to map that has key as normalcy.
  // We could have used vectors instead of maps here, but in the future we
  // might not have the bin numbers in a row?
  std::map<int, EventBin*> NormalcyBin;

  bool   DisplayState;
  double DisplayNormalcyThreshold;
  double DisplayProbabilityThreshold;
};

typedef std::map<int, EventTypeInfo*> EventTypeMap;

//----------------------------------------------------------------------------
EventTypeInfo::~EventTypeInfo()
{
  std::map<int, EventBin*>::iterator iter = this->NormalcyBin.begin();
  while (iter != this->NormalcyBin.end())
    {
    delete iter->second;
    ++iter;
    }
}

} // end anonymous namespace

//----------------------------------------------------------------------------
struct vtkVgEventRepresentation::vtkInternal
{
  vtkInternal(vtkVgEventRepresentation* rep);
  ~vtkInternal();

  void RandomizeEventColors(bool state);

  int  RegisterEventType(const int id);

  bool UpdateActiveClassifier(vtkVgEvent* event);

  bool RandomEventColors;

  EventTypeMap EventTypes;
  vtkTimeStamp TypesBuildTime;

  // use the same transform for all event actors
  vtkTransform*  ActorTransform;

  float NormalcyBinOpacity[NumberOfNormalcyBins];

  vtkVgEventRepresentation* Representation;
};

//-----------------------------------------------------------------------------
vtkVgEventRepresentation::vtkInternal::vtkInternal(vtkVgEventRepresentation* rep)
  : Representation(rep)
{
  this->RandomEventColors = false;
  this->ActorTransform = vtkTransform::New();
}

//-----------------------------------------------------------------------------
vtkVgEventRepresentation::vtkInternal::~vtkInternal()
{
  std::map<int, EventTypeInfo*>::iterator iter = this->EventTypes.begin();
  while (iter != this->EventTypes.end())
    {
    delete iter->second;
    ++iter;
    }
  this->ActorTransform->Delete();
}

//-----------------------------------------------------------------------------
void vtkVgEventRepresentation::vtkInternal::RandomizeEventColors(bool state)
{
  this->RandomEventColors = state;
}

//-----------------------------------------------------------------------------
int vtkVgEventRepresentation::vtkInternal::RegisterEventType(const int id)
{
  int numOfNormalcyBins = this->Representation->NumberOfNormalcyBins;

  double opacityDelta = 0.6 / (numOfNormalcyBins - 1);

  EventTypeInfo* eti = new EventTypeInfo(id);
  this->EventTypes.insert(std::make_pair(id, eti));

  eti->DisplayState = false;
  eti->DisplayNormalcyThreshold = 1.0;
  eti->DisplayProbabilityThreshold = 0.0;

  for (int j = 0; j < numOfNormalcyBins; j++)
    {
    EventBin* bin = new EventBin;
    eti->NormalcyBin.insert(std::make_pair(j, bin));

    bin->PolyData = vtkPolyData::New();
    bin->Verts    = vtkCellArray::New();
    bin->Lines    = vtkCellArray::New();

    bin->PolyData->SetVerts(bin->Verts);
    bin->Verts->FastDelete();
    bin->PolyData->SetLines(bin->Lines);
    bin->Lines->FastDelete();

    bin->EventIds = vtkIdTypeArray::New();
    bin->EventIds->SetName("Event Ids");

    bin->PolyData->GetCellData()->AddArray(bin->EventIds);
    bin->EventIds->FastDelete();

    vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
    mapper->SetInputData(bin->PolyData);

    bin->EventActor = vtkActor::New();
    bin->EventActor->SetMapper(mapper);
    bin->EventActor->SetUserTransform(this->ActorTransform);

    this->Representation->NewPropCollection->AddItem(bin->EventActor);
    this->Representation->ActivePropCollection->AddItem(bin->EventActor);

    bin->EventColorArray = vtkUnsignedCharArray::New();
    bin->EventColorArray->SetNumberOfComponents(4);
    bin->EventColorArray->SetName("Event Color");
    bin->PolyData->GetCellData()->SetScalars(bin->EventColorArray);
    bin->EventColorArray->FastDelete();

    bin->TemporaryEventIds = vtkIdTypeArray::New();
    bin->TemporaryEventColorArray = vtkUnsignedCharArray::New();
    bin->TemporaryEventColorArray->SetNumberOfComponents(4);

    // set correct initial visibility
    bin->EventActor->SetVisibility(this->Representation->GetVisible());

    // Compute the opacities to be used in scalar data later. We do this
    // instead of setting opacity on the actor itself, since it allows us to
    // trick the renderer into enabling Z-writes for this geometry. Events need
    // to write to the Z buffer so that underlying tracks (which are rendered
    // later) will be masked. If vtkActor supported enabling Z-writes
    // unconditionally, we wouldn't need to do this.
    int ji = numOfNormalcyBins - 1 - j;
    this->NormalcyBinOpacity[j] = 0.4 + ji * opacityDelta;

    // swapping "top" and "bottom" of normalcy scaling
    bin->EventActor->GetProperty()->SetLineWidth(
      this->Representation->GetLineWidth() + ji);
    bin->EventActor->GetProperty()->SetPointSize(
      this->Representation->GetLineWidth() + ji);

    mapper->FastDelete();

    bin->PolyData->SetPoints(this->Representation->GetEventModel()
                             ->GetTrackModel()->GetPoints());
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
bool vtkVgEventRepresentation::vtkInternal::UpdateActiveClassifier(
  vtkVgEvent* event)
{
  event->ResetActiveClassifier();

  if (!this->Representation->EventFilter)
    {
    return true;
    }

  int bestType = this->Representation->EventFilter->GetBestClassifier(event);
  if (bestType != -1)
    {
    event->SetActiveClassifier(bestType);
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
vtkVgEventRepresentation::vtkVgEventRepresentation()
{
  this->Visible = 1;
  this->Internal = new vtkInternal(this);

  this->NewPropCollection    = vtkPropCollectionRef::New();
  this->ActivePropCollection = vtkPropCollectionRef::New();
  this->ExpirePropCollection = vtkPropCollectionRef::New();

  this->Picker = vtkSmartPointer<vtkVgCellPicker>::New();
  this->Picker->PickFromListOn();
  this->Picker->SetTolerance(0.01);
  this->PickPosition[0] = this->PickPosition[1] = this->PickPosition[2] = 0.0;

  this->TrackRepresentation = 0;
  this->ZOffset = 0.0;
  this->LineWidth = 3.0f;

  this->UseNormalcyCues = false;
  this->NormalcyCuesSwapped = false;
}

//-----------------------------------------------------------------------------
vtkVgEventRepresentation::~vtkVgEventRepresentation()
{
  if (this->TrackRepresentation)
    {
    this->TrackRepresentation->UnRegister(this);
    }
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgEventRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgEventRepresentation::GetNewRenderObjects() const
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgEventRepresentation::GetActiveRenderObjects() const
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgEventRepresentation::GetExpiredRenderObjects() const
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgEventRepresentation::GetNewRenderObjects()
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgEventRepresentation::GetActiveRenderObjects()
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgEventRepresentation::GetExpiredRenderObjects()
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
void vtkVgEventRepresentation::ResetTemporaryRenderObjects()
{
  this->NewPropCollection->RemoveAllItems();
  this->ExpirePropCollection->RemoveAllItems();
}

//-----------------------------------------------------------------------------
void vtkVgEventRepresentation::SetVisible(int flag)
{
  if (flag == this->Visible)
    {
    return;
    }

  this->Visible = flag;
  this->ActivePropCollection->InitTraversal();
  while (vtkProp* prop = this->ActivePropCollection->GetNextProp())
    {
    // FIXME - really should only make visible if there is polydata
    prop->SetVisibility(flag);
    }
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventRepresentation::Initialize()
{
}

//-----------------------------------------------------------------------------
void vtkVgEventRepresentation::RandomizeEventColors(bool state)
{
  if (state != this->Internal->RandomEventColors)
    {
    this->Internal->RandomizeEventColors(state);
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventRepresentation::Update()
{
  // Note: We do this assuming no one else is adding forced tracks.
  if (this->TrackRepresentation)
    {
    this->TrackRepresentation->ClearForceShownTracks();
    }

  if (!this->EventModel || !this->GetVisible())
    {
    return;
    }

  if ((this->EventTypeRegistry &&
       this->EventTypeRegistry->GetMTime() > this->Internal->TypesBuildTime) ||
      (!this->EventTypeRegistry && !this->Internal->EventTypes.empty()))
    {
    this->UpdateEventTypes();
    }

  EventTypeMap::const_iterator iter, end = this->Internal->EventTypes.end();
  for (iter = this->Internal->EventTypes.begin(); iter != end; ++iter)
    {
    for (int j = 0; j < this->NumberOfNormalcyBins; j++)
      {
      EventBin* bin = iter->second->NormalcyBin[j];
      bin->Verts->Reset();
      bin->Lines->Reset();
      bin->EventIds->Reset();
      bin->TemporaryEventIds->Reset();
      bin->PolyData->GetCellData()->GetScalars()->Reset();
      bin->TemporaryEventColorArray->Reset();

      // Not the best place for the allocate, but it doesn't do anything after
      // the first allocation.  Granted, it is also quite wasteful of memory
      // since we're handling the case where all events are in all bins and are
      // displayed at the same time (but is an upper bounds, thus we won't do
      // any additional allocating of memory).
      bin->EventIds->Allocate(this->EventModel->GetNumberOfEvents());
      bin->TemporaryEventIds->Allocate(this->EventModel->GetNumberOfEvents());
      bin->PolyData->GetCellData()->GetScalars()->Allocate(
        this->EventModel->GetNumberOfEvents() * 3);
      bin->TemporaryEventColorArray->Allocate(
        this->EventModel->GetNumberOfEvents() * 3);
      }
    }

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

    bool shown = this->Internal->UpdateActiveClassifier(theEvent);

    // if we don't recognize the active type of the event, ignore it
    int eventType = theEvent->GetActiveClassifierType();
    if (this->Internal->EventTypes.find(eventType) ==
        this->Internal->EventTypes.end())
      {
      continue;
      }

    // don't display if this event is being filtered out
    if (!shown)
      continue;

    int normalcyBin =
      this->NormalcyCuesSwapped ? this->NumberOfNormalcyBins - 1 : 0;

    if (this->UseNormalcyCues)
      {
      // which normalcy bin does it fall in
      double min = this->EventModel->GetNormalcyMinForType(eventType);
      double max = this->EventModel->GetNormalcyMaxForType(eventType);

      double delta = theEvent->GetActiveClassifierNormalcy() - min;
      double range = max - min;

      if (range == 0.0)
        {
        range = 1.0;
        }

      normalcyBin = this->NumberOfNormalcyBins * delta / range;

      if (normalcyBin == this->NumberOfNormalcyBins)
        {
        normalcyBin = NumberOfNormalcyBins - 1;
        }
      }

    const vgEventType& type = this->EventTypeRegistry->GetTypeById(eventType);
    EventBin* bin =
      this->Internal->EventTypes[eventType]->NormalcyBin[normalcyBin];

    int numberOfTracks = this->EventModel->GetUseTrackGroups() ?
                         theEvent->GetNumberOfTrackGroups() : theEvent->GetNumberOfTracks();
    // force the display of supporting tracks if we have a track representation
    vtkIdList* forcedTracks = 0;
    if (this->TrackRepresentation)
      {
      forcedTracks = vtkIdList::New();
      forcedTracks->Allocate(numberOfTracks);
      }

    vtkUnsignedCharArray* colors;
    for (int i = 0; i < numberOfTracks; ++i)
      {
      vtkVgTrackDisplayData tdd =
        this->EventModel->GetTrackDisplayData(theEvent, i);
      if (tdd.NumIds == 0)
        {
        continue;
        }

      if (forcedTracks)
        {
        forcedTracks->InsertNextId(this->EventModel->GetUseTrackGroups() ?
                                   theEvent->GetTrackGroupTrack(i)->GetId() :
                                   theEvent->GetTrack(i)->GetId());
        }

      // if only a single point (curently) for the event, display as Vert
      if (tdd.NumIds == 1)
        {
        bin->Verts->InsertNextCell(1, tdd.IdsStart);
        bin->EventIds->InsertNextValue(theEvent->GetId());
        colors = vtkUnsignedCharArray::SafeDownCast(
                   bin->PolyData->GetCellData()->GetScalars());
        }
      else
        {
        bin->Lines->InsertNextCell(tdd.NumIds, tdd.IdsStart);
        bin->TemporaryEventIds->InsertNextValue(theEvent->GetId());
        colors = bin->TemporaryEventColorArray;
        }

      unsigned char color[4];
      if (theEvent->GetUseCustomColor())
        {
        double customColor[3];
        theEvent->GetCustomColor(customColor);
        color[0] = 255 * customColor[0];
        color[1] = 255 * customColor[1];
        color[2] = 255 * customColor[2];
        }
      else if (this->HasOverrideColor)
        {
        color[0] = 255 * this->OverrideColor[0];
        color[1] = 255 * this->OverrideColor[1];
        color[2] = 255 * this->OverrideColor[2];
        }
      else if (this->Internal->RandomEventColors || type.GetUseRandomColors())
        {
        int colorIndex = (theEvent->GetId() + i) % NumRandomColors;
        color[0] = RandomColors[colorIndex][0];
        color[1] = RandomColors[colorIndex][1];
        color[2] = RandomColors[colorIndex][2];
        }
      else
        {
        double c[3];
        type.GetTrackColor(i, c[0], c[1], c[2]);
        color[0] = 255 * c[0];
        color[1] = 255 * c[1];
        color[2] = 255 * c[2];
        }
      color[3] = 255 * this->Internal->NormalcyBinOpacity[normalcyBin];

      double m = this->GetColorMultiplier();
      if (m != 1.0)
        {
        color[0] *= m;
        color[1] *= m;
        color[2] *= m;
        }
      colors->InsertNextTupleValue(color);
      }

    if (forcedTracks)
      {
      this->TrackRepresentation->ForceShowTracks(forcedTracks);
      forcedTracks->Delete();
      }
    }
  for (iter = this->Internal->EventTypes.begin(); iter != end; ++iter)
    {
    for (int j = 0; j < this->NumberOfNormalcyBins; j++)
      {
      EventBin* bin = iter->second->NormalcyBin[j];

      // An actor should only be visible if there is at least one cell to
      // display because the bounds of an "empty" polydata actor are determined
      // from its points; we may be storing points in image and points world
      // coordinates in the same vtkPoints but using only one or the other -
      // if there is nothing to display for a frame, the bounds might then be
      // computed incorrectly.
      bin->EventActor->SetVisibility(bin->PolyData->GetNumberOfCells());

      // copy "line" event id and color info to the approriate arrays
      if (bin->TemporaryEventIds->GetNumberOfTuples())
        {
        void* writePointer = bin->EventIds->WriteVoidPointer(
          bin->EventIds->GetNumberOfTuples(),
          bin->TemporaryEventIds->GetNumberOfTuples());
        memcpy(writePointer, bin->TemporaryEventIds->GetPointer(0),
               bin->TemporaryEventIds->GetNumberOfTuples() * sizeof(vtkIdType));

        writePointer = bin->EventColorArray->WriteVoidPointer(
          bin->EventColorArray->GetNumberOfTuples() * 4,
          bin->TemporaryEventColorArray->GetNumberOfTuples() * 4);
        memcpy(writePointer, bin->TemporaryEventColorArray->GetPointer(0),
               bin->TemporaryEventColorArray->GetNumberOfTuples() * 4);
        }

      bin->PolyData->Modified();
      bin->PolyData->DeleteCells();
      }
    }

  // Finally, setup the transform.  Test the RepresentationMatrix against
  // a single point in our result if (any), to determine whether or not to
  // multiply through by -1 to handle OpenGL issue with -1 homogeneous
  // coordinate
  vtkSmartPointer<vtkPolyDataCollection> polyDataCollection =
    vtkSmartPointer<vtkPolyDataCollection>::New();
  EventBin* bin = 0;
  for (iter = this->Internal->EventTypes.begin(); iter != end; ++iter)
    {
    for (int j = 0; j < this->NumberOfNormalcyBins; j++)
      {
      bin = iter->second->NormalcyBin[j];
      polyDataCollection->AddItem(bin->PolyData);
      }
    }
  if (bin)
    {
    this->SetupActorTransform(polyDataCollection, this->RepresentationMatrix,
                              this->Internal->ActorTransform);

    this->Internal->ActorTransform->Translate(0.0, 0.0, this->ZOffset +
      vtkVgRepresentationBase::GetZOffset(bin->EventActor));
    }

  this->UpdateTime.Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventRepresentation::UpdateEventTypes()
{
  this->Internal->TypesBuildTime.Modified();

  if (this->EventTypeRegistry)
    {
    // Add mappings for new types.
    for (int i = 0; i < this->EventTypeRegistry->GetNumberOfTypes(); ++i)
      {
      const vgEventType& type = this->EventTypeRegistry->GetType(i);
      if (this->Internal->EventTypes.count(type.GetId()) == 0)
        {
        this->Internal->RegisterEventType(type.GetId());
        }
      }

    // Remove mappings for types that have been removed.
    EventTypeMap::iterator iter = this->Internal->EventTypes.begin();
    while (iter != this->Internal->EventTypes.end())
      {
      if (this->EventTypeRegistry->GetTypeIndex(iter->first) < 0)
        {
        for (int i = 0; i < this->NumberOfNormalcyBins; ++i)
          {
          this->ExpirePropCollection->AddItem(
            iter->second->NormalcyBin[i]->EventActor);
          }
        delete iter->second;
        this->Internal->EventTypes.erase(iter++);
        }
      else
        {
        ++iter;
        }
      }
    }
  else
    {
    EventTypeMap::const_iterator iter, end = this->Internal->EventTypes.end();
    for (iter = this->Internal->EventTypes.begin(); iter != end; ++iter)
      {
      delete iter->second;
      }
    this->Internal->EventTypes.clear();
    }
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgEventRepresentation::
Pick(double renX, double renY, vtkRenderer* ren, vtkIdType& pickType)
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
    vtkIdType cellId = this->Picker->GetCellId();
    this->Picker->GetPickPosition(this->PickPosition);
    vtkIdTypeArray* da = dynamic_cast<vtkIdTypeArray*>(
      this->Picker->GetDataSet()->GetCellData()->GetArray("Event Ids"));
    if (!da)
      {
      return -1;
      }
    vtkIdType eventId = da->GetValue(cellId);
    pickType = vtkVgPickData::PickedEvent;
    return eventId;
    }

  return -1;
}

//-----------------------------------------------------------------------------
void vtkVgEventRepresentation::SwapNormalcyCues()
{
  for (int i = 0; i < this->EventTypeRegistry->GetNumberOfTypes(); i++)
    {
    int type = this->EventTypeRegistry->GetType(i).GetId();
    int top = this->NumberOfNormalcyBins - 1;
    for (int j = 0; j < this->NumberOfNormalcyBins; j++, top--)
      {
      if (j >= top)
        {
        break;
        }

      EventTypeInfo* eti = this->Internal->EventTypes[type];
      vtkActor* A = eti->NormalcyBin[j]->EventActor;
      vtkActor* B = eti->NormalcyBin[top]->EventActor;

      double tmpWidth = A->GetProperty()->GetLineWidth();
      A->GetProperty()->SetLineWidth(B->GetProperty()->GetLineWidth());
      A->GetProperty()->SetPointSize(B->GetProperty()->GetLineWidth());
      B->GetProperty()->SetLineWidth(tmpWidth);
      B->GetProperty()->SetPointSize(tmpWidth);
      }
    }

  for (int i = 0, j = this->NumberOfNormalcyBins - 1; i < j; ++i, --j)
    {
    std::swap(this->Internal->NormalcyBinOpacity[i],
              this->Internal->NormalcyBinOpacity[j]);
    }

  this->Superclass::SwapNormalcyCues();
}
