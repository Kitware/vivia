/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpProject_h
#define __vpProject_h

#include "vpProjectBase.h"

#include <qtGlobal.h>

#include <vtkSmartPointer.h>

#include <QHash>
#include <QSharedPointer>

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
class vtkVgTrackRepresentation;

class vtkVpReaderBase;
class vtkVpTrackModel;

class vpProject : public vpProjectBase
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
  void SetIsValid(const QString& file, FileState state);

  // Description:
  // Check if a given file is valid or not.
  FileState IsValid(const QString& file);

  void CopyConfig(const vpProject& srcProject);

#define PROJECT_FIELD_TAG(x) static constexpr const char* x##Tag = #x
  PROJECT_FIELD_TAG(OverviewFile);
  PROJECT_FIELD_TAG(DataSetSpecifier);
  PROJECT_FIELD_TAG(TracksFile);
  PROJECT_FIELD_TAG(TrackTraitsFile);
  PROJECT_FIELD_TAG(EventsFile);
  PROJECT_FIELD_TAG(EventLinksFile);
  PROJECT_FIELD_TAG(IconsFile);
  PROJECT_FIELD_TAG(ActivitiesFile);
  PROJECT_FIELD_TAG(InformaticsIconFile);
  PROJECT_FIELD_TAG(NormalcyMapsFile);
  PROJECT_FIELD_TAG(PrecomputeActivity);

  PROJECT_FIELD_TAG(OverviewSpacing);
  PROJECT_FIELD_TAG(OverviewOrigin);
  PROJECT_FIELD_TAG(AnalysisDimensions);

  PROJECT_FIELD_TAG(AOI);
  PROJECT_FIELD_TAG(AOIUpperLeftLatLon); // DEPRECATED (vidtk reader only)
  PROJECT_FIELD_TAG(AOIUpperRightLatLon); // DEPRECATED (vidtk reader only)
  PROJECT_FIELD_TAG(AOILowerLeftLatLon); // DEPRECATED (vidtk reader only)
  PROJECT_FIELD_TAG(AOILowerRightLatLon); // DEPRECATED (vidtk reader only)

  PROJECT_FIELD_TAG(ColorWindow);
  PROJECT_FIELD_TAG(ColorLevel);

  PROJECT_FIELD_TAG(ColorMultiplier);
  PROJECT_FIELD_TAG(TrackColorOverride);

  PROJECT_FIELD_TAG(FrameNumberOffset);
  PROJECT_FIELD_TAG(ImageTimeMapFile);
  PROJECT_FIELD_TAG(HomographyIndexFile);

  PROJECT_FIELD_TAG(FiltersFile);
  PROJECT_FIELD_TAG(SceneElementsFile);

  PROJECT_FIELD_TAG(ImageToGcsMatrix);
#undef PROJECT_FIELD_TAG

  const QHash<QString, QString*> TagFileMap;

  QHash<QString, FileState> FileValidityMap;

  // Description:
  // Project specific models and representations
  vtkSmartPointer<vtkVpTrackModel> TrackModel;
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
  vtkSmartPointer<vtkMatrix4x4> ImageToGcsMatrix;

  // Description:
  // Others
  vtkSmartPointer<vtkVgPicker> Picker;
  QSharedPointer<vpModelIO> ModelIO;

  int ProjectId;
  int NextCreateTrackId;

  bool IsVisible;

  QString Name;

protected:
  static QHash<QString, QString*> BuildTagFileMap(vpProjectBase* base);

private:
  QTE_DISABLE_COPY(vpProject);
};

#endif
