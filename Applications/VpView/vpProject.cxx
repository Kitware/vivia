/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpProject.h"

#include "vpModelIO.h"

#include <vtkVgActivityManager.h>
#include <vtkVgEventIconRepresentation.h>
#include <vtkVgEventModel.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgEventRepresentation.h>
#include <vtkVgIconManager.h>
#include <vtkVgPicker.h>
#include <vtkVgTrackHeadRepresentation.h>
#include <vtkVgTrackModel.h>
#include <vtkVgTrackRepresentation.h>

//-----------------------------------------------------------------------------
vpProject::vpProject(int id) :
  OverviewFileTag("OverviewFile"),
  DataSetSpecifierTag("DataSetSpecifier"),
  TracksFileTag("TracksFile"),
  TrackTraitsFileTag("TrackTraitsFile"),
  EventsFileTag("EventsFile"),
  EventLinksFileTag("EventLinksFile"),
  IconsFileTag("IconsFile"),
  ActivitiesFileTag("ActivitiesFile"),
  InformaticsIconFileTag("InformaticsIconFile"),
  NormalcyMapsFileTag("NormalcyMapsFile"),
  PrecomputeActivityTag("PrecomputeActivity"),
  OverviewSpacingTag("OverviewSpacing"),
  OverviewOriginTag("OverviewOrigin"),
  AnalysisDimensionsTag("AnalysisDimensions"),
  AOIUpperLeftLatLonTag("AOIUpperLeftLatLon"),
  AOIUpperRightLatLonTag("AOIUpperRightLatLon"),
  AOILowerLeftLatLonTag("AOILowerLeftLatLon"),
  AOILowerRightLatLonTag("AOILowerRightLatLon"),
  ColorWindowTag("ColorWindow"),
  ColorLevelTag("ColorLevel"),
  ColorMultiplierTag("ColorMultiplier"),
  TrackColorOverrideTag("TrackColorOverride"),
  FrameNumberOffsetTag("FrameNumberOffset"),
  ImageTimeMapFileTag("ImageTimeMapFile"),
  HomographyIndexFileTag("HomographyIndexFile"),
  FiltersFileTag("FiltersFile"),
  SceneElementsFileTag("SceneElementsFile"),
  ImageToGcsMatrixTag("ImageToGcsMatrix"),
  ProjectId(id),
  NextCreateTrackId(3000000),
  IsVisible(true)
{
  // Internal storage. This is useful as we are going to iterate over the filenames.
  TagFileMap.insert(
    std::make_pair(this->OverviewFileTag, &this->OverviewFile));
  TagFileMap.insert(
    std::make_pair(this->DataSetSpecifierTag, &this->DataSetSpecifier));
  TagFileMap.insert(
    std::make_pair(this->TracksFileTag, &this->TracksFile));
  TagFileMap.insert(
    std::make_pair(this->TrackTraitsFileTag, &this->TrackTraitsFile));
  TagFileMap.insert(
    std::make_pair(this->EventsFileTag, &this->EventsFile));
  TagFileMap.insert(
    std::make_pair(this->EventLinksFileTag, &this->EventLinksFile));
  TagFileMap.insert(
    std::make_pair(this->IconsFileTag, &this->IconsFile));
  TagFileMap.insert(
    std::make_pair(this->ActivitiesFileTag, &this->ActivitiesFile));
  TagFileMap.insert(
    std::make_pair(this->InformaticsIconFileTag, &this->InformaticsIconFile));
  TagFileMap.insert(
    std::make_pair(this->NormalcyMapsFileTag, &this->NormalcyMapsFile));
  TagFileMap.insert(
    std::make_pair(this->ImageTimeMapFileTag, &this->ImageTimeMapFile));
  TagFileMap.insert(
    std::make_pair(this->HomographyIndexFileTag, &this->HomographyIndexFile));
  TagFileMap.insert(
    std::make_pair(this->FiltersFileTag, &this->FiltersFile));
  TagFileMap.insert(
    std::make_pair(this->SceneElementsFileTag, &this->SceneElementsFile));

  this->PrecomputeActivity = 0;

  this->OverviewSpacing = 1.0;
  this->OverviewOrigin[0] = this->OverviewOrigin[1] = 0.0;
  this->AnalysisDimensions[0] = this->AnalysisDimensions[1] = 0.0;

  this->AOIUpperLeftLatLon[0] = this->AOIUpperLeftLatLon[1] = 444;
  this->AOIUpperRightLatLon[0] = this->AOIUpperRightLatLon[1] = 444;
  this->AOILowerLeftLatLon[0] = this->AOILowerLeftLatLon[1] = 444;
  this->AOILowerRightLatLon[0] = this->AOILowerRightLatLon[1] = 444;

  this->ColorWindow = 255.0;
  this->ColorLevel = 127.5;

  this->ColorMultiplier = 1.0;
  this->HasTrackColorOverride = false;

  this->FrameNumberOffset = 0;

  this->TrackModel = vtkSmartPointer<vtkVgTrackModel>::New();
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
void vpProject::SetIsValid(const std::string& file, int state)
{
  if (!file.empty())
    {
    this->FileValidityMap[file] = state;
    }
}

//-----------------------------------------------------------------------------
int vpProject::IsValid(const std::string& file)
{
  if (file.empty())
    {
    return FILE_NAME_EMPTY;
    }

  vpProject::FileValidityMapConstItr itr = this->FileValidityMap.find(file);
  if (itr != this->FileValidityMap.end())
    {
    return itr->second;
    }

  return FILE_NAME_NOT_EMPTY;
}


//-----------------------------------------------------------------------------
void vpProject::CopyConfig(const vpProject& srcProject)
{
  this->ConfigFileStem = srcProject.ConfigFileStem;
  this->OverviewFile = srcProject.ConfigFileStem;
  this->DataSetSpecifier = srcProject.DataSetSpecifier;
  this->TracksFile = srcProject.TracksFile;
  this->TrackTraitsFile = srcProject.TrackTraitsFile;
  this->EventsFile = srcProject.EventsFile;
  this->EventLinksFile = srcProject.EventLinksFile;
  this->IconsFile = srcProject.IconsFile;
  this->ActivitiesFile = srcProject.ActivitiesFile;
  this->InformaticsIconFile = srcProject.InformaticsIconFile;
  this->NormalcyMapsFile = srcProject.NormalcyMapsFile;
  this->ImageTimeMapFile = srcProject.ImageTimeMapFile;
  this->HomographyIndexFile = srcProject.HomographyIndexFile;
  this->FiltersFile = srcProject.FiltersFile;
  this->SceneElementsFile = srcProject.SceneElementsFile;

  this->PrecomputeActivity = srcProject.PrecomputeActivity;

  this->OverviewSpacing = srcProject.OverviewSpacing;
  this->OverviewOrigin[0] = srcProject.OverviewOrigin[0];
  this->OverviewOrigin[1] = srcProject.OverviewOrigin[1];

  this->ColorWindow = srcProject.ColorWindow;
  this->ColorLevel = srcProject.ColorLevel;

  this->ColorMultiplier = srcProject.ColorMultiplier;
  this->FrameNumberOffset = srcProject.FrameNumberOffset;

  this->HasTrackColorOverride = srcProject.HasTrackColorOverride;
  this->TrackColorOverride[0] = srcProject.TrackColorOverride[0];
  this->TrackColorOverride[1] = srcProject.TrackColorOverride[1];
  this->TrackColorOverride[2] = srcProject.TrackColorOverride[2];

  this->AOIUpperLeftLatLon[0] = srcProject.AOIUpperLeftLatLon[0];
  this->AOIUpperLeftLatLon[1] = srcProject.AOIUpperLeftLatLon[1];
  this->AOIUpperRightLatLon[0] = srcProject.AOIUpperRightLatLon[0];
  this->AOIUpperRightLatLon[1] = srcProject.AOIUpperRightLatLon[1];
  this->AOILowerLeftLatLon[0] = srcProject.AOILowerLeftLatLon[0];
  this->AOILowerLeftLatLon[1] = srcProject.AOILowerLeftLatLon[1];
  this->AOILowerRightLatLon[0] = srcProject.AOILowerRightLatLon[0];
  this->AOILowerRightLatLon[1] = srcProject.AOILowerRightLatLon[1];

  this->AnalysisDimensions[0] = srcProject.AnalysisDimensions[0];
  this->AnalysisDimensions[1] = srcProject.AnalysisDimensions[1];

  this->TagFileMap = srcProject.TagFileMap;
  this->FileValidityMap = srcProject.FileValidityMap;
}
