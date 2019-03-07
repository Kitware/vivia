/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsScenePrivate_h
#define __vsScenePrivate_h

#include <vsEventInfo.h>

#include <vgVideoPlayer.h>

#include <vtkVgInstance.h>
#include <vtkVgTimeStamp.h>

#include <vgColor.h>
#include <vgMatrix.h>

#include <vtkSmartPointer.h>
#include <vtkTimeStamp.h>

#include <QHash>
#include <QPoint>
#include <QSet>
#include <QSharedPointer>

class QVTKWidget;
class vtkActor;
class vtkAssembly;
class vtkImageActor;
class vtkImageData;
class vtkMatrix4x4;
class vtkObject;
class vtkPNGWriter;
class vtkProp;
class vtkRenderer;
class vtkRenderWindow;
class vtkTimerLog;
class vtkWindowToImageFilter;

class vtkVgEvent;
class vtkVgEventInfo;
class vtkVgModelBase;
class vtkVgContourOperatorManager;
class vtkVgRepresentationBase;
class vtkVgTrack;
class vtkVgTrackInfo;
class vtkVgTrackPVOFilter;
class vtkVgTrackModel;
class vtkVgTrackRepresentation;
class vtkVgTrackHeadRepresentation;
class vtkVgTrackLabelRepresentation;
class vtkVgEventModel;
class vtkVgEventFilter;
class vtkVgEventRepresentation;
class vtkVgEventLabelRepresentation;
class vtkVgEventRegionRepresentation;

class vgColor;

class vgMixerWidget;

struct vsTrackInfo;

class vsAlertList;
class vsCore;
class vsContourWidget;
class vsEventDataModel;
class vsEventTreeModel;
class vsEventTreeSelectionModel;
class vsEventTreeWidget;
class vsRegionList;
class vsScene;
class vsSceneTrackColorHelper;
class vsTrackTreeModel;
class vsTrackTreeSelectionModel;
class vsVideoSource;

class vsAbstractSceneTrackColorHelper
{
public:
  virtual ~vsAbstractSceneTrackColorHelper() {}
  virtual const vsTrackInfo* infoForTrack(vtkVgTrack* track) const = 0;
};

class vsScenePrivate
{
protected:
  QTE_DECLARE_PUBLIC_PTR(vsScene)

public:
  struct Update
    {
    Update() : videoUpdated(false) {}
    Update(vtkVgTimeStamp vts) : time(vts), videoUpdated(true) {}

    vtkVgTimeStamp time;
    bool videoUpdated;
    };

  struct Graph
    {
    vtkVgInstance<vtkVgTrackModel> TrackModel;
    vtkVgInstance<vtkVgTrackRepresentation> TrackRepresentation;
    vtkVgInstance<vtkVgTrackHeadRepresentation> TrackHeadRepresentation;
    vtkVgInstance<vtkVgTrackLabelRepresentation> TrackLabelRepresentation;

    vtkVgInstance<vtkVgEventModel> EventModel;
    vtkVgInstance<vtkVgEventRepresentation> EventRepresentation;
    vtkVgInstance<vtkVgEventRegionRepresentation> EventHeadRepresentation;
    vtkVgInstance<vtkVgEventLabelRepresentation> EventLabelRepresentation;

    vtkTimeStamp TrackRepresentationsUpdateTime;
    vtkTimeStamp EventRepresentationsUpdateTime;
    };

  struct ContourInfo
    {
    ContourInfo(vsContourWidget* w = 0) : widget(w), enabled(true) {}

    QSharedPointer<vsContourWidget> widget;
    vtkSmartPointer<vtkImageActor> maskActor;
    bool enabled;
    };

  vsScenePrivate(vsScene* q, vsCore* core);
  ~vsScenePrivate();

  void createFilterEventGroup(const QString& name,
                              vsEventInfo::Group group,
                              vsEventInfo::Group parent = vsEventInfo::All,
                              bool expanded = false);
  void createFilterEventGroup(const QString& name,
                              vsEventInfo::Group group,
                              bool expanded);
  void setupFilterEventGroup(vsEventInfo::Group group,
                             bool visibility = false,
                             double threshold = 0.1);
  void setupFilterEventGroup(const QList<vsEventInfo>& eventTypes,
                             vsEventInfo::Group group,
                             bool visibility = false,
                             double threshold = 0.1);
  void setFilterEventGroupVisibility(vsEventInfo::Group group);
  void addFilter(const vsEventInfo& info, vsEventInfo::Group group,
                 bool visibility = true, double threshold = 0.0);

  void createMaskActor(ContourInfo& info,
                       const vgColor& fcolor, const vgColor& bcolor);

  void createFilterMask(ContourInfo& info);
  void removeFilterMask(ContourInfo& info);

  void createSelectorMask(ContourInfo& info);
  void removeSelectorMask(ContourInfo& info);

  void setAssemblyZRange(vtkAssembly* a, double z1, double z2);
  void updateContourMaskPositions();

  void addFilterContour(ContourInfo& info);
  void removeFilterContour(ContourInfo& info);

  void initializeGraph(Graph&, const char* labelPrefix = 0);

  void setupRepresentations(Graph& graph,
                            float trackHeadWidth, float trackTrailWidth,
                            float eventHeadWidth, float eventTrailWidth);

  bool updateModels(Graph& graph, bool forceUpdate = false);
  bool updateModels(vtkVgModelBase* model, vtkObject* filter,
                    QList<vtkVgRepresentationBase*> modelReps,
                    QList<vtkProp*> insertAfter,
                    vtkTimeStamp& repsUpdateTime,
                    bool forceUpdate = false);

  void updateTrackColors();

  vtkVgTrackInfo trackInfo(vtkIdType trackId);
  vtkVgEventInfo eventInfo(vtkIdType eventId);

  vtkVgTrack* findTrack(vtkIdType trackId);
  vtkVgEvent* findEvent(vtkIdType eventId);

  QString buildLocationTextFromDisplay(int x, int y);

  static vtkSmartPointer<vtkImageData> createDummyImage();
  static vtkSmartPointer<vtkImageData> createFilterMaskImage(
    const double (&bounds)[6]);

  static void setRepresentationTransforms(Graph&, vtkMatrix4x4*);

  const vsAbstractSceneTrackColorHelper* trackColorHelper() const;

  vsCore* const Core;

  vtkVgInstance<vtkRenderWindow> RenderWindow;
  vtkVgInstance<vtkRenderer> Renderer;
  vtkVgInstance<vtkImageActor> ImageActor;

  vgVideoPlayer VideoPlayer;
  vsVideoSource* VideoSource;
  bool SourceIsStreaming;

  vtkVgTimeStamp StreamDelay;

  vtkVgTimeStamp TrackTrailLength;

  QMap<int, vgColor> TrackColors;
  QScopedPointer<vsSceneTrackColorHelper> TrackColorHelper;

  vgColor SelectionColor;
  QSet<vtkVgTrack*> SelectedTracks;
  QSet<vtkVgEvent*> SelectedEvents;

  vgColor FilteringMaskColor;
  vtkVgInstance<vtkAssembly> FilterMaskProps;
  vtkVgInstance<vtkAssembly> SelectorMaskProps;

  vtkVgInstance<vtkActor> SelectorMaskQuad;
  vtkVgInstance<vtkActor> TrackPropsBegin;
  vtkVgInstance<vtkActor> EventPropsBegin;

  vtkVgInstance<vtkImageActor> TrackingMask;

  vtkVgInstance<vtkVgContourOperatorManager> ContourOperatorManager;

  vtkVgInstance<vtkVgTrackPVOFilter> TrackFilter;
  vtkVgInstance<vtkVgEventFilter> EventFilter;

  Graph NormalGraph;
  Graph GroundTruthGraph;
  bool GroundTruthEnabled;

  QScopedPointer<vsContourWidget> EditContour;
  QHash<int, ContourInfo> Contours;
  QHash<int, vsEventInfo> Alerts;

  QVTKWidget* RenderWidget;

  vgMixerWidget* FilterWidget;
  QHash<vsEventInfo::Group, int> FilterGroup;

  vsTrackTreeModel* TrackTreeModel;

  vsEventDataModel* EventDataModel;
  vsEventTreeModel* EventTreeModel;

  vsTrackTreeSelectionModel* TrackTreeSelectionModel;
  vsEventTreeSelectionModel* EventTreeSelectionModel;

  bool NeedViewReset;
  QList<Update> PendingUpdates;
  vtkVgTimeStamp CurrentFrameTime;
  vtkVgTimeStamp CurrentHomographyReferenceTime;
  vgMatrix4d CurrentTransformEigen;
  vtkVgInstance<vtkMatrix4x4> CurrentTransformVtk;
  vtkVgVideoFrameMetaData CurrentFrameMetaData;

  QPoint LastCursorPosition;

  vtkIdType PendingJumpTrackId;
  vtkIdType PendingJumpEventId;

  bool FocusOnTargetEnabled;

  vtkSmartPointer<vtkWindowToImageFilter> WindowToImageFilter;
  vtkSmartPointer<vtkPNGWriter> PngWriter;
  bool WriteRenderedImages;
  QString ImageOutputDirectory;
  vtkIdType ImageCounter;
  QString ScreenShotFileName;
  bool SaveScreenShot;

  double MaxTimeForQuickNote;
  vtkIdType LastManualEventCreated;
  vtkSmartPointer<vtkTimerLog> TimerLog;

private:
  QTE_DECLARE_PUBLIC(vsScene)
  Q_DISABLE_COPY(vsScenePrivate)
};

#endif
