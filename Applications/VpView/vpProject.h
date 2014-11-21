/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpProject_h
#define __vpProject_h

#include <QSharedPointer>

#include <vtkSmartPointer.h>

#include <string>
#include <map>
#include <vector>
#include <algorithm>

class vtkMatrix4x4;

class vpModelIO;

class vtkVgActivityManager;
class vtkVgEventIconRepresentation;
class vtkVgEventModel;
class vtkVgEventRegionRepresentation;
class vtkVgEventRepresentation;
class vtkVgIconManager;
class vtkVgPicker;
class vtkVgTrackHeadRepresentation;
class vtkVgTrackModel;
class vtkVgTrackRepresentation;
class vtkVpReaderBase;

class vpProject
{
public:
  enum FileState
    {
    FILE_NOT_EXIST  = 0,
    FILE_NAME_EMPTY = 1,
    FILE_EXIST      = 2,
    FILE_NAME_NOT_EMPTY = 3
    };

  // Description:
  // Constructor / Destructor.
  vpProject(int id);
  ~vpProject();

  // Description:
  // Set if a file is valid (if it exists on the system).
  void SetIsValid(const std::string& file, int state);

  // Description:
  // Check if a given file is valid or not.
  int IsValid(const std::string& file);

  void CopyConfig(const vpProject& srcProject);

  // Description:
  // Keep track of file path stem for finding other files
  std::string ConfigFileStem;

  // Description:
  // Various file sources used for the data.
  std::string OverviewFile;
  std::string DataSetSpecifier;
  std::string TracksFile;
  std::string TrackTraitsFile;
  std::string EventsFile;
  std::string EventLinksFile;
  std::string IconsFile;
  std::string ActivitiesFile;
  std::string InformaticsIconFile;
  std::string NormalcyMapsFile;
  std::string ImageTimeMapFile;
  std::string HomographyIndexFile;
  std::string FiltersFile;
  std::string SceneElementsFile;

  std::string OverviewFileTag;
  std::string DataSetSpecifierTag;
  std::string TracksFileTag;
  std::string TrackTraitsFileTag;
  std::string EventsFileTag;
  std::string EventLinksFileTag;
  std::string IconsFileTag;
  std::string ActivitiesFileTag;
  std::string InformaticsIconFileTag;
  std::string NormalcyMapsFileTag;
  std::string PrecomputeActivityTag;

  std::string OverviewSpacingTag;
  std::string OverviewOriginTag;
  std::string AnalysisDimensionsTag;

  std::string AOIUpperLeftLatLonTag;
  std::string AOIUpperRightLatLonTag;
  std::string AOILowerLeftLatLonTag;
  std::string AOILowerRightLatLonTag;

  std::string ColorWindowTag;
  std::string ColorLevelTag;

  std::string ColorMultiplierTag;
  std::string TrackColorOverrideTag;

  std::string FrameNumberOffsetTag;
  std::string ImageTimeMapFileTag;
  std::string HomographyIndexFileTag;

  std::string FiltersFileTag;
  std::string SceneElementsFileTag;

  std::string ImageToGcsMatrixTag;

  int    PrecomputeActivity;

  double OverviewSpacing;
  double OverviewOrigin [2];

  double AOIUpperLeftLatLon[2];
  double AOIUpperRightLatLon[2];
  double AOILowerLeftLatLon[2];
  double AOILowerRightLatLon[2];

  double AnalysisDimensions[2];

  double ColorWindow;
  double ColorLevel;

  double ColorMultiplier;

  bool HasTrackColorOverride;
  double TrackColorOverride[3];

  int FrameNumberOffset;

  // Description:
  // Useful data structure.
  std::map<std::string, std::string*> TagFileMap;
  typedef std::map<std::string, std::string*>::const_iterator TagFileMapItr;

  std::map<std::string, int> FileValidityMap;
  typedef std::map<std::string, int>::const_iterator FileValidityMapConstItr;

  // Description:
  // Project specific models and representations
  vtkSmartPointer<vtkVgTrackModel> TrackModel;
  vtkSmartPointer<vtkVgTrackRepresentation> TrackRepresentation;
  vtkSmartPointer<vtkVgTrackRepresentation> SelectedTrackRepresentation;
  vtkSmartPointer<vtkVgTrackHeadRepresentation> TrackHeadRepresentation;
  vtkSmartPointer<vtkVgTrackHeadRepresentation> SelectedTrackHeadRepresentation;

  vtkSmartPointer<vtkVgEventModel> EventModel;
  vtkSmartPointer<vtkVgEventRepresentation> EventRepresentation;
  vtkSmartPointer<vtkVgEventRepresentation> SelectedEventRepresentation;
  vtkSmartPointer<vtkVgEventIconRepresentation> EventIconRepresentation;
  vtkSmartPointer<vtkVgEventRegionRepresentation> EventRegionRepresentation;

  vtkSmartPointer<vtkVgActivityManager> ActivityManager;

  vtkSmartPointer<vtkVgTrackHeadRepresentation> SceneElementRepresentation;
  vtkSmartPointer<vtkVgTrackHeadRepresentation> SelectedSceneElementRepresentation;

  vtkSmartPointer<vtkVgIconManager> IconManager;

  // Description:
  // Project specific transformation matrices
  double ImageToGcsMatrixArray[18];
  vtkSmartPointer<vtkMatrix4x4> ImageToGcsMatrix;

  // Description:
  // Others
  vtkSmartPointer<vtkVgPicker> Picker;
  QSharedPointer<vpModelIO> ModelIO;

  int ProjectId;
  int NextCreateTrackId;

  bool IsVisible;

  std::string Name;

private:
  vpProject(const vpProject& src);        // Not implemented.
  void operator=(const vpProject& src);   // Not implemented.
};

#endif // __vpProject_h
