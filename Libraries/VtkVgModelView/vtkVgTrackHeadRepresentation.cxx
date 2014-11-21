/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgTrackHeadRepresentation.h"

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
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

#include <vtkVgCellPicker.h>
#include <vtkVgColorUtil.h>
#include <vtkVgTrack.h>
#include <vtkVgTriangulateConcavePolysFilter.h>

#include "vtkVgPickData.h"
#include "vtkVgTrackFilter.h"
#include "vtkVgTrackModel.h"

vtkStandardNewMacro(vtkVgTrackHeadRepresentation);

//----------------------------------------------------------------------------
class vtkVgTrackHeadRepresentation::vtkInternal
{
public:

  vtkCellArray* HeadVerts;
  vtkPolyData* HeadPolyData;
  vtkCellArray* HeadLines;
  vtkIdTypeArray* HeadTrackIds;

  vtkPolyData* HeadFillPolyData;

  vtkIdTypeArray* TemporaryTrackIds;
  vtkUnsignedCharArray* TemporaryColors;

  vtkInternal()
    {
    this->HeadPolyData = vtkPolyData::New();
    this->HeadTrackIds = vtkIdTypeArray::New();
    this->HeadTrackIds->SetName("Track Ids");
    this->HeadPolyData->GetCellData()->AddArray(this->HeadTrackIds);
    this->HeadTrackIds->FastDelete();
    this->HeadLines = vtkCellArray::New();
    this->HeadPolyData->SetLines(this->HeadLines);
    this->HeadLines->FastDelete();
    this->HeadVerts = vtkCellArray::New();
    this->HeadPolyData->SetVerts(this->HeadVerts);
    this->HeadVerts->FastDelete();

    this->HeadFillPolyData = vtkPolyData::New();

    vtkUnsignedCharArray* scalars = vtkUnsignedCharArray::New();
    scalars->SetNumberOfComponents(3);
    scalars->SetName("Track Color");
    this->HeadPolyData->GetCellData()->SetScalars(scalars);
    scalars->FastDelete();

    this->TemporaryTrackIds = vtkIdTypeArray::New();
    this->TemporaryColors = vtkUnsignedCharArray::New();
    this->TemporaryColors->SetNumberOfComponents(3);
    }

  ~vtkInternal()
    {
    this->TemporaryTrackIds->Delete();
    this->TemporaryColors->Delete();
    this->HeadPolyData->Delete();
    this->HeadFillPolyData->Delete();
    }
};

//-----------------------------------------------------------------------------
vtkVgTrackHeadRepresentation::vtkVgTrackHeadRepresentation()
{
  this->Internal = new vtkInternal();

  vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
  mapper->SetInputData(this->Internal->HeadPolyData);
  this->HeadActor = vtkActor::New();
  this->HeadActor->SetMapper(mapper);
  mapper->FastDelete();
  this->Visible = 1;

  this->ActorTransform = vtkTransform::New();
  this->HeadActor->SetUserTransform(this->ActorTransform);
  this->ActorTransform->FastDelete();

  this->HeadActor->GetProperty()->SetColor(1, 1, 0);
  this->HeadActor->GetProperty()->SetLineWidth(1);
  this->HeadActor->GetProperty()->SetPointSize(6);

  this->NewPropCollection    = vtkPropCollectionRef::New();
  this->ActivePropCollection = vtkPropCollectionRef::New();
  this->ExpirePropCollection = vtkPropCollectionRef::New();

  this->NewPropCollection->AddItem(this->HeadActor);
  this->ActivePropCollection->AddItem(this->HeadActor);

  this->DisplayAllHeads = false;
  this->ShowFill = false;
  this->ZOffset = 0;

  // "Fill" actor
  this->FillOpacity = 0.2;
  vtkPolyDataMapper* fillMapper = vtkPolyDataMapper::New();
  fillMapper->SetInputData(this->Internal->HeadFillPolyData);
  this->HeadFillActor = vtkActor::New();
  this->HeadFillActor->SetMapper(fillMapper);
  fillMapper->FastDelete();
  this->HeadFillActor->GetProperty()->SetColor(1, 1, 0);
  this->HeadFillActor->GetProperty()->SetOpacity(this->FillOpacity);
  this->HeadFillActor->SetUserTransform(this->ActorTransform);
  this->NewPropCollection->AddItem(this->HeadFillActor);
  this->ActivePropCollection->AddItem(this->HeadFillActor);
  this->HeadFillActor->VisibilityOff();

  // Support picking
  this->Picker = vtkSmartPointer<vtkVgCellPicker>::New();
  this->Picker->PickFromListOn();
  this->Picker->SetTolerance(0.001);
}

//-----------------------------------------------------------------------------
vtkVgTrackHeadRepresentation::~vtkVgTrackHeadRepresentation()
{
  this->HeadActor->Delete();
  this->HeadFillActor->Delete();
  delete this->Internal;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgTrackHeadRepresentation::GetNewRenderObjects() const
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgTrackHeadRepresentation::GetActiveRenderObjects() const
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgTrackHeadRepresentation::GetExpiredRenderObjects() const
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgTrackHeadRepresentation::GetNewRenderObjects()
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgTrackHeadRepresentation::GetActiveRenderObjects()
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgTrackHeadRepresentation::GetExpiredRenderObjects()
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
void vtkVgTrackHeadRepresentation::ResetTemporaryRenderObjects()
{
  this->NewPropCollection->RemoveAllItems();
  this->ExpirePropCollection->RemoveAllItems();
}

//-----------------------------------------------------------------------------
void vtkVgTrackHeadRepresentation::SetVisible(int flag)
{
  if (flag == this->Visible)
    {
    return;
    }

  this->Visible = flag;
  this->HeadActor->SetVisibility(
    flag && this->Internal->HeadPolyData->GetNumberOfCells());
  this->HeadFillActor->SetVisibility(this->ShowFill &&
    flag && this->Internal->HeadFillPolyData->GetNumberOfCells());
}

//-----------------------------------------------------------------------------
void vtkVgTrackHeadRepresentation::SetShowFill(bool showFill)
{
  if (showFill == this->ShowFill)
    {
    return;
    }

  this->ShowFill = showFill;
  // for now, we only hide a previously visible actor; we don't compute the
  // fill for a previously non-visible actor (dependent on the application to
  // update).
  if (!showFill)
    {
    this->HeadFillActor->VisibilityOff();
    }
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrackHeadRepresentation::SetFillOpacity(double opacity)
{
  if (opacity == this->FillOpacity)
    {
    return;
    }

  this->FillOpacity = opacity;
  this->HeadFillActor->GetProperty()->SetOpacity(this->FillOpacity);
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrackHeadRepresentation::Update()
{
  if (!this->GetVisible())
    {
    return;
    }

  this->Internal->HeadVerts->Reset();
  this->Internal->HeadLines->Reset();

  this->Internal->HeadTrackIds->Reset();
  this->Internal->TemporaryTrackIds->Reset();
  this->Internal->TemporaryColors->Reset();

  this->Internal->HeadPolyData->GetCellData()->GetScalars()->Reset();

  vtkSmartPointer<vtkCellArray> fillPolys;
  vtkSmartPointer<vtkUnsignedCharArray> fillColors;
  if (this->ShowFill)
    {
    // setup objects for fill
    fillPolys = vtkSmartPointer<vtkCellArray>::New();
    fillColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    fillColors->SetNumberOfComponents(3);
    fillColors->SetName("Track Fill Color");
    }

  // not the best place for the allocate, but it doesn't do anything after the
  // 1st allocation.  Just an estimate, as we don't know how many pieces tracks
  // will be broken up into
  vtkIdType numTracks = this->TrackModel->GetNumberOfTracks();
  this->Internal->HeadTrackIds->Allocate(2 * numTracks);
  this->Internal->TemporaryTrackIds->Allocate(numTracks);
  this->Internal->TemporaryColors->Allocate(3 * numTracks);

  this->Internal->HeadPolyData->SetPoints(this->TrackModel->GetPoints());

  vtkVgTimeStamp currFrame = this->TrackModel->GetCurrentTimeStamp();

  vtkUnsignedCharArray* colors =
    vtkUnsignedCharArray::SafeDownCast(
      this->Internal->HeadPolyData->GetCellData()->GetScalars());

  vtkVgTrackInfo trackInfo;
  this->TrackModel->InitTrackTraversal();
  while ((trackInfo = this->TrackModel->GetNextTrack()).GetTrack())
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
    if (!(trackInfo.GetDisplayTrack() || this->DisplayAllHeads) ||
        !(trackInfo.GetPassesFilters() && trackInfo.GetHeadVisible()))
      {
      continue;
      }
    if (this->TrackFilter &&
        this->TrackFilter->GetBestClassifier(track) < 0)
      {
      continue;
      }

    const double* color = this->GetTrackColor(trackInfo);
    unsigned char rgb[3];
    vtkVgColorUtil::convertMultiplied(color, this->ColorMultiplier, rgb);

    vtkIdType npts, *pts, trackPointId;
    track->GetHeadIdentifier(currFrame, npts, pts, trackPointId);
    if (npts == 1 || (npts == 0 && trackPointId > -1))
      {
      if (npts == 1)
        {
        this->Internal->HeadVerts->InsertNextCell(1, pts);
        }
      else
        {
        this->Internal->HeadVerts->InsertNextCell(1, &trackPointId);
        }
      this->Internal->HeadTrackIds->InsertNextValue(track->GetId());
      colors->InsertNextTupleValue(rgb);
      }
    else if (npts > 1)
      {
      this->Internal->HeadLines->InsertNextCell(npts, pts);
      this->Internal->TemporaryTrackIds->InsertNextValue(track->GetId());
      this->Internal->TemporaryColors->InsertNextTupleValue(rgb);
      if (this->ShowFill)
        {
        fillPolys->InsertNextCell(npts, pts);
        fillColors->InsertNextTupleValue(rgb);
        }
      }
    }

  if (this->Internal->TemporaryTrackIds->GetNumberOfTuples())
    {
    vtkIdType* writePointer = this->Internal->HeadTrackIds->WritePointer(
      this->Internal->HeadTrackIds->GetNumberOfTuples(),
      this->Internal->TemporaryTrackIds->GetNumberOfTuples());

    memcpy(writePointer, this->Internal->TemporaryTrackIds->GetPointer(0),
           this->Internal->TemporaryTrackIds->GetNumberOfTuples() * sizeof(vtkIdType));

    unsigned char* colorsWritePointer = colors->WritePointer(
      colors->GetNumberOfTuples() * 3,
      this->Internal->TemporaryColors->GetNumberOfTuples() * 3);

    memcpy(colorsWritePointer, this->Internal->TemporaryColors->GetPointer(0),
           this->Internal->TemporaryColors->GetNumberOfTuples() * 3);
    }


  if (this->ShowFill && fillPolys->GetNumberOfCells())
    {
    vtkPolyData *fillPolyData = vtkPolyData::New();
    fillPolyData->SetPoints(this->TrackModel->GetPoints());
    fillPolyData->SetPolys(fillPolys);
    fillPolyData->GetCellData()->SetScalars(fillColors);
    vtkVgTriangulateConcavePolysFilter* concavePolys =
      vtkVgTriangulateConcavePolysFilter::New();
    concavePolys->SetInputData(fillPolyData);
    fillPolyData->FastDelete();
    concavePolys->Update();
    this->Internal->HeadFillPolyData->DeepCopy(concavePolys->GetOutput());
    concavePolys->Delete();
    }
  else
    {
    this->Internal->HeadFillPolyData->Reset();
    }

  // An actor should only be visible if there is at least one cell to
  // display because the bounds of an "empty" polydata actor are determined
  // from its points; we may be storing points in image and points world
  // coordinates in the same vtkPoints but using only one or the other -
  // if there is nothing to display for a frame, the bounds might then be
  // computed incorrectly.
  this->HeadActor->SetVisibility(
    this->Internal->HeadPolyData->GetNumberOfCells());
  this->HeadFillActor->SetVisibility(
    this->Internal->HeadFillPolyData->GetNumberOfCells());

  this->Internal->HeadPolyData->Modified();
  this->Internal->HeadPolyData->DeleteCells();

  // Finally, setup the transform.  Test the RepresentationMatrix against
  // a single point in our result if (any), to determine whether or not to
  // multiply through by -1 to handle OpenGL issue with -1 homogeneous
  // coordinate
  vtkSmartPointer<vtkPolyDataCollection> polyDataCollection =
    vtkSmartPointer<vtkPolyDataCollection>::New();
  polyDataCollection->AddItem(this->Internal->HeadPolyData);
  this->SetupActorTransform(polyDataCollection, this->RepresentationMatrix,
                            this->ActorTransform);

  // finish setting up the transform
  this->ActorTransform->Translate(
    0.0, 0.0,
    this->ZOffset + vtkVgRepresentationBase::GetZOffset(this->HeadActor));
  this->ActorTransform->Translate(
    0.0, 0.0,
    this->ZOffset + vtkVgRepresentationBase::GetZOffset(this->HeadFillActor));

  this->UpdateTime.Modified();
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgTrackHeadRepresentation::Pick(double renX, double renY,
                                             vtkRenderer* ren,
                                             vtkIdType& pickType)
{
  pickType = vtkVgPickData::EmptyPick;

  if (!this->GetVisible())
    {
    return -1;
    }

  this->Picker->InitializePickList();

  this->Picker->AddPickList(this->HeadActor);

  int pickStatus = this->Picker->Pick(renX, renY, 0.0, ren);

  if (pickStatus)
    {
    vtkIdType cellId = this->Picker->GetCellId();
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
void vtkVgTrackHeadRepresentation::SetLineWidth(float width)
{
  this->HeadActor->GetProperty()->SetLineWidth(width);
}

//-----------------------------------------------------------------------------
float vtkVgTrackHeadRepresentation::GetLineWidth()
{
  return this->HeadActor->GetProperty()->GetLineWidth();
}

//-----------------------------------------------------------------------------
void vtkVgTrackHeadRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
