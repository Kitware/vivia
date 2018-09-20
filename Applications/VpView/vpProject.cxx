/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpProject.h"

#include "vpModelIO.h"
#include "vtkVpTrackModel.h"

#include <vtkVgActivityManager.h>
#include <vtkVgEventIconRepresentation.h>
#include <vtkVgEventModel.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgEventRepresentation.h>
#include <vtkVgIconManager.h>
#include <vtkVgPicker.h>
#include <vtkVgTrackHeadRepresentation.h>
#include <vtkVgTrackRepresentation.h>

//-----------------------------------------------------------------------------
vpProject::vpProject(int id) :
  TagFileMap{BuildTagFileMap(this)},
  ProjectId(id),
  NextCreateTrackId(3000000),
  IsVisible(true)
{
  this->TrackModel = vtkSmartPointer<vtkVpTrackModel>::New();
  this->TrackRepresentation = vtkSmartPointer<vtkVgTrackRepresentation>::New();
  this->SelectedTrackRepresentation =
    vtkSmartPointer<vtkVgTrackRepresentation>::New();
  this->TrackHeadRepresentation =
    vtkSmartPointer<vtkVgTrackHeadRepresentation>::New();
  this->SelectedTrackHeadRepresentation =
    vtkSmartPointer<vtkVgTrackHeadRepresentation>::New();

  this->EventModel = vtkSmartPointer<vtkVgEventModel>::New();
  this->EventRepresentation = vtkSmartPointer<vtkVgEventRepresentation>::New();
  this->SelectedEventRepresentation = vtkSmartPointer<vtkVgEventRepresentation>::New();
  this->EventIconRepresentation =
    vtkSmartPointer<vtkVgEventIconRepresentation>::New();
  this->EventRegionRepresentation =
    vtkSmartPointer<vtkVgEventRegionRepresentation>::New();

  this->ActivityManager = vtkSmartPointer<vtkVgActivityManager>::New();

  this->SceneElementRepresentation =
    vtkSmartPointer<vtkVgTrackHeadRepresentation>::New();
  this->SceneElementRepresentation->SetShowFill(true);
  this->SelectedSceneElementRepresentation =
    vtkSmartPointer<vtkVgTrackHeadRepresentation>::New();

  this->IconManager = vtkSmartPointer<vtkVgIconManager>::New();

  this->Picker = vtkSmartPointer<vtkVgPicker>::New();
}

//-----------------------------------------------------------------------------
vpProject::~vpProject()
{
}

//-----------------------------------------------------------------------------
QHash<QString, QString*> vpProject::BuildTagFileMap(vpProjectBase* base)
{
  QHash<QString, QString*> map;

#define ADD_MAPPING(x) map.insert(x##Tag, &base->x)
  ADD_MAPPING(OverviewFile);
  ADD_MAPPING(DataSetSpecifier);
  ADD_MAPPING(TracksFile);
  ADD_MAPPING(TrackTraitsFile);
  ADD_MAPPING(EventsFile);
  ADD_MAPPING(EventLinksFile);
  ADD_MAPPING(IconsFile);
  ADD_MAPPING(ActivitiesFile);
  ADD_MAPPING(InformaticsIconFile);
  ADD_MAPPING(NormalcyMapsFile);
  ADD_MAPPING(ImageTimeMapFile);
  ADD_MAPPING(HomographyIndexFile);
  ADD_MAPPING(FiltersFile);
  ADD_MAPPING(SceneElementsFile);
#undef ADD_MAPPING

  return map;
}

//-----------------------------------------------------------------------------
void vpProject::SetIsValid(const QString& file, FileState state)
{
  if (!file.isEmpty())
    {
    this->FileValidityMap[file] = state;
    }
}

//-----------------------------------------------------------------------------
vpProject::FileState vpProject::IsValid(const QString& file)
{
  if (file.isEmpty())
    {
    return FILE_NAME_EMPTY;
    }

  return this->FileValidityMap.value(file, FILE_NAME_NOT_EMPTY);
}


//-----------------------------------------------------------------------------
void vpProject::CopyConfig(const vpProject& srcProject)
{
  vpProjectBase::operator=(srcProject);

  this->FileValidityMap = srcProject.FileValidityMap;
}
