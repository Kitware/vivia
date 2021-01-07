// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgTrackRepresentation.h"

#include "vtkVgContourOperatorManager.h"
#include "vtkVgPickData.h"
#include "vtkVgPicker.h"
#include "vtkVgTrackFilter.h"
#include "vtkVgTrackModel.h"
#include "vtkVgTrack.h"

#include <vtkVgCellPicker.h>
#include <vtkVgClipPolyData.h>
#include <vtkVgColorUtil.h>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCharArray.h>
#include <vtkIdList.h>
#include <vtkIdListCollection.h>
#include <vtkImplicitBoolean.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataCollection.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

// C/C++ includes
#include <cassert>
#include <set>

vtkStandardNewMacro(vtkVgTrackRepresentation);

typedef std::set<vtkIdType> TrackIdSet;

//----------------------------------------------------------------------------
class vtkVgTrackRepresentation::vtkInternal
{
public:
  vtkPolyData*    ActiveTrackPolyData;
  vtkCellArray*   ActiveTrackVerts;
  vtkCellArray*   ActiveTrackLines;
  vtkIdTypeArray* ActiveTrackIds;
  vtkVgClipPolyData* ActiveSelectorClipPolyData;
  vtkVgClipPolyData* ActiveFilterClipPolyData;
  vtkCharArray*   ActiveSkipClipCells;

  vtkPolyData*    ExpiringTrackPolyData;
  vtkCellArray*   ExpiringTrackVerts;
  vtkCellArray*   ExpiringTrackLines;
  vtkIdTypeArray* ExpiringTrackIds;
  vtkVgClipPolyData* ExpiringSelectorClipPolyData;
  vtkVgClipPolyData* ExpiringFilterClipPolyData;
  vtkCharArray*   ExpiringSkipClipCells;

  // Variables to support verts/lines ordering issue in vtkPolyData
  vtkCharArray*   TemporarySkipClipCells;
  vtkIdTypeArray* TemporaryTrackIds;
  vtkUnsignedCharArray* TemporaryColors;
  vtkCharArray*   TemporaryExpiringSkipClipCells;
  vtkIdTypeArray* TemporaryExpiringTrackIds;
  vtkUnsignedCharArray* TemporaryExpiringColors;

  TrackIdSet ForceShownTracks;

  vtkInternal()
    {
    this->ActiveTrackPolyData = vtkPolyData::New();
    this->ActiveTrackVerts = vtkCellArray::New();
    this->ActiveTrackPolyData->SetVerts(this->ActiveTrackVerts);
    this->ActiveTrackVerts->FastDelete();
    this->ActiveTrackLines = vtkCellArray::New();
    this->ActiveTrackPolyData->SetLines(this->ActiveTrackLines);
    this->ActiveTrackLines->FastDelete();
    this->ActiveTrackIds = vtkIdTypeArray::New();
    this->ActiveTrackIds->SetName("Track Ids");
    this->ActiveTrackPolyData->GetCellData()->AddArray(this->ActiveTrackIds);
    this->ActiveTrackIds->FastDelete();
    this->ActiveSkipClipCells = vtkCharArray::New();
    this->ActiveSkipClipCells->SetName("Skip Cells");
    this->ActiveTrackPolyData->GetCellData()->AddArray(this->ActiveSkipClipCells);
    this->ActiveSkipClipCells->FastDelete();
    this->ActiveSelectorClipPolyData = vtkVgClipPolyData::New();
    this->ActiveSelectorClipPolyData->InsideOutOn();
    this->ActiveFilterClipPolyData = vtkVgClipPolyData::New();
    this->ActiveFilterClipPolyData->InsideOutOff();

    // selector input (when used), will always be the raw poly data; filter input
    // could be the selector or the raw poly data
    this->ActiveSelectorClipPolyData->SetInputData(this->ActiveTrackPolyData);

    this->ExpiringTrackPolyData = vtkPolyData::New();
    this->ExpiringTrackIds = vtkIdTypeArray::New();
    this->ExpiringTrackIds->SetName("Track Ids");
    this->ExpiringTrackPolyData->GetCellData()->AddArray(this->ExpiringTrackIds);
    this->ExpiringTrackIds->FastDelete();
    this->ExpiringSkipClipCells = vtkCharArray::New();
    this->ExpiringSkipClipCells->SetName("Skip Cells");
    this->ExpiringTrackPolyData->GetCellData()->AddArray(this->ExpiringSkipClipCells);
    this->ExpiringSkipClipCells->FastDelete();
    this->ExpiringTrackVerts = vtkCellArray::New();
    this->ExpiringTrackPolyData->SetVerts(this->ExpiringTrackVerts);
    this->ExpiringTrackVerts->FastDelete();
    this->ExpiringTrackLines = vtkCellArray::New();
    this->ExpiringTrackPolyData->SetLines(this->ExpiringTrackLines);
    this->ExpiringTrackLines->FastDelete();
    this->ExpiringSelectorClipPolyData = vtkVgClipPolyData::New();
    this->ExpiringSelectorClipPolyData->InsideOutOn();
    this->ExpiringFilterClipPolyData = vtkVgClipPolyData::New();
    this->ExpiringFilterClipPolyData->InsideOutOff();

    // selector input (when used), will always be the raw poly data; filter input
    // could be the selector or the raw poly data
    this->ExpiringSelectorClipPolyData->SetInputData(this->ExpiringTrackPolyData);

    this->TemporarySkipClipCells = vtkCharArray::New();
    this->TemporaryTrackIds = vtkIdTypeArray::New();
    this->TemporaryColors = vtkUnsignedCharArray::New();
    this->TemporaryColors->SetNumberOfComponents(3);

    this->TemporaryExpiringSkipClipCells = vtkCharArray::New();
    this->TemporaryExpiringTrackIds = vtkIdTypeArray::New();
    this->TemporaryExpiringColors = vtkUnsignedCharArray::New();
    this->TemporaryExpiringColors->SetNumberOfComponents(3);
    }

  ~vtkInternal()
    {
    this->ActiveTrackPolyData->Delete();
    this->ExpiringTrackPolyData->Delete();
    this->ActiveSelectorClipPolyData->Delete();
    this->ActiveFilterClipPolyData->Delete();
    this->ExpiringSelectorClipPolyData->Delete();
    this->ExpiringFilterClipPolyData->Delete();
    this->TemporarySkipClipCells->Delete();
    this->TemporaryTrackIds->Delete();
    this->TemporaryColors->Delete();
    this->TemporaryExpiringSkipClipCells->Delete();
    this->TemporaryExpiringTrackIds->Delete();
    this->TemporaryExpiringColors->Delete();
    }
};

//-----------------------------------------------------------------------------
vtkVgTrackRepresentation::vtkVgTrackRepresentation()
{
  this->Internal = new vtkInternal();

  this->Visible = true;

  vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
  mapper->SetInputData(this->Internal->ActiveTrackPolyData);
  this->TrackActor = vtkActor::New();
  this->TrackActor->SetMapper(mapper);
  this->TrackActor->GetProperty()->SetColor(1.0, 1.0, 1.0);
  this->TrackActor->GetProperty()->SetLineWidth(3);
  this->TrackActor->GetProperty()->SetPointSize(3);
  mapper->FastDelete();

  vtkPolyDataMapper* mapper2 = vtkPolyDataMapper::New();
  mapper2->SetInputData(this->Internal->ExpiringTrackPolyData);
  this->ExpiringTrackActor = vtkActor::New();
  this->ExpiringTrackActor->SetMapper(mapper2);
  this->ExpiringTrackActor->GetProperty()->SetColor(0.8, 0.8, 0.8);
  this->ExpiringTrackActor->GetProperty()->SetLineWidth(2);
  this->ExpiringTrackActor->GetProperty()->SetPointSize(2);
  mapper2->FastDelete();

  // Use same transform for both Track and ExpiringTrack
  this->ActorTransform = vtkSmartPointer<vtkTransform>::New();
  this->TrackActor->SetUserTransform(this->ActorTransform);
  this->ExpiringTrackActor->SetUserTransform(this->ActorTransform);

  this->ZOffset = 0;

  vtkUnsignedCharArray* scalars = vtkUnsignedCharArray::New();
  scalars->SetNumberOfComponents(3);
  scalars->SetName("Track Color");
  this->Internal->ActiveTrackPolyData->GetCellData()->SetScalars(scalars);
  scalars->FastDelete();

  scalars = vtkUnsignedCharArray::New();
  scalars->SetNumberOfComponents(3);
  scalars->SetName("Track Color");
  this->Internal->ExpiringTrackPolyData->GetCellData()->SetScalars(scalars);
  scalars->FastDelete();

  // Make track actors imperceptibly translucent so that they will be rendered
  // during the during the same phase of the pipeline as events. This is a hack
  // that allows tracks to be rendered *after* events so that events can mask
  // underlying tracks.
  this->TrackActor->GetProperty()->SetOpacity(0.999);
  this->ExpiringTrackActor->GetProperty()->SetOpacity(0.999);

  // Support picking
  this->Picker = vtkSmartPointer<vtkVgCellPicker>::New();
  this->Picker->PickFromListOn();
  this->Picker->SetTolerance(0.01);
  this->PickPosition[0] = this->PickPosition[1] = this->PickPosition[2] = 0.0;

  this->NewPropCollection    = vtkPropCollectionRef::New();
  this->ActivePropCollection = vtkPropCollectionRef::New();
  this->ExpirePropCollection = vtkPropCollectionRef::New();

  this->NewPropCollection->AddItem(this->TrackActor);
  this->NewPropCollection->AddItem(this->ExpiringTrackActor);

  this->ActivePropCollection->AddItem(this->TrackActor);
  this->ActivePropCollection->AddItem(this->ExpiringTrackActor);

  this->OnlyDisplayForcedTracks = false;
}

//-----------------------------------------------------------------------------
vtkVgTrackRepresentation::~vtkVgTrackRepresentation()
{
  this->TrackActor->Delete();
  this->ExpiringTrackActor->Delete();
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgTrackRepresentation::GetNewRenderObjects() const
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgTrackRepresentation::GetActiveRenderObjects() const
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgTrackRepresentation::GetExpiredRenderObjects() const
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgTrackRepresentation::GetNewRenderObjects()
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgTrackRepresentation::GetActiveRenderObjects()
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgTrackRepresentation::GetExpiredRenderObjects()
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentation::ResetTemporaryRenderObjects()
{
  this->NewPropCollection->RemoveAllItems();
  this->ExpirePropCollection->RemoveAllItems();
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentation::SetVisible(int flag)
{
  if (flag == this->Visible)
    {
    return;
    }

  this->Visible = flag;
  this->TrackActor->SetVisibility(
    flag && this->Internal->ActiveTrackPolyData->GetNumberOfCells());
  this->ExpiringTrackActor->SetVisibility(
    flag && this->Internal->ExpiringTrackPolyData->GetNumberOfCells());
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentation::Update()
{
  if (!this->GetVisible())
    {
    return;
    }

  this->SetupPipelineForFiltersAndSelectors();

  assert(this->ActorTransform ==
         static_cast<vtkTransform*>(this->TrackActor->GetUserTransform()));
  assert(this->ActorTransform ==
         static_cast<vtkTransform*>(this->ExpiringTrackActor->GetUserTransform()));

  this->Internal->ActiveTrackVerts->Reset();
  this->Internal->ActiveTrackLines->Reset();
  this->Internal->ExpiringTrackVerts->Reset();
  this->Internal->ExpiringTrackLines->Reset();

  this->Internal->ActiveTrackIds->Reset();
  this->Internal->ExpiringTrackIds->Reset();

  this->Internal->TemporarySkipClipCells->Reset();
  this->Internal->TemporaryTrackIds->Reset();
  this->Internal->TemporaryColors->Reset();
  this->Internal->TemporaryExpiringSkipClipCells->Reset();
  this->Internal->TemporaryExpiringTrackIds->Reset();
  this->Internal->TemporaryExpiringColors->Reset();

  this->Internal->ActiveTrackPolyData->GetCellData()->GetScalars()->Reset();
  this->Internal->ExpiringTrackPolyData->GetCellData()->GetScalars()->Reset();

  this->Internal->ActiveSkipClipCells->Reset();
  this->Internal->ExpiringSkipClipCells->Reset();

  // not the best place for the allocate, but it doesn't do anything after the
  // 1st allocation.
  int numTracks = this->TrackModel->GetNumberOfTracks();
  this->Internal->ActiveTrackIds->Allocate(numTracks);
  this->Internal->ExpiringTrackIds->Allocate(numTracks);
  this->Internal->ActiveSkipClipCells->Allocate(numTracks);
  this->Internal->ExpiringSkipClipCells->Allocate(numTracks);

  this->Internal->TemporaryTrackIds->Allocate(numTracks);
  this->Internal->TemporaryColors->Allocate(3 * numTracks);
  this->Internal->TemporarySkipClipCells->Allocate(numTracks);

  this->Internal->TemporaryExpiringTrackIds->Allocate(numTracks);
  this->Internal->TemporaryExpiringColors->Allocate(3 * numTracks);
  this->Internal->TemporaryExpiringSkipClipCells->Allocate(numTracks);

  this->Internal->ActiveTrackPolyData->SetPoints(this->TrackModel->GetPoints());
  this->Internal->ExpiringTrackPolyData->SetPoints(this->TrackModel->GetPoints());

  vtkVgTimeStamp currFrame = this->TrackModel->GetCurrentTimeStamp();

  TrackIdSet& forceShownTracks = this->Internal->ForceShownTracks;

  vtkUnsignedCharArray* activeColors =
    vtkUnsignedCharArray::SafeDownCast(
      this->Internal->ActiveTrackPolyData->GetCellData()->GetScalars());

  vtkUnsignedCharArray* expiredColors =
    vtkUnsignedCharArray::SafeDownCast(
      this->Internal->ExpiringTrackPolyData->GetCellData()->GetScalars());

  // add 'force shown' tracks
  bool showExtraTracks = forceShownTracks.size() != 0;
  if (showExtraTracks)
    {
    TrackIdSet::iterator itr = forceShownTracks.begin();
    for (TrackIdSet::iterator end(forceShownTracks.end()); itr != end; ++itr)
      {
      vtkVgTrackInfo info = this->TrackModel->GetTrackInfo(*itr);
      if (!info.GetTrack())
        {
        continue;
        }

      this->AddTrackRep(info.GetTrack(), currFrame, true,
                        info, activeColors, expiredColors);
      }
    }

  // build polydata for displayed tracks
  if (!this->OnlyDisplayForcedTracks)
    {
    vtkVgTrackInfo trackInfo;
    this->TrackModel->InitTrackTraversal();
    while ((trackInfo = this->TrackModel->GetNextDisplayedTrack()).GetTrack())
      {
      vtkVgTrack* track = trackInfo.GetTrack();
      if (track == this->ExcludedTrack)
        {
        continue;
        }
      unsigned int mask = this->GetDisplayMask();
      if ((mask & track->GetDisplayFlags()) != mask)
        {
        continue;
        }
      if (this->TrackFilter && this->TrackFilter->GetBestClassifier(track) < 0)
        {
        continue;
        }

      // make sure we don't add the same track twice
      if (!showExtraTracks ||
          forceShownTracks.find(track->GetId()) == forceShownTracks.end())
        {
        this->AddTrackRep(track, currFrame, false,
                          trackInfo, activeColors, expiredColors);
        }
      }
    }

  if (this->Internal->TemporaryTrackIds->GetNumberOfTuples())
    {
    this->AppendArray(this->Internal->ActiveTrackIds,
                      this->Internal->TemporaryTrackIds,
                      this->Internal->TemporaryTrackIds->GetPointer(0));
    this->AppendArray(activeColors, this->Internal->TemporaryColors,
                      this->Internal->TemporaryColors->GetPointer(0));
    this->AppendArray(this->Internal->ActiveSkipClipCells,
                      this->Internal->TemporarySkipClipCells,
                      this->Internal->TemporarySkipClipCells->GetPointer(0));
    }
  if (this->Internal->TemporaryExpiringTrackIds->GetNumberOfTuples())
    {
    this->AppendArray(this->Internal->ExpiringTrackIds,
                      this->Internal->TemporaryExpiringTrackIds,
                      this->Internal->TemporaryExpiringTrackIds->GetPointer(0));
    this->AppendArray(expiredColors, this->Internal->TemporaryExpiringColors,
                      this->Internal->TemporaryExpiringColors->GetPointer(0));
    this->AppendArray(
      this->Internal->ExpiringSkipClipCells,
      this->Internal->TemporaryExpiringSkipClipCells,
      this->Internal->TemporaryExpiringSkipClipCells->GetPointer(0));
    }

  // An actor should only be visible if there is at least one cell to
  // display because the bounds of an "empty" polydata actor are determined
  // from its points; we may be storing points in image and points world
  // coordinates in the same vtkPoints but using only one or the other -
  // if there is nothing to display for a frame, the bounds might then be
  // computed incorrectly.
  this->ExpiringTrackActor->SetVisibility(
    this->Internal->ExpiringTrackPolyData->GetNumberOfCells());
  this->TrackActor->SetVisibility(
    this->Internal->ActiveTrackPolyData->GetNumberOfCells());

  this->Internal->ExpiringTrackPolyData->Modified();
  this->Internal->ExpiringTrackPolyData->DeleteCells();

  this->Internal->ActiveTrackPolyData->Modified();
  this->Internal->ActiveTrackPolyData->DeleteCells();

  // Finally, setup the transform.  Test the RepresentationMatrix against
  // a single point in our result if (any), to determine whether or not to
  // multiply through by -1 to handle OpenGL issue with -1 homogeneous
  // coordinate
  vtkSmartPointer<vtkPolyDataCollection> polyDataCollection =
    vtkSmartPointer<vtkPolyDataCollection>::New();
  polyDataCollection->AddItem(this->Internal->ActiveTrackPolyData);
  polyDataCollection->AddItem(this->Internal->ExpiringTrackPolyData);
  this->SetupActorTransform(polyDataCollection, this->RepresentationMatrix,
                            this->ActorTransform);

  // finish setting up the transform
  this->ActorTransform->Translate(0.0, 0.0, this->ZOffset +
    vtkVgRepresentationBase::GetZOffset(this->TrackActor));

  this->UpdateTime.Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentation::AddTrackRep(vtkVgTrack* track,
                                           vtkVgTimeStamp& currFrame,
                                           bool displayRegardlessOfFrame,
                                           const vtkVgTrackInfo& info,
                                           vtkUnsignedCharArray* activeColors,
                                           vtkUnsignedCharArray* expiredColors)
{
  // TODO: Clean up this code
  vtkVgTrackDisplayData displayData =
    this->TrackModel->GetTrackDisplayData(track, displayRegardlessOfFrame);
  if (!displayData.IdsStart)
    {
    return;
    }

  const double* color;
  unsigned char activeRGB[3];
  unsigned char expiredRGB[3];

  if (!this->UsingPerFrameColors())
    {
    color = this->GetTrackColor(info);
    vtkVgColorUtil::convertMultiplied(color, this->ColorMultiplier, activeRGB);

    // Use a slightly darker shade for expired tracks
    const double expiredMultiplier = this->ColorMultiplier * 0.7;
    vtkVgColorUtil::convertMultiplied(color, expiredMultiplier, expiredRGB);
    }

  if (currFrame < track->GetStartFrame() || track->GetEndFrame() < currFrame)
    {
    if (displayData.NumIds == 1)   // Verts (single/1st pt of track)
      {
      this->Internal->ExpiringTrackVerts->InsertNextCell(1, displayData.IdsStart);
      if (this->UsingPerFrameColors())
        {
        color = this->GetTrackColor(info, displayData.Scalars[0]);
        vtkVgColorUtil::convertMultiplied(color, this->ColorMultiplier,
                                          expiredRGB);
        }

      expiredColors->InsertNextTypedTuple(expiredRGB);

      this->Internal->ExpiringTrackIds->InsertNextValue(track->GetId());
      this->Internal->ExpiringSkipClipCells->InsertNextValue(
        displayRegardlessOfFrame ? 1 : 0);
      }
    else // Lines
      {
      if (!this->UsingPerFrameColors())
        {
        this->Internal->ExpiringTrackLines->InsertNextCell(displayData.NumIds,
            displayData.IdsStart);
        this->Internal->TemporaryExpiringColors->InsertNextTypedTuple(expiredRGB);
        this->Internal->TemporaryExpiringTrackIds->InsertNextValue(track->GetId());
        this->Internal->TemporaryExpiringSkipClipCells->InsertNextValue(
          displayRegardlessOfFrame ? 1 : 0);
        }
      else
        {
        // Number of cells = number of points - 1
        int numberOfIds = static_cast<int>(displayData.NumIds) - 1;
        for (int i = 0; i < numberOfIds; ++i)
          {
          this->Internal->ExpiringTrackLines->InsertNextCell(
            2, displayData.IdsStart + i);

          color = this->GetTrackColor(info, displayData.Scalars[i]);
          vtkVgColorUtil::convertMultiplied(color, this->ColorMultiplier,
                                            expiredRGB);
          this->Internal->TemporaryExpiringColors->InsertNextTypedTuple(expiredRGB);
          this->Internal->TemporaryExpiringTrackIds->InsertNextValue(track->GetId());
          this->Internal->TemporaryExpiringSkipClipCells->InsertNextValue(
            displayRegardlessOfFrame ? 1 : 0);
          }
        }
      }
    }
  else
    {
    if (displayData.NumIds == 1)   // Verts (single/1st pt of track)
      {
      if (this->UsingPerFrameColors())
        {
        color = this->GetTrackColor(info, displayData.Scalars[0]);
        vtkVgColorUtil::convertMultiplied(color, this->ColorMultiplier,
                                          activeRGB);
        }

      this->Internal->ActiveTrackVerts->InsertNextCell(1, displayData.IdsStart);
      activeColors->InsertNextTypedTuple(activeRGB);
      this->Internal->ActiveTrackIds->InsertNextValue(track->GetId());
      this->Internal->ActiveSkipClipCells->InsertNextValue(
        displayRegardlessOfFrame ? 1 : 0);
      }
    else // Lines
      {
      if (!this->UsingPerFrameColors())
        {
        this->Internal->ActiveTrackLines->InsertNextCell(displayData.NumIds,
                                                         displayData.IdsStart);
        this->Internal->TemporaryColors->InsertNextTypedTuple(activeRGB);
        this->Internal->TemporaryTrackIds->InsertNextValue(track->GetId());
        this->Internal->TemporarySkipClipCells->InsertNextValue(
          displayRegardlessOfFrame ? 1 : 0);
        }
      else
        {
        // Number of cells = number of points - 1
        int numberOfIds = static_cast<int>(displayData.NumIds) - 1;
        for (int i = 0; i < numberOfIds; ++i)
          {
          this->Internal->ActiveTrackLines->InsertNextCell(2,
              displayData.IdsStart + i);

          color = this->GetTrackColor(info, displayData.Scalars[i]);
          vtkVgColorUtil::convertMultiplied(color, this->ColorMultiplier,
                                            activeRGB);
          this->Internal->TemporaryColors->InsertNextTypedTuple(activeRGB);
          this->Internal->TemporaryTrackIds->InsertNextValue(track->GetId());
          this->Internal->TemporarySkipClipCells->InsertNextValue(
            displayRegardlessOfFrame ? 1 : 0);
          }
        }

      }
    }
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgTrackRepresentation::Pick(double renX, double renY,
                                         vtkRenderer* ren, vtkIdType& pickType)
{
  pickType = vtkVgPickData::EmptyPick;

  if (!this->GetVisible())
    {
    return -1;
    }

  this->Picker->InitializePickList();

  this->Picker->AddPickList(this->TrackActor);
  this->Picker->AddPickList(this->ExpiringTrackActor);

  int pickStatus = this->Picker->Pick(renX, renY, 0.0, ren);

  if (pickStatus)
    {
    vtkIdType cellId = this->Picker->GetCellId();
    this->Picker->GetPickPosition(this->PickPosition);
    vtkIdTypeArray* da =
      dynamic_cast<vtkIdTypeArray*>(
        this->Picker->GetDataSet()->GetCellData()->GetArray("Track Ids"));
    if (!da)
      {
      return -1;
      }
    vtkIdType trackId = da->GetValue(cellId);
    pickType = vtkVgPickData::PickedTrack;
    return trackId;
    }

  return -1;
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentation::ForceShowTracks(vtkIdList* ids)
{
  vtkIdType* idsArray = ids->GetPointer(0);
  this->Internal->ForceShownTracks.insert(idsArray,
                                          idsArray + ids->GetNumberOfIds());
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentation::ClearForceShownTracks()
{
  if (!this->Internal->ForceShownTracks.empty())
    {
    this->Internal->ForceShownTracks.clear();
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentation::SetupPipelineForFiltersAndSelectors()
{
  // nothing to do if we don't have "contours"
  if (!this->ContourOperatorManager)
    {
    return;
    }

  bool useSelectors = this->ContourOperatorManager->GetNumberOfEnabledSelectors() ?
                      true : false;
  bool useFilters = this->ContourOperatorManager->GetNumberOfEnabledFilters() ?
                    true : false;

  vtkPolyDataMapper* trackMapper = vtkPolyDataMapper::SafeDownCast(
    this->TrackActor->GetMapper());
  vtkPolyDataMapper* expiringTrackMapper = vtkPolyDataMapper::SafeDownCast(
    this->ExpiringTrackActor->GetMapper());

  if (!useSelectors && !useFilters)
    {
    trackMapper->SetInputData(
      this->Internal->ActiveTrackPolyData);
    expiringTrackMapper->SetInputData(
      this->Internal->ExpiringTrackPolyData);
    }
  else if (useSelectors)  // where does the output of the selector go
    {
    this->Internal->ActiveSelectorClipPolyData->SetClipFunction(
      this->ContourOperatorManager->GetSelectorBoolean());
    this->Internal->ExpiringSelectorClipPolyData->SetClipFunction(
      this->ContourOperatorManager->GetSelectorBoolean());
    if (useFilters)
      {
      this->Internal->ActiveFilterClipPolyData->SetInputConnection(
        this->Internal->ActiveSelectorClipPolyData->GetOutputPort());
      this->Internal->ExpiringFilterClipPolyData->SetInputConnection(
        this->Internal->ExpiringSelectorClipPolyData->GetOutputPort());
      }
    else
      {
      trackMapper->SetInputConnection(
        this->Internal->ActiveSelectorClipPolyData->GetOutputPort());
      expiringTrackMapper->SetInputConnection(
        this->Internal->ExpiringSelectorClipPolyData->GetOutputPort());
      }
    }

  if (useFilters)
    {
    this->Internal->ActiveFilterClipPolyData->SetClipFunction(
      this->ContourOperatorManager->GetFilterBoolean());
    this->Internal->ExpiringFilterClipPolyData->SetClipFunction(
      this->ContourOperatorManager->GetFilterBoolean());

    if (!useSelectors)
      {
      this->Internal->ActiveFilterClipPolyData->SetInputData(
        this->Internal->ActiveTrackPolyData);
      this->Internal->ExpiringFilterClipPolyData->SetInputData(
        this->Internal->ExpiringTrackPolyData);
      }
    trackMapper->SetInputConnection(
      this->Internal->ActiveFilterClipPolyData->GetOutputPort());
    expiringTrackMapper->SetInputConnection(
      this->Internal->ExpiringFilterClipPolyData->GetOutputPort());
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentation::SetActiveTrackLineWidth(float width)
{
  this->TrackActor->GetProperty()->SetLineWidth(width);
  this->TrackActor->GetProperty()->SetPointSize(width);
}

//-----------------------------------------------------------------------------
float vtkVgTrackRepresentation::GetActiveTrackLineWidth()
{
  return this->TrackActor->GetProperty()->GetLineWidth();
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentation::SetExpiringTrackLineWidth(float width)
{
  this->ExpiringTrackActor->GetProperty()->SetLineWidth(width);
  this->ExpiringTrackActor->GetProperty()->SetPointSize(width);
}

//-----------------------------------------------------------------------------
float vtkVgTrackRepresentation::GetExpiringTrackLineWidth()
{
  return this->ExpiringTrackActor->GetProperty()->GetLineWidth();
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentation::AppendArray(vtkDataArray* base,
                                           vtkDataArray* add,
                                           void* addPointer)
{
  void* writePointer = base->WriteVoidPointer(
    base->GetNumberOfTuples() * base->GetNumberOfComponents(),
    add->GetNumberOfTuples() * add->GetNumberOfComponents());
  memcpy(writePointer, addPointer,
         add->GetNumberOfTuples() * add->GetNumberOfComponents() *
         add->GetElementComponentSize());
}
