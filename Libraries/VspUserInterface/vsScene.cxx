// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsScene.h"
#include "vsScenePrivate.h"

#include "vsAlertList.h"
#include "vsContourWidget.h"
#include "vsCore.h"
#include "vsDebug.h"
#include "vsEventDataModel.h"
#include "vsEventRatingMenu.h"
#include "vsEventTreeModel.h"
#include "vsEventTreeSelectionModel.h"
#include "vsEventTreeView.h"
#include "vsEventTreeWidget.h"
#include "vsRegionList.h"
#include "vsSettings.h"
#include "vsTrackInfo.h"
#include "vsTrackTreeModel.h"
#include "vsTrackTreeSelectionModel.h"
#include "vsTrackTreeWidget.h"

#include <vsDisplayInfo.h>

#include <vtkVgQtUtil.h>

#include <vtkVgEventLabelRepresentation.h>
#include <vtkVgEventModel.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgEventRepresentation.h>
#include <vtkVgTrackPVOFilter.h>
#include <vtkVgTrackHeadRepresentation.h>
#include <vtkVgTrackLabelRepresentation.h>
#include <vtkVgTrackModel.h>
#include <vtkVgTrackRepresentation.h>

#include <vtkVgAdapt.h>
#include <vtkVgContourOperatorManager.h>
#include <vtkVgEvent.h>
#include <vtkVgEventFilter.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVgInteractorStyleRubberBand2D.h>
#include <vtkVgSpaceConversion.h>
#include <vtkVgTrack.h>
#include <vtkVgUtil.h>

#include <vgMixerWidget.h>
#include <vgTextEditDialog.h>

#include <vgCheckArg.h>
#include <vgRange.h>

#include <vtkAssembly.h>
#include <vtkCamera.h>
#include <vtkImageActor.h>
#include <vtkImageProperty.h>
#include <vtkLookupTable.h>
#include <vtkPoints.h>
#include <vtkProp3DCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkTimerLog.h>
#include <vtkWindowToImageFilter.h>
#include <QVTKWidget.h>

#include <qtGradient.h>
#include <qtStlUtil.h>

#include <QApplication>
#include <QClipboard>
#include <QMenu>

#include <Eigen/LU>

QTE_IMPLEMENT_D_FUNC(vsScene)

#define FORWARD_SIGNAL(_so, _sn, _dn, _p) \
  connect(_so, SIGNAL(_sn _p), this, SIGNAL(_dn _p))

//-----------------------------------------------------------------------------
vsScene::vsScene(vsCore* core, QObject* parent)
  : QObject(parent), d_ptr(new vsScenePrivate(this, core))
{
  QTE_D(vsScene);

  // Initialize the homography reference frame to MinTime to handle case of KWA
  // older than v3 for which we don't have reference frame info, and thus want
  // to ignore the reference frame (which setting to MinTime will do)
  d->CurrentHomographyReferenceTime.SetToMinTime();

  // Connect our outputs to core inputs
  connect(this, SIGNAL(contourCompleted(vsContour)),
          core, SLOT(addContour(vsContour)));

  // Connect to core outputs
  connect(core, SIGNAL(updated()), this, SLOT(postUpdate()));

  connect(core, SIGNAL(videoSourceChanged(vsVideoSource*)),
          this, SLOT(resetVideo(vsVideoSource*)));

  connect(core, SIGNAL(contourAdded(vsContour)),
          this, SLOT(addContour(vsContour)));
  connect(core, SIGNAL(contourTypeChanged(int, vsContour::Type)),
          this, SLOT(setContourType(int, vsContour::Type)));
  connect(core, SIGNAL(contourPointsChanged(int, QPolygonF)),
          this, SLOT(setContourPoints(int, QPolygonF)));
  connect(core, SIGNAL(contourRemoved(int)),
          this, SLOT(removeContour(int)));

  connect(core, SIGNAL(userEventTypeAdded(int, vsEventInfo, double)),
          this, SLOT(addUserEventType(int, vsEventInfo, double)));

  connect(core, SIGNAL(alertAdded(int, vsAlert)),
          this, SLOT(addAlert(int, vsAlert)));
  connect(core, SIGNAL(alertChanged(int, vsAlert)),
          this, SLOT(updateAlert(int, vsAlert)));

  connect(core, SIGNAL(manualEventCreated(vtkIdType)),
          this, SLOT(startQuickNoteTimer(vtkIdType)));

  connect(core, SIGNAL(trackNoteChanged(vtkVgTrack*, QString)),
          this, SLOT(updateTrackNotes()));
  connect(core, SIGNAL(eventNoteChanged(vtkVgEvent*, QString)),
          this, SLOT(updateEventNotes()));

  // Connect video player
  connect(&d->VideoPlayer, SIGNAL(frameAvailable(vtkVgVideoFrame, qint64)),
          this, SLOT(updateVideoFrame(vtkVgVideoFrame, qint64)));
  FORWARD_SIGNAL(&d->VideoPlayer, playbackSpeedChanged, playbackStatusChanged,
                 (vgVideoPlayer::PlaybackMode, qreal));
  FORWARD_SIGNAL(&d->VideoPlayer, seekRequestDiscarded,
                 videoSeekRequestDiscarded, (qint64, vtkVgTimeStamp));
  d->VideoPlayer.setDebugArea(vsdVideoPlayback);

  // Set default track trail length (will trigger an update)
  this->setTrackTrailLength(4.0);

  d->TimerLog = vtkSmartPointer<vtkTimerLog>::New();
  d->LastManualEventCreated = -1;
}

//-----------------------------------------------------------------------------
vsScene::~vsScene()
{
}

//-----------------------------------------------------------------------------
void vsScene::setupUi(QVTKWidget* renderWidget)
{
  QTE_D(vsScene);

  d->RenderWidget = renderWidget;

  // Everything in the scene should fall between z = (-1.0, 1.0). Exploit this
  // to improve our depth buffer resolution.
  d->Renderer->GetActiveCamera()->ParallelProjectionOn();
  d->Renderer->GetActiveCamera()->SetClippingRange(1.0, 3.0);
  d->Renderer->GetActiveCamera()->SetPosition(0.0, 0.0, 2.0);

  // Set up render pipeline
  d->Renderer->SetBackground(0, 0, 0);
  d->RenderWindow->AddRenderer(d->Renderer);
  renderWidget->SetRenderWindow(d->RenderWindow);

  // Don't allow camera rotation since it confuses contour drawing
  vtkVgInteractorStyleRubberBand2D* style =
    vtkVgInteractorStyleRubberBand2D::New();
  style->SetRenderer(d->Renderer);
  renderWidget->GetInteractor()->SetInteractorStyle(style);
  style->FastDelete();

  // Listen for 'pick' events, and connect context menu and position display
  vtkConnect(style, vtkVgInteractorStyleRubberBand2D::LeftClickEvent,
             this, SLOT(onLeftClick()));
  vtkConnect(style, vtkVgInteractorStyleRubberBand2D::RightClickEvent,
             this, SLOT(onRightClick()));
  vtkConnect(renderWidget->GetInteractor(), vtkCommand::MouseMoveEvent,
             this, SLOT(vtkSceneMouseMoveEvent()));
  vtkConnect(renderWidget->GetInteractor(), vtkCommand::LeaveEvent,
             this, SLOT(vtkSceneLeaveEvent()));

  // Set up video display (must feed a dummy initial frame)
  d->ImageActor->SetInputData(vsScenePrivate::createDummyImage());
  d->Renderer->AddViewProp(d->ImageActor);
  d->ImageActor->SetPosition(0.0, 0.0, -0.2);

  // Set up track and event display
  vsSettings settings;
  d->setupRepresentations(d->NormalGraph,
                          settings.normalTrackHeadWidth(),
                          settings.normalTrackTrailWidth(),
                          settings.normalEventHeadWidth(),
                          settings.normalEventTrailWidth());
  d->setupRepresentations(d->GroundTruthGraph,
                          settings.groundTruthTrackHeadWidth(),
                          settings.groundTruthTrackTrailWidth(),
                          settings.groundTruthEventHeadWidth(),
                          settings.groundTruthEventTrailWidth());

  // Add to renderer; tracks go *after* events so that translucent events will
  // mask underlying tracks...
  d->Renderer->AddViewProp(d->EventPropsBegin);
  d->Renderer->AddViewProp(d->TrackPropsBegin);

  // Add mask props (in order of rendering)
  //
  // NOTE: To avoid rendering artifacts, newer filter masks are positioned
  // further from the camera than older ones, and selector masks must be closer
  // to the camera than the mask quad; depth testing will be enabled for
  // this geometry even though masks may be translucent
  d->Renderer->AddViewProp(d->FilterMaskProps);
  d->Renderer->AddViewProp(d->SelectorMaskProps);
  d->Renderer->AddViewProp(d->SelectorMaskQuad);
  d->Renderer->AddViewProp(d->TrackingMask);
  d->SelectorMaskQuad->SetPosition(0.0, 0.0, -0.15);

  // setup outputing images from render window
  d->WindowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
  d->WindowToImageFilter->SetInput(d->RenderWindow);
  d->PngWriter = vtkSmartPointer<vtkPNGWriter>::New();
  d->PngWriter->SetInputConnection(d->WindowToImageFilter->GetOutputPort());

  vtkConnect(d->RenderWindow, vtkCommand::RenderEvent,
             this, SLOT(writeRenderedImages()));
}

//-----------------------------------------------------------------------------
void vsScene::setupFilterWidget(vgMixerWidget* filterWidget)
{
  QTE_D(vsScene);

  d->FilterWidget = filterWidget;

  // Connect signals/slots first so changes to defaults are seen
  connect(filterWidget, SIGNAL(stateChanged(int, bool)),
          this, SLOT(setEventVisibility(int, bool)));
  connect(filterWidget, SIGNAL(valueChanged(int, double)),
          this, SLOT(setEventThreshold(int, double)));
  connect(filterWidget, SIGNAL(invertedChanged(int, bool)),
          this, SLOT(setEventThresholdInverted(int, bool)));

  // Set up object type filters
  int groupId = filterWidget->addGroup("Object Type");
  filterWidget->setExpanded(groupId);
  foreach (vsTrackInfo ti, vsTrackInfo::trackTypes())
    {
    if (ti.id != vsTrackInfo::Unclassified)
      filterWidget->addItem(ti.id, ti.name, groupId);
    }

  // Set up groups for event type filters
  d->createFilterEventGroup("Event Type", vsEventInfo::All, true);
  d->createFilterEventGroup("Classifier", vsEventInfo::Classifier);
  d->createFilterEventGroup("User Defined", vsEventInfo::User);
  d->createFilterEventGroup("Alert", vsEventInfo::Alert);
  d->createFilterEventGroup("General", vsEventInfo::General);
  d->createFilterEventGroup("Person", vsEventInfo::Person,
                            vsEventInfo::Classifier);
  d->createFilterEventGroup("Vehicle", vsEventInfo::Vehicle,
                            vsEventInfo::Classifier);

  // Set up (built-in) event type filters
  d->setupFilterEventGroup(vsEventInfo::Person);
  d->setupFilterEventGroup(vsEventInfo::Vehicle);
  d->setupFilterEventGroup(vsEventInfo::General, true);
  d->setupFilterEventGroup(d->Core->manualEventTypes(),
                           vsEventInfo::User, true);

  // Show filters when needed
  d->setFilterEventGroupVisibility(vsEventInfo::Classifier);
  d->setFilterEventGroupVisibility(vsEventInfo::User);
  d->setFilterEventGroupVisibility(vsEventInfo::Alert);
  d->setFilterEventGroupVisibility(vsEventInfo::General);

  connect(d->Core, SIGNAL(eventGroupExpected(vsEventInfo::Group)),
          this, SLOT(showEventGroupFilter(vsEventInfo::Group)));
  connect(d->Core, SIGNAL(alertRemoved(int, bool)),
          this, SLOT(removeAlert(int, bool)));
}

//-----------------------------------------------------------------------------
void vsScene::setupRegionList(vsRegionList* list)
{
  QTE_D(vsScene);

  vsCore* core = d->Core; // just so connections look neater

  connect(core, SIGNAL(contourAdded(vsContour)),
          list, SLOT(addRegion(vsContour)));
  connect(core, SIGNAL(contourNameChanged(int, QString)),
          list, SLOT(setRegionName(int, QString)));
  connect(core, SIGNAL(contourTypeChanged(int, vsContour::Type)),
          list, SLOT(setRegionType(int, vsContour::Type)));
  connect(core, SIGNAL(contourRemoved(int)),
          list, SLOT(removeRegion(int)));

  connect(list, SIGNAL(regionNameChanged(int, QString)),
          core, SLOT(setContourName(int, QString)));
  connect(list, SIGNAL(regionTypeChanged(int, vsContour::Type)),
          core, SLOT(setContourType(int, vsContour::Type)));
  connect(list, SIGNAL(regionVisibilityChanged(int, bool)),
          this, SLOT(setContourEnabled(int, bool)));
  connect(list, SIGNAL(regionRemoved(int)),
          core, SLOT(removeContour(int)));

  connect(core, SIGNAL(manualEventTypesUpdated(QList<vsEventInfo>)),
          list, SLOT(setEventTypes(QList<vsEventInfo>)));
  connect(list, SIGNAL(regionConvertedToEvent(int, int)),
          this, SLOT(convertContourToEvent(int, int)));
}

//-----------------------------------------------------------------------------
void vsScene::setupAlertList(vsAlertList* list)
{
  QTE_D(vsScene);

  vsCore* core = d->Core; // just so connections look neater

  list->setSwatchCache(core->swatchCache());

  connect(core, SIGNAL(alertAdded(int, vsAlert)),
          list, SLOT(addAlert(int, vsAlert)));
  connect(core, SIGNAL(alertChanged(int, vsAlert)),
          list, SLOT(updateAlert(int, vsAlert)));
  connect(core, SIGNAL(alertMatchesChanged(int, int)),
          list, SLOT(setAlertMatches(int, int)));
  connect(core, SIGNAL(alertEnabledChanged(int, bool)),
          list, SLOT(setAlertEnabled(int, bool)));
  connect(core, SIGNAL(alertRemoved(int, bool)),
          list, SLOT(removeAlert(int)));

  connect(list, SIGNAL(alertChanged(int, vsAlert)),
          core, SLOT(updateAlert(int, vsAlert)));
  connect(list, SIGNAL(alertActivationChanged(int, bool)),
          core, SLOT(setAlertEnabled(int, bool)));
  connect(list, SIGNAL(alertRemoved(int)),
          core, SLOT(removeAlert(int)));

  connect(this, SIGNAL(alertThresholdChanged(int, double)),
          list, SLOT(updateAlertThreshold(int, double)));
  connect(list, SIGNAL(alertChanged(int, vsAlert)),
          this, SLOT(updateAlertThreshold(int, vsAlert)));
}

//-----------------------------------------------------------------------------
void vsScene::setupEventTree(
  vsEventTreeWidget* eventTree,
  vsEventTreeWidget* verifiedTree,
  vsEventTreeWidget* rejectedTree)
{
  QTE_D(vsScene);

  d->EventTreeModel =
    new vsEventTreeModel(this, d->EventFilter, d->Core->eventTypeRegistry(),
                         d->Core->swatchCache(), this);
  d->EventTreeSelectionModel =
    new vsEventTreeSelectionModel(d->EventTreeModel, this);

  connect(d->EventTreeModel, SIGNAL(eventRatingChanged(vtkIdType, int)),
          d->Core, SLOT(setEventRating(vtkIdType, int)));
  connect(d->EventTreeModel, SIGNAL(eventStatusChanged(vtkIdType, int)),
          d->Core, SLOT(setEventStatus(vtkIdType, int)));
  connect(d->EventTreeModel, SIGNAL(eventNoteChanged(vtkIdType, QString)),
          d->Core, SLOT(setEventNote(vtkIdType, QString)));
  connect(d->Core, SIGNAL(eventAdded(vtkVgEvent*, vsEventId)),
          d->EventTreeModel, SLOT(addEvent(vtkVgEvent*)));
  connect(d->Core, SIGNAL(eventChanged(vtkVgEvent*)),
          d->EventTreeModel, SLOT(updateEvent(vtkVgEvent*)));
  connect(d->Core, SIGNAL(eventRemoved(vtkIdType)),
          d->EventTreeModel, SLOT(removeEvent(vtkIdType)));
  connect(d->Core, SIGNAL(eventStatusChanged(vtkVgEvent*, int)),
          this, SLOT(setEventStatus(vtkVgEvent*, int)));
  connect(d->Core, SIGNAL(eventRatingChanged(vtkVgEvent*, int)),
          d->EventTreeModel, SLOT(updateEvent(vtkVgEvent*)));

  connect(d->EventTreeSelectionModel, SIGNAL(selectionChanged(QSet<vtkIdType>)),
          this, SLOT(updateEventSelection(QSet<vtkIdType>)));

  rejectedTree->setHiddenItemsShown(true);

  this->setupEventTree(eventTree, vs::UnverifiedEvent);
  this->setupEventTree(verifiedTree, vs::VerifiedEvent);
  this->setupEventTree(rejectedTree, vs::RejectedEvent);
}

//-----------------------------------------------------------------------------
void vsScene::setupEventTree(vsEventTreeWidget* tree, int type)
{
  QTE_D(vsScene);

  tree->setStatusFilter(static_cast<vsEventStatus>(type));
  tree->setModel(d->EventTreeModel);
  tree->setSelectionModel(d->EventTreeSelectionModel);

  connect(tree, SIGNAL(jumpToEvent(vtkIdType, bool)),
          this, SLOT(jumpToEvent(vtkIdType, bool)));

  connect(tree, SIGNAL(setEventStartRequested(vtkIdType)),
          this, SLOT(setEventStart(vtkIdType)));
  connect(tree, SIGNAL(setEventEndRequested(vtkIdType)),
          this, SLOT(setEventEnd(vtkIdType)));
}

//-----------------------------------------------------------------------------
void vsScene::setupTrackTree(vsTrackTreeWidget* tree)
{
  QTE_D(vsScene);

  d->TrackTreeModel =
    new vsTrackTreeModel(d->Core, this, d->TrackFilter, this);
  d->TrackTreeSelectionModel =
    new vsTrackTreeSelectionModel(d->TrackTreeModel, this);

  connect(d->Core, SIGNAL(trackAdded(vtkVgTrack*)),
          d->TrackTreeModel, SLOT(addTrack(vtkVgTrack*)));
  connect(d->Core, SIGNAL(trackChanged(vtkVgTrack*)),
          d->TrackTreeModel, SLOT(updateTrack(vtkVgTrack*)));
  connect(d->TrackTreeModel, SIGNAL(trackNoteChanged(vtkIdType, QString)),
          d->Core, SLOT(setTrackNote(vtkIdType, QString)));
  connect(tree, SIGNAL(jumpToTrack(vtkIdType, bool)),
          this, SLOT(jumpToTrack(vtkIdType, bool)));
  connect(this, SIGNAL(pickedTrack(vtkIdType)),
          tree, SLOT(selectTrack(vtkIdType)));
  connect(tree, SIGNAL(trackFollowingRequested(vtkIdType)),
          d->Core, SLOT(startFollowingTrack(vtkIdType)));
  connect(tree, SIGNAL(trackFollowingCanceled()),
          d->Core, SLOT(cancelFollowing()));

  connect(d->TrackTreeSelectionModel, SIGNAL(selectionChanged(QSet<vtkIdType>)),
          this, SLOT(updateTrackSelection(QSet<vtkIdType>)));

  tree->setModel(d->TrackTreeModel);
  tree->setSelectionModel(d->TrackTreeSelectionModel);
}

//-----------------------------------------------------------------------------
QAbstractItemModel* vsScene::eventDataModel()
{
  QTE_D(vsScene);

  if (!d->EventDataModel)
    {
    d->EventDataModel = new vsEventDataModel(this, this);

    connect(d->Core, SIGNAL(eventAdded(vtkVgEvent*, vsEventId)),
            d->EventDataModel, SLOT(addEvent(vtkVgEvent*, vsEventId)));
    connect(d->Core, SIGNAL(eventChanged(vtkVgEvent*)),
            d->EventDataModel, SLOT(updateEvent(vtkVgEvent*)));
    connect(d->Core, SIGNAL(eventRemoved(vtkIdType)),
            d->EventDataModel, SLOT(removeEvent(vtkIdType)));

    connect(d->Core, SIGNAL(eventStatusChanged(vtkVgEvent*, int)),
            d->EventDataModel, SLOT(updateEvent(vtkVgEvent*)));
    connect(d->Core, SIGNAL(eventRatingChanged(vtkVgEvent*, int)),
            d->EventDataModel, SLOT(updateEvent(vtkVgEvent*)));
    connect(d->Core, SIGNAL(eventNoteChanged(vtkVgEvent*, QString)),
            d->EventDataModel, SLOT(updateEvent(vtkVgEvent*)));
    }

  return d->EventDataModel;
}

//-----------------------------------------------------------------------------
void vsScene::showEventGroupFilter(vsEventInfo::Group group)
{
  this->setEventGroupFilterVisible(group, true);
}

//-----------------------------------------------------------------------------
void vsScene::hideEventGroupFilter(vsEventInfo::Group group)
{
  this->setEventGroupFilterVisible(group, false);
}

//-----------------------------------------------------------------------------
void vsScene::setEventGroupFilterVisible(
  vsEventInfo::Group group, bool visibility)
{
  QTE_D(vsScene);
  CHECK_ARG(d->FilterGroup.contains(group));
  d->FilterWidget->setGroupVisible(d->FilterGroup[group], visibility);
}

//-----------------------------------------------------------------------------
void vsScene::setEventGroupVisibility(
  vsEventInfo::Group group, bool visibility)
{
  QTE_D(vsScene);
  CHECK_ARG(d->FilterGroup.contains(group));
  d->FilterWidget->setGroupState(d->FilterGroup[group], visibility);
}

//-----------------------------------------------------------------------------
void vsScene::setEventGroupThreshold(
  vsEventInfo::Group group, double threshold)
{
  QTE_D(vsScene);
  CHECK_ARG(d->FilterGroup.contains(group));
  d->FilterWidget->setGroupValue(d->FilterGroup[group], threshold);
}

//-----------------------------------------------------------------------------
void vsScene::setGroundTruthVisible(bool visibility)
{
  QTE_D(vsScene);

  d->GroundTruthEnabled = visibility;
  if (!d->GroundTruthEnabled)
    {
    d->GroundTruthGraph.TrackRepresentation->SetVisible(false);
    d->GroundTruthGraph.TrackHeadRepresentation->SetVisible(false);
    d->GroundTruthGraph.TrackLabelRepresentation->SetVisible(false);
    d->GroundTruthGraph.EventRepresentation->SetVisible(false);
    d->GroundTruthGraph.EventHeadRepresentation->SetVisible(false);
    d->GroundTruthGraph.EventLabelRepresentation->SetVisible(false);
    }
  else
    {
    d->GroundTruthGraph.TrackRepresentation->SetVisible(
      d->NormalGraph.TrackRepresentation->GetVisible());
    d->GroundTruthGraph.TrackHeadRepresentation->SetVisible(
      d->NormalGraph.TrackHeadRepresentation->GetVisible());
    d->GroundTruthGraph.TrackLabelRepresentation->SetVisible(
      d->NormalGraph.TrackLabelRepresentation->GetVisible());
    d->GroundTruthGraph.EventRepresentation->SetVisible(
      d->NormalGraph.EventRepresentation->GetVisible());
    d->GroundTruthGraph.EventHeadRepresentation->SetVisible(
      d->NormalGraph.EventHeadRepresentation->GetVisible());
    d->GroundTruthGraph.EventLabelRepresentation->SetVisible(
      d->NormalGraph.EventLabelRepresentation->GetVisible());
    }
  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::setEventVisibility(int type, bool visibility)
{
  QTE_D(vsScene);

  switch (type)
    {
    case vsTrackInfo::Person:
      d->TrackFilter->SetShowType(vtkVgTrack::Person, visibility);
      break;
    case vsTrackInfo::Vehicle:
      d->TrackFilter->SetShowType(vtkVgTrack::Vehicle, visibility);
      break;
    case vsTrackInfo::Other:
      d->TrackFilter->SetShowType(vtkVgTrack::Other, visibility);
      break;
    default:
      d->EventFilter->SetShowType(type, visibility);
      break;
    }
  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::setEventThreshold(int type, double threshold)
{
  QTE_D(vsScene);

  switch (type)
    {
    case vsTrackInfo::Person:
      d->TrackFilter->SetMinProbability(vtkVgTrack::Person, threshold);
      break;
    case vsTrackInfo::Vehicle:
      d->TrackFilter->SetMinProbability(vtkVgTrack::Vehicle, threshold);
      break;
    case vsTrackInfo::Other:
      d->TrackFilter->SetMinProbability(vtkVgTrack::Other, threshold);
      break;
    default:
      d->EventFilter->SetMinProbability(type, threshold);
      break;
    }

  if (vsEventInfo::eventGroup(type) == vsEventInfo::Alert)
    {
    // When an alert's threshold changes, notify the alert list so that the
    // editor will have the current threshold
    emit this->alertThresholdChanged(type, threshold);
    }

  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::setEventThresholdInverted(int type, bool inverted)
{
  QTE_D(vsScene);

  switch (type)
    {
    case vsTrackInfo::Person:
      if (inverted)
        {
        d->TrackFilter->SetMaxProbability(vtkVgTrack::Person,
          d->TrackFilter->GetMinProbability(vtkVgTrack::Person));
        d->TrackFilter->SetMinProbability(vtkVgTrack::Person, 0.0);
        }
      else
        {
        d->TrackFilter->SetMinProbability(vtkVgTrack::Person,
          d->TrackFilter->GetMaxProbability(vtkVgTrack::Person));
        d->TrackFilter->SetMaxProbability(vtkVgTrack::Person, 1.0);
        }
      break;
    case vsTrackInfo::Vehicle:
      if (inverted)
        {
        d->TrackFilter->SetMaxProbability(vtkVgTrack::Vehicle,
          d->TrackFilter->GetMinProbability(vtkVgTrack::Vehicle));
        d->TrackFilter->SetMinProbability(vtkVgTrack::Vehicle, 0.0);
        }
      else
        {
        d->TrackFilter->SetMinProbability(vtkVgTrack::Vehicle,
          d->TrackFilter->GetMaxProbability(vtkVgTrack::Vehicle));
        d->TrackFilter->SetMaxProbability(vtkVgTrack::Vehicle, 1.0);
        }
      break;
    case vsTrackInfo::Other:
      if (inverted)
        {
        d->TrackFilter->SetMaxProbability(vtkVgTrack::Other,
          d->TrackFilter->GetMinProbability(vtkVgTrack::Other));
        d->TrackFilter->SetMinProbability(vtkVgTrack::Other, 0.0);
        }
      else
        {
        d->TrackFilter->SetMinProbability(vtkVgTrack::Other,
          d->TrackFilter->GetMaxProbability(vtkVgTrack::Other));
        d->TrackFilter->SetMaxProbability(vtkVgTrack::Other, 1.0);
        }
      break;
    default:
      d->EventFilter->SetInverse(type, inverted);
      break;
    }

  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::resetView()
{
  QTE_D(vsScene);

  double bounds[6];
  d->ImageActor->GetBounds(bounds);
  double w = bounds[1] - bounds[0], h = bounds[3] - bounds[2], a = w / h;
  double* ra = d->Renderer->GetAspect();
  double s = 0.5 * h; // fit height
  if (a > ra[0])
    s *= a / ra[0]; // fit width
  d->Renderer->ResetCamera(bounds);
  d->Renderer->GetActiveCamera()->SetParallelScale(s);

  // We don't need to update the scene, but we DO need to repaint, so we emit
  // our updated signal, but don't bother with all of update/postUpdate
  emit this->updated();
}

//-----------------------------------------------------------------------------
void vsScene::panTo(double x, double y)
{
  QTE_D(vsScene);

  vtkCamera* camera = d->Renderer->GetActiveCamera();

  double lastFocalPt[3], lastPos[3];
  camera->GetFocalPoint(lastFocalPt);
  camera->GetPosition(lastPos);

  camera->SetFocalPoint(x, y, lastFocalPt[2]);
  camera->SetPosition(x, y, lastPos[2]);
}

//-----------------------------------------------------------------------------
void vsScene::jumpToTrack(vtkVgTrack* track)
{
  CHECK_ARG(track);

  QTE_D(vsScene);

  // Get the point ID at the track current head
  vtkIdType pointId = track->GetClosestFramePtId(d->CurrentFrameTime);
  CHECK_ARG(pointId != -1);

  // Get point's stabilized coordinates and convert to image coordinates
  double point[3];
  track->GetPoints()->GetPoint(pointId, point);
  vtkVgApplyHomography(point, d->CurrentTransformVtk, point);

  // Pan the camera to track head as of the current frame
  this->panTo(point[0], point[1]);
}

//-----------------------------------------------------------------------------
void vsScene::jumpToEvent()
{
  QTE_D(vsScene);

  CHECK_ARG(d->FocusOnTargetEnabled);

  vtkVgEvent* event = d->findEvent(d->PendingJumpEventId);
  d->PendingJumpEventId = -1;

  // Make sure that event hasn't gone missing...
  CHECK_ARG(event);

  if (event->GetNumberOfTracks())
    {
    this->jumpToTrack(event->GetTrack(0));
    }
  else
    {
    // \TODO handle track-less events
    }
}

//-----------------------------------------------------------------------------
void vsScene::jumpToItem(vgfItemReference itemRef, vgf::JumpFlags flags)
{
  vtkIdType id = static_cast<vtkIdType>(itemRef.InternalId);

  switch (itemRef.Type)
    {
    case vgf::TrackItem:
      this->jumpToTrack(id, flags.testFlag(vgf::JumpToEndTime));
      break;
    case vgf::EventItem:
      this->jumpToEvent(id, flags.testFlag(vgf::JumpToEndTime));
      break;
    default:
      qWarning() << "vsScene::jumpToItem: Unknown item type" << itemRef.Type;
      break;
    }
}

//-----------------------------------------------------------------------------
void vsScene::jumpToEvent(vtkIdType eventId, bool jumpToEnd)
{
  QTE_D(vsScene);

  vtkVgEvent* event = d->findEvent(eventId);
  CHECK_ARG(event);
  emit this->jumpToItem();

  vtkVgTimeStamp time =
    jumpToEnd ? event->GetEndFrame() : event->GetStartFrame();
  vg::SeekMode direction =
    jumpToEnd ? vg::SeekUpperBound : vg::SeekLowerBound;

  // Allow the video frame homography to update before centering the camera
  d->PendingJumpEventId = eventId;

  this->seekVideo(time, direction, -1);
}

//-----------------------------------------------------------------------------
void vsScene::jumpToTrack()
{
  QTE_D(vsScene);

  CHECK_ARG(d->FocusOnTargetEnabled);

  this->jumpToTrack(d->findTrack(d->PendingJumpTrackId));
  d->PendingJumpTrackId = -1;
}

//-----------------------------------------------------------------------------
void vsScene::jumpToTrack(vtkIdType trackId, bool jumpToEnd)
{
  QTE_D(vsScene);

  vtkVgTrack* track = d->findTrack(trackId);
  CHECK_ARG(track);
  emit this->jumpToItem();

  vtkVgTimeStamp time =
    jumpToEnd ? track->GetEndFrame() : track->GetStartFrame();
  vg::SeekMode direction =
    jumpToEnd ? vg::SeekUpperBound : vg::SeekLowerBound;

  // Allow the video frame homography to update before centering the camera
  d->PendingJumpTrackId = trackId;

  this->seekVideo(time, direction, -1);
}

//-----------------------------------------------------------------------------
void vsScene::setTrackSelection(
  QSet<vtkIdType> selectedIds, vtkIdType currentId)
{
  QTE_D(vsScene);
  d->TrackTreeSelectionModel->setSelectedTracks(selectedIds);
  d->TrackTreeSelectionModel->setCurrentTrack(currentId);
}

//-----------------------------------------------------------------------------
void vsScene::setEventSelection(
  QSet<vtkIdType> selectedIds, vtkIdType currentId)
{
  QTE_D(vsScene);
  d->EventTreeSelectionModel->setSelectedEvents(selectedIds);
  d->EventTreeSelectionModel->setCurrentEvent(currentId);
}

//-----------------------------------------------------------------------------
void vsScene::updateTrackSelection(QSet<vtkIdType> selectedIds)
{
  QTE_D(vsScene);

  QSet<vtkVgTrack*> selectedTracks;
  bool modified = false;

  // Build new set of selected tracks
  foreach (vtkIdType id, selectedIds)
    {
    if (auto* const track = d->findTrack(id))
      {
      selectedTracks.insert(track);
      }
    }

  // Turn off selected color of tracks that are no longer selected
  foreach (auto* const track, d->SelectedTracks)
    {
    if (!selectedTracks.contains(track))
      {
      track->UseCustomColorOff();
      modified = true;
      }
    }

  // Turn on selected color for newly selected tracks
  foreach (auto* const track, selectedTracks)
    {
    if (!d->SelectedTracks.contains(track))
      {
      track->SetCustomColor(d->SelectionColor.constData().array);
      track->UseCustomColorOn();
      modified = true;
      }
    }

  if (modified)
    {
    d->SelectedTracks = selectedTracks;

    d->NormalGraph.TrackModel->Modified();
    d->GroundTruthGraph.TrackModel->Modified();
    this->postUpdate();

    emit this->selectedTracksChanged(selectedIds);
    }
}

//-----------------------------------------------------------------------------
void vsScene::updateEventSelection(QSet<vtkIdType> selectedIds)
{
  QTE_D(vsScene);

  QSet<vtkVgEvent*> selectedEvents;
  bool modified = false;

  // Build new set of selected events
  foreach (vtkIdType id, selectedIds)
    {
    if (auto* const event = d->findEvent(id))
      {
      selectedEvents.insert(event);
      }
    }

  // Turn off selected color of events that are no longer selected
  foreach (auto* const event, d->SelectedEvents)
    {
    if (!selectedEvents.contains(event))
      {
      event->UseCustomColorOff();
      modified = true;
      }
    }

  // Turn on selected color for newly selected events
  foreach (auto* const event, selectedEvents)
    {
    if (!d->SelectedEvents.contains(event))
      {
      event->SetCustomColor(d->SelectionColor.constData().array);
      event->UseCustomColorOn();
      modified = true;
      }
    }

  if (modified)
    {
    d->SelectedEvents = selectedEvents;

    d->NormalGraph.EventModel->Modified();
    d->GroundTruthGraph.EventModel->Modified();
    this->postUpdate();

    emit this->selectedEventsChanged(selectedIds);
    emit this->selectedEventsChanged(selectedEvents.toList());
    }
}

//-----------------------------------------------------------------------------
void vsScene::setEventStart(vtkIdType id)
{
  QTE_D(vsScene);
  if (d->CurrentFrameTime.IsValid())
    {
    d->Core->setEventStart(id, d->CurrentFrameTime);
    }
}

//-----------------------------------------------------------------------------
void vsScene::setEventEnd(vtkIdType id)
{
  QTE_D(vsScene);
  if (d->CurrentFrameTime.IsValid())
    {
    d->Core->setEventEnd(id, d->CurrentFrameTime);
    }
}

//-----------------------------------------------------------------------------
void vsScene::cancelInteraction()
{
  this->interruptDrawing();
  emit this->interactionCanceled();
}

//-----------------------------------------------------------------------------
void vsScene::beginDrawing(vsContour::Type type)
{
  QTE_D(vsScene);

  // End any other interaction that may currently be active
  this->cancelInteraction();

  emit this->contourStarted();
  emit this->statusMessageAvailable(
    "Drawing contour"
    "(<b>left click</b> to add points;"
    " <b>right click</b> to enter editing mode)");

  d->EditContour.reset(
    new vsContourWidget(d->Core->createContourId(),
                        d->RenderWindow->GetInteractor()));
  d->EditContour->setType(type);
  d->EditContour->setMatrix(d->CurrentTransformVtk);
  d->EditContour->begin();

  connect(d->EditContour.data(), SIGNAL(interactionComplete()),
          this, SLOT(beginContourManipulation()));
  connect(d->EditContour.data(), SIGNAL(manipulationComplete()),
          this, SLOT(finalizeContour()));
}

//-----------------------------------------------------------------------------
void vsScene::interruptDrawing()
{
  QTE_D(vsScene);

  if (!d->EditContour)
    return;

  switch (d->EditContour->state())
    {
    case vsContourWidget::Begin:
      // If the contour is still in initial drawing state, just delete it
      d->EditContour.reset();
      emit this->drawingCanceled();
      break;

    case vsContourWidget::Manipulate:
      // If the contour has been drawn but not finalized, do so
      this->finalizeContour();
      break;
    }

  d->RenderWindow->Render();
  emit this->statusMessageAvailable("");
}

//-----------------------------------------------------------------------------
int vsScene::contourState() const
{
  QTE_D_CONST(vsScene);
  return (d->EditContour ? d->EditContour->state() : -1);
}

//-----------------------------------------------------------------------------
void vsScene::setDrawingType(vsContour::Type newType)
{
  QTE_D(vsScene);

  if (d->EditContour && d->EditContour->type() != newType)
    {
    if (d->EditContour->state() == vsContourWidget::Manipulate)
      {
      if (!d->EditContour->isClosed() && vsContour::isLoopType(newType))
        {
        d->EditContour->close();
        }
      }

    d->EditContour->setType(newType);
    d->RenderWindow->Render();
    }
}

//-----------------------------------------------------------------------------
void vsScene::closeContour()
{
  QTE_D(vsScene);

  if (!d->EditContour)
    return;

  // Get number of points we have, and number needed (don't count point under
  // mouse cursor that has not been "committed", as it will be removed)
  vtkIdType numPoints = d->EditContour->points()->GetNumberOfPoints();
  vtkIdType neededPoints =
    (d->EditContour->state() == vsContourWidget::Begin ? 4 : 3);

  // Check that we have enough points to close the region
  if (numPoints >= neededPoints)
    {
    d->EditContour->close();
    emit this->contourClosed();

    // Finalize the contour
    this->finalizeContour();
    }
  else
    {
    emit this->statusMessageAvailable("Not enough points in region");
    }
}

//-----------------------------------------------------------------------------
void vsScene::beginContourManipulation()
{
  QTE_D(vsScene);
  if (d->EditContour->isClosed())
    emit this->contourClosed();
  emit this->statusMessageAvailable(
    "Editing contour (<b>right click</b> when done)");
}

//-----------------------------------------------------------------------------
void vsScene::finalizeContour()
{
  QTE_D(vsScene);

  // Finalize contour
  if (d->EditContour)
    {
    vtkIdType numPoints =
      d->EditContour->points()->GetNumberOfPoints();

    vsContour::Type currentType = d->EditContour->type();
    bool loopRequired = vsContour::isLoopType(currentType);

    if (numPoints < 2 || (numPoints < 3 && loopRequired))
      {
      emit this->statusMessageAvailable("Not enough points in region");
      d->EditContour.reset();
      }
    else
      {
      // Close contour if needed
      if (!d->EditContour->isClosed() && loopRequired)
        d->EditContour->close();

      // Finalize contour
      d->EditContour->setMatrix(d->CurrentTransformVtk);
      d->EditContour->finalize();

      // Emit completed contour and destroy widget
      emit this->contourCompleted(d->EditContour.data()->toContour());
      emit this->statusMessageAvailable("");
      d->EditContour.reset();
      }

    this->postUpdate();
    emit this->drawingEnded();
    }
}

//-----------------------------------------------------------------------------
void vsScene::addContour(vsContour contour)
{
  QTE_D(vsScene);

  Q_ASSERT(!d->Contours.contains(contour.id()));

  vtkRenderWindowInteractor* rwi = d->RenderWindow->GetInteractor();
  vsScenePrivate::ContourInfo info(new vsContourWidget(contour, rwi));
  info.widget->setMatrix(d->CurrentTransformVtk);
  d->addFilterContour(info);
  d->Contours.insert(contour.id(), info);
}

//-----------------------------------------------------------------------------
void vsScene::setContourType(int id, vsContour::Type newType)
{
  QTE_D(vsScene);

  if (!d->Contours.contains(id))
    return;

  vsScenePrivate::ContourInfo& info = d->Contours[id];
  vsContour::Type currentType = info.widget->type();

  // Anything to do?
  if (currentType != newType)
    {
    // Remove filters/selectors from VTK model/view
    d->removeFilterContour(info);

    // Update the type
    info.widget->setType(newType);

    // Create new masks if necessary
    d->addFilterContour(info);

    // Done
    this->postUpdate();
    }
}

//-----------------------------------------------------------------------------
void vsScene::setContourPoints(int id, QPolygonF points)
{
  QTE_D(vsScene);

  if (!d->Contours.contains(id))
    return;

  vsScenePrivate::ContourInfo& info = d->Contours[id];
  info.widget->setPoints(points);
  this->postUpdate();
}

//-----------------------------------------------------------------------------
bool vsScene::setContourEnabled(int id, bool enabled)
{
  QTE_D(vsScene);

  if (!d->Contours.contains(id))
    return false;

  vsScenePrivate::ContourInfo& info = d->Contours[id];

  // Anything to do?
  if (info.enabled != enabled)
    {
    vsContour::Type type = info.widget->type();
    if (type == vsContour::Tripwire)
      {
      // \TODO need some way to apply filtering to associated events
      }
    else if (type == vsContour::Selector || type == vsContour::Filter)
      {
      info.maskActor->SetVisibility(enabled);

      if (type == vsContour::Selector)
        {
        if (enabled)
          {
          d->SelectorMaskQuad->SetVisibility(true);
          }
        else
          {
          // If all selectors have been disabled, make sure the image overlay
          // mask is hidden
          vtkProp3DCollection* props = d->SelectorMaskProps->GetParts();
          props->InitTraversal();
          int numVisible = 0;
          while (vtkProp* prop = props->GetNextProp())
            prop->GetVisibility() && ++numVisible;
          d->SelectorMaskQuad->SetVisibility(numVisible > 0);
          }
        }

      d->ContourOperatorManager->SetContourEnabled(
        info.widget->points(), enabled);
      }

    // Change visibility and update
    info.enabled = enabled;
    info.widget->setVisible(enabled);
    this->postUpdate();
    }

  return true;
}

//-----------------------------------------------------------------------------
void vsScene::removeContour(int id)
{
  QTE_D(vsScene);

  if (!d->Contours.contains(id))
    return;

  // Remove contour from internal map
  vsScenePrivate::ContourInfo info = d->Contours.take(id);

  // Remove filters/selectors from VTK model/view
  d->removeFilterContour(info);

  // Done
  this->postUpdate();
}

//-----------------------------------------------------------------------------
bool vsScene::convertContourToEvent(int id, int eventType)
{
  QTE_D(vsScene);

  if (!d->CurrentFrameTime.IsValid())
    {
    // Can't convert with no valid time
    // TODO: show warning to user?
    return false;
    }

  return d->Core->convertContourToEvent(id, eventType, d->CurrentFrameTime);
}

//-----------------------------------------------------------------------------
void vsScene::addUserEventType(
  int id, vsEventInfo info, double initialThreshold)
{
  Q_UNUSED(id);
  QTE_D(vsScene);
  d->addFilter(info, info.group, true, initialThreshold);
}

//-----------------------------------------------------------------------------
void vsScene::addAlert(int id, vsAlert alert)
{
  Q_UNUSED(id);
  QTE_D(vsScene);
  d->addFilter(alert.eventInfo, vsEventInfo::Alert,
               true, alert.displayThreshold);
}

//-----------------------------------------------------------------------------
void vsScene::updateAlert(int id, vsAlert alert)
{
  Q_UNUSED(id);
  QTE_D(vsScene);
  d->FilterWidget->setText(alert.eventInfo.type, alert.eventInfo.name);
}

//-----------------------------------------------------------------------------
void vsScene::updateAlertThreshold(int id, vsAlert alert)
{
  Q_UNUSED(id);
  QTE_D(vsScene);
  d->FilterWidget->setValue(alert.eventInfo.type, alert.displayThreshold);
}

//-----------------------------------------------------------------------------
void vsScene::removeAlert(int id, bool unregister)
{
  if (unregister)
    {
    QTE_D(vsScene);
    d->FilterWidget->removeItem(id);
    // \TODO should remove from d->EventFilter also?
    }
}

//-----------------------------------------------------------------------------
void vsScene::startQuickNoteTimer(vtkIdType eventId)
{
  QTE_D(vsScene);

  d->LastManualEventCreated = eventId;
  d->TimerLog->StartTimer();
}

//-----------------------------------------------------------------------------
void vsScene::postUpdate()
{
  QTE_D(vsScene);
  d->PendingUpdates.append(vsScenePrivate::Update());
  QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

//-----------------------------------------------------------------------------
void vsScene::postUpdate(vtkVgTimeStamp ts)
{
  QTE_D(vsScene);
  d->PendingUpdates.append(vsScenePrivate::Update(ts));
  QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

//-----------------------------------------------------------------------------
void vsScene::update()
{
  QTE_D(vsScene);

  // Punt if there are no outstanding updates
  if (!d->PendingUpdates.count())
    return;

  // Process pending updates, updating the current video time to the most
  // recent video update
  foreach (const vsScenePrivate::Update& u, d->PendingUpdates)
    {
    if (u.videoUpdated)
      d->CurrentFrameTime = u.time;
    }
  d->PendingUpdates.clear();

  // Update actors and redraw
  if (d->CurrentFrameTime.IsValid())
    {
    // Normal events and tracks
    bool forceUpdate = d->Core->eventTypeRegistry()->GetMTime() >
                       d->NormalGraph.EventRepresentation->GetUpdateTime();
    d->updateModels(d->NormalGraph, forceUpdate);

    // Ground-truth versions
    if (d->Core->isGroundTruthDataPresent())
      d->updateModels(d->GroundTruthGraph);
    }

  if (d->PendingJumpEventId != -1)
    this->jumpToEvent();

  if (d->PendingJumpTrackId != -1)
    this->jumpToTrack();

  emit this->updated();
}

//-----------------------------------------------------------------------------
void vsScene::setVideoSamplingMode(int mode)
{
  QTE_D(vsScene);

  switch (mode)
    {
    default:
    case 0:
      d->ImageActor->GetProperty()->SetInterpolationTypeToNearest();
      break;
    case 1:
      d->ImageActor->GetProperty()->SetInterpolationTypeToLinear();
      break;
    case 2:
      d->ImageActor->GetProperty()->SetInterpolationTypeToCubic();
      break;
    }

  d->ImageActor->Update();

  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::setTracksVisible(bool visibility)
{
  QTE_D(vsScene);

  const bool flag = !visibility;
  d->NormalGraph.TrackRepresentation->SetOnlyDisplayForcedTracks(flag);
  d->GroundTruthGraph.TrackRepresentation->SetOnlyDisplayForcedTracks(flag);

  // If tracks not shown, and event tracks not shown, hide track representation
  // entirely
  const bool repVisibility =
    visibility || d->NormalGraph.EventRepresentation->GetVisible();
  d->NormalGraph.TrackRepresentation->SetVisible(repVisibility);
  d->GroundTruthGraph.TrackRepresentation->SetVisible(
    d->GroundTruthEnabled && repVisibility);

  // Determine label visibility
  visibility |= !!d->NormalGraph.TrackHeadRepresentation->GetVisible();
  visibility &= d->NormalGraph.TrackLabelRepresentation->GetShowName() ||
                d->NormalGraph.TrackLabelRepresentation->GetShowProbability();
  d->NormalGraph.TrackLabelRepresentation->SetVisible(visibility);
  d->GroundTruthGraph.TrackLabelRepresentation->SetVisible(
    d->GroundTruthEnabled && visibility);

  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::setTrackBoxesVisible(bool visibility)
{
  QTE_D(vsScene);

  d->NormalGraph.TrackHeadRepresentation->SetVisible(visibility);
  d->GroundTruthGraph.TrackHeadRepresentation->SetVisible(
    d->GroundTruthEnabled && visibility);

  // Determine label visibility
  visibility |=
    !d->NormalGraph.TrackRepresentation->GetOnlyDisplayForcedTracks();
  visibility &= d->NormalGraph.TrackLabelRepresentation->GetShowName() ||
                d->NormalGraph.TrackLabelRepresentation->GetShowProbability();
  d->NormalGraph.TrackLabelRepresentation->SetVisible(visibility);
  d->GroundTruthGraph.TrackLabelRepresentation->SetVisible(
    d->GroundTruthEnabled && visibility);

  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::setTrackIdsVisible(bool visibility)
{
  QTE_D(vsScene);

  d->NormalGraph.TrackLabelRepresentation->SetShowName(visibility);
  d->GroundTruthGraph.TrackLabelRepresentation->SetShowName(visibility);

  visibility |= d->NormalGraph.TrackLabelRepresentation->GetShowProbability();
  visibility &=
    !d->NormalGraph.TrackRepresentation->GetOnlyDisplayForcedTracks() ||
    d->NormalGraph.TrackHeadRepresentation->GetVisible();
  d->NormalGraph.TrackLabelRepresentation->SetVisible(visibility);
  d->GroundTruthGraph.TrackLabelRepresentation->SetVisible(
    d->GroundTruthEnabled && visibility);

  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::setTrackPvoScoresVisible(bool visibility)
{
  QTE_D(vsScene);

  d->NormalGraph.TrackLabelRepresentation->SetShowProbability(visibility);
  d->GroundTruthGraph.TrackLabelRepresentation->SetShowProbability(visibility);

  visibility |= d->NormalGraph.TrackLabelRepresentation->GetShowName();
  visibility &=
    !d->NormalGraph.TrackRepresentation->GetOnlyDisplayForcedTracks() ||
    d->NormalGraph.TrackHeadRepresentation->GetVisible();
  d->NormalGraph.TrackLabelRepresentation->SetVisible(visibility);
  d->GroundTruthGraph.TrackLabelRepresentation->SetVisible(
    d->GroundTruthEnabled && visibility);

  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::setVideoLiveOffset(double offset)
{
  QTE_D(vsScene);

  d->VideoPlayer.setLivePlaybackOffset(offset > 0.0 ? offset * 1e6 : 0.0);
}

//-----------------------------------------------------------------------------
double vsScene::videoLiveOffset() const
{
  QTE_D_CONST(vsScene);

  return d->VideoPlayer.livePlaybackOffset() * 1e-6;
}

//-----------------------------------------------------------------------------
void vsScene::setTrackTrailLength(double length)
{
  QTE_D(vsScene);

  vtkVgTimeStamp& ttl = d->TrackTrailLength;
  (length > 0.0
   ? ttl.SetTime(length * 1e6)
   : ttl.SetToMaxTime());

  // Update existing tracks
  d->NormalGraph.TrackModel->SetMaximumDisplayDuration(ttl);
  d->GroundTruthGraph.TrackModel->SetMaximumDisplayDuration(ttl);
  this->postUpdate();
}

//-----------------------------------------------------------------------------
double vsScene::trackTrailLength() const
{
  QTE_D_CONST(vsScene);

  if (d->TrackTrailLength.IsMaxTime())
    return 0.0;
  return d->TrackTrailLength.GetTime() * 1e-6;
}

//-----------------------------------------------------------------------------
void vsScene::setSelectionColor(QColor color)
{
  QTE_D(vsScene);
  d->SelectionColor = color;
  // TODO update track/event colors
  emit this->trackSceneUpdated();
  emit this->eventSceneUpdated();
}

//-----------------------------------------------------------------------------
QColor vsScene::selectionColor() const
{
  QTE_D_CONST(vsScene);
  return d->SelectionColor.toQColor();
}

//-----------------------------------------------------------------------------
void vsScene::setFilteringMaskVisible(bool visibility)
{
  QTE_D(vsScene);
  d->FilterMaskProps->SetVisibility(visibility);
  d->SelectorMaskProps->SetVisibility(visibility);
  d->SelectorMaskQuad->SetVisibility(visibility);
  emit this->updated();
}

//-----------------------------------------------------------------------------
void vsScene::setTrackingMaskVisible(bool visibility)
{
  QTE_D(vsScene);
  d->TrackingMask->SetVisibility(visibility);
  emit this->updated();
}

//-----------------------------------------------------------------------------
void vsScene::setEventTracksVisible(bool visibility)
{
  QTE_D(vsScene);

  d->NormalGraph.EventRepresentation->SetVisible(visibility);
  d->GroundTruthGraph.EventRepresentation->SetVisible(
    d->GroundTruthEnabled && visibility);
  if (!visibility)
    {
    d->NormalGraph.TrackRepresentation->ClearForceShownTracks();
    d->GroundTruthGraph.TrackRepresentation->ClearForceShownTracks();
    }

  // If event tracks not shown, and tracks not shown, hide track representation
  // entirely
  visibility |=
    !d->NormalGraph.TrackRepresentation->GetOnlyDisplayForcedTracks();
  d->NormalGraph.TrackRepresentation->SetVisible(visibility);
  d->GroundTruthGraph.TrackRepresentation->SetVisible(
    d->GroundTruthEnabled && visibility);

  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::setEventBoxesVisible(bool visibility)
{
  QTE_D(vsScene);

  d->NormalGraph.EventHeadRepresentation->SetVisible(visibility);
  d->GroundTruthGraph.EventHeadRepresentation->SetVisible(
    d->GroundTruthEnabled && visibility);

  vtkVgEventLabelRepresentation::enumLocationSourceMode lsm =
    (visibility ? vtkVgEventLabelRepresentation::LSM_RegionThenTrack
                : vtkVgEventLabelRepresentation::LSM_Track);
  d->NormalGraph.EventLabelRepresentation->SetLocationSourceMode(lsm);
  d->GroundTruthGraph.EventLabelRepresentation->SetLocationSourceMode(lsm);

  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::setEventLabelsVisible(bool visibility)
{
  QTE_D(vsScene);
  d->NormalGraph.EventLabelRepresentation->SetVisible(visibility);
  d->GroundTruthGraph.EventLabelRepresentation->SetVisible(
    d->GroundTruthEnabled && visibility);
  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::setEventProbabilityVisible(bool visibility)
{
  QTE_D(vsScene);
  d->NormalGraph.EventLabelRepresentation->SetShowProbability(visibility);
  d->GroundTruthGraph.EventLabelRepresentation->SetShowProbability(
    visibility);
  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::setNotesVisible(bool visibility)
{
  QTE_D(vsScene);
  d->NormalGraph.TrackLabelRepresentation->SetShowNote(visibility);
  d->NormalGraph.EventLabelRepresentation->SetShowNote(visibility);
  d->GroundTruthGraph.TrackLabelRepresentation->SetShowNote(visibility);
  d->GroundTruthGraph.EventLabelRepresentation->SetShowNote(visibility);
  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::updateTrackNotes()
{
  QTE_D(vsScene);

  if (d->NormalGraph.TrackLabelRepresentation->GetShowNote())
    {
    d->NormalGraph.TrackLabelRepresentation->Modified();
    d->GroundTruthGraph.TrackLabelRepresentation->Modified();
    this->postUpdate();
    }
}

//-----------------------------------------------------------------------------
void vsScene::updateEventNotes()
{
  QTE_D(vsScene);

  if (d->NormalGraph.EventLabelRepresentation->GetShowNote())
    {
    d->NormalGraph.EventLabelRepresentation->Modified();
    d->GroundTruthGraph.EventLabelRepresentation->Modified();
    this->postUpdate();
    }
}

//-----------------------------------------------------------------------------
QPointF vsScene::viewToFrame(const QPointF& in)
{
  QTE_D(vsScene);

  double pt[3] = { in.x(), d->RenderWidget->height() - in.y(), 0.0 };
  vtkVgSpaceConversion::DisplayToWorldNormalized(d->Renderer, pt, pt);

  double frameBounds[6];
  d->ImageActor->GetBounds(frameBounds);
  const double h = frameBounds[3] - frameBounds[2];

  return QPointF(pt[0], h - pt[1]);
}

//-----------------------------------------------------------------------------
vgGeocodedCoordinate vsScene::viewToLatLon(const QPointF& in)
{
  QTE_D(vsScene);

  vtkSmartPointer<vtkMatrix4x4> imageToLatLonMatrix =
    d->CurrentFrameMetaData.MakeImageToLatLonMatrix();

  vgGeocodedCoordinate geoCoord;
  if (imageToLatLonMatrix)
    {
    const double h = d->RenderWidget->height();
    double point[3] = { in.x(), h - in.y(), 0.0 };
    vtkVgSpaceConversion::DisplayToWorldNormalized(d->Renderer, point, point);
    vtkVgApplyHomography(point, imageToLatLonMatrix,
                         geoCoord.Easting, geoCoord.Northing);
    geoCoord.GCS = d->CurrentFrameMetaData.WorldLocation.GCS;
    }
  return geoCoord;
}

//-----------------------------------------------------------------------------
vgMatrix4d vsScene::currentTransform() const
{
  QTE_D_CONST(vsScene);
  return d->CurrentTransformEigen;
}

//-----------------------------------------------------------------------------
const vtkVgVideoFrameMetaData& vsScene::currentFrameMetaData() const
{
  QTE_D_CONST(vsScene);
  return d->CurrentFrameMetaData;
}

//-----------------------------------------------------------------------------
vtkRenderer* vsScene::renderer()
{
  QTE_D(vsScene);
  return d->Renderer;
}

//-----------------------------------------------------------------------------
vgVideoPlayer::PlaybackMode vsScene::videoPlaybackStatus() const
{
  QTE_D_CONST(vsScene);
  return d->VideoPlayer.playbackMode();
}

//-----------------------------------------------------------------------------
qreal vsScene::videoPlaybackSpeed() const
{
  QTE_D_CONST(vsScene);
  return d->VideoPlayer.playbackSpeed();
}

//-----------------------------------------------------------------------------
void vsScene::setVideoPlaybackSpeed(
  vgVideoPlayer::PlaybackMode mode, qreal rate)
{
  QTE_D(vsScene);
  d->VideoPlayer.setPlaybackSpeed(mode, rate);
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vsScene::currentVideoTime() const
{
  QTE_D_CONST(vsScene);
  return d->CurrentFrameTime;
}

//-----------------------------------------------------------------------------
void vsScene::seekVideo(
  vtkVgTimeStamp ts, vg::SeekMode direction, qint64 requestId)
{
  QTE_D(vsScene);
  if (d->VideoPlayer.playbackMode() == vgVideoPlayer::Live)
    {
    // Seek in live playback mode causes playback to pause
    d->VideoPlayer.setPlaybackSpeed(vgVideoPlayer::Paused, 1.0);
    }
  d->VideoPlayer.seekToTimestamp(requestId, ts, direction);
}

//-----------------------------------------------------------------------------
void vsScene::resetVideo()
{
  QTE_D(vsScene);
  d->VideoPlayer.setVideoSource(d->VideoSource = 0);

  // \TODO clear video
}

//-----------------------------------------------------------------------------
void vsScene::resetVideo(vsVideoSource* source)
{
  QTE_D(vsScene);
  this->postUpdate(vtkVgTimeStamp());
  d->NeedViewReset = true;
  d->SourceIsStreaming = false;

  // \TODO clear video

  d->VideoPlayer.setPlaybackSpeed(vgVideoPlayer::Stopped, 0.0);
  d->VideoPlayer.setVideoSource(d->VideoSource = source);

  if (source)
    {
    this->setSourceStreaming(source->isStreaming());

    connect(source, SIGNAL(streamingChanged(bool)),
            this, SLOT(setSourceStreaming(bool)));
    connect(source, SIGNAL(frameRangeAvailable(vtkVgTimeStamp, vtkVgTimeStamp)),
            this, SLOT(setSourceFrameRange(vtkVgTimeStamp, vtkVgTimeStamp)));

    connect(source, SIGNAL(destroyed(QObject*)),
            this, SLOT(removeVideoSource(QObject*)));
    }
}

//-----------------------------------------------------------------------------
void vsScene::setSourceStreaming(bool isStreaming)
{
  QTE_D(vsScene);

  if (d->SourceIsStreaming && !isStreaming)
    {
    const vgRange<vtkVgTimeStamp> range = d->VideoSource->frameRange();
    d->VideoPlayer.setSourceFrameRange(range.lower, range.upper);
    }

  d->SourceIsStreaming = isStreaming;
  d->VideoPlayer.setSourceStreaming(isStreaming);
}

//-----------------------------------------------------------------------------
void vsScene::setSourceFrameRange(vtkVgTimeStamp first, vtkVgTimeStamp last)
{
  QTE_D(vsScene);

  // If we are streaming, adjust frame range reported to video player by stream
  // delay, if set
  if (d->SourceIsStreaming && d->StreamDelay.IsValid())
    {
    last.ShiftBackward(d->StreamDelay);
    last = qMax(first, last);
    }

  d->VideoPlayer.setSourceFrameRange(first, last);
}

//-----------------------------------------------------------------------------
void vsScene::removeVideoSource(QObject* source)
{
  QTE_D(vsScene);
  QObject* currentSource = d->VideoSource;
  if (source == currentSource)
    {
    // If the source that just went away is the current source, clear the
    // source and reset the video... this check is needed as the old source may
    // not be destroyed before we have switched to a new source, in which case
    // resetting would break the new source
    this->resetVideo();
    }
}

//-----------------------------------------------------------------------------
void vsScene::updateVideoFrame(vtkVgVideoFrame frame, qint64 requestId)
{
  QTE_D(vsScene);

  // Update video frame
  d->ImageActor->SetInputData(frame.Image.GetPointer());

  // Recenter image on first frame
  if (d->NeedViewReset)
    {
    this->resetView();
    d->NeedViewReset = false;
    }

  // Update homography reference time
  const vtkVgVideoFrameMetaData& metadata = frame.MetaData;
  if (metadata.HomographyReferenceFrame < 0)
    {
    // Initializes to 0 (MinTime), so if we get here and NOT 0, that means we
    // must have valid reference frame data. Thus, since we got a -1, this
    // frame must be "garbage", which amounts to a reference reset (one is
    // about to happen); we don't want to display ANY tracks / events on this
    // frame (which setting to MaxTime will accomplish).
    if (d->CurrentHomographyReferenceTime.GetTime() != 0)
      d->CurrentHomographyReferenceTime.SetToMaxTime();
    }
  else
    {
    d->CurrentHomographyReferenceTime =
      d->Core->homographyReferenceTime(metadata.HomographyReferenceFrame,
                                       metadata.Time);
    }
  d->CurrentFrameMetaData = metadata;

  // Update track/event display for current frame
  this->postUpdate(metadata.Time);

  // Generate the transform matrix
  double ib[6];
  d->ImageActor->GetBounds(ib);
  const auto& hi = vtkVgAdapt(metadata.Homography).inverse();
  auto fy = vgMatrix4d{vgMatrix4d::Identity()};
  fy(1, 1) = -1.0;
  fy(1, 3) = ib[3] - ib[2];
  d->CurrentTransformEigen = fy * hi;

  if (d->CurrentTransformEigen(3, 3) < 0)
    {
    // If the 3,3 component is negative, we're like to run into an OpenGL issue
    // where points are not rendered when the homogeneous coordinate after
    // transformation is < 0.  This is a temporary fix.
    d->CurrentTransformEigen *= -1.0;
    }

  // Copy Eigen matrix to VTK matrix
  vtkVgInstance<vtkMatrix4x4> xf;
  vtkVgAdapt(d->CurrentTransformEigen, xf);

  d->CurrentTransformVtk->DeepCopy(xf);

  // Update representation transform matrices
  vsScenePrivate::setRepresentationTransforms(d->NormalGraph, xf);
  vsScenePrivate::setRepresentationTransforms(d->GroundTruthGraph, xf);
  d->FilterMaskProps->SetUserMatrix(xf);
  d->SelectorMaskProps->SetUserMatrix(xf);
  d->updateContourMaskPositions();

  // Update contour transform matrices
  if (d->EditContour)
    d->EditContour->setMatrix(xf);
  foreach (const vsScenePrivate::ContourInfo& info, d->Contours)
    info.widget->setMatrix(xf);

  d->Renderer->ResetCameraClippingRange();

  // Update location
  const int x = d->LastCursorPosition.x(), y = d->LastCursorPosition.y();
  emit this->locationTextUpdated(d->buildLocationTextFromDisplay(x, y));

  // Pass along metadata
  emit this->transformChanged(d->CurrentTransformEigen);
  emit this->videoMetadataUpdated(metadata, requestId);
  emit this->currentTimeChanged(metadata.Time.GetRawTimeStamp());
}

//-----------------------------------------------------------------------------
void vsScene::onLeftClick()
{
  QTE_D(vsScene);

  vtkRenderWindowInteractor* interactor = d->RenderWindow->GetInteractor();

  int x, y;
  interactor->GetEventPosition(x, y);

  // Pick against normal events
  vtkIdType eventId = this->tryPick(d->NormalGraph.EventLabelRepresentation,
                                    d->NormalGraph.EventHeadRepresentation,
                                    d->NormalGraph.EventRepresentation, x, y);
  if (eventId != -1)
    {
    // Ctrl-Click to give positive feedback
    if (interactor->GetControlKey())
      {
      d->Core->setEventRating(eventId, vgObjectStatus::Adjudicated);
      QModelIndex idx = d->EventTreeModel->indexOfEvent(eventId);
      d->EventTreeModel->setData(
        idx, vs::VerifiedEvent, vsEventTreeModel::StatusRole);
      }

    // TODO handle ctrl to multi-select?
    d->EventTreeSelectionModel->selectEvent(eventId);
    emit this->pickedEvent(eventId);
    return;
    }

  // Pick against GT events
  eventId = this->tryPick(d->GroundTruthGraph.EventLabelRepresentation,
                          d->GroundTruthGraph.EventHeadRepresentation,
                          d->GroundTruthGraph.EventRepresentation, x, y);
  if (eventId != -1)
    {
    // TODO handle ctrl to multi-select?
    d->EventTreeSelectionModel->selectEvent(eventId);
    emit this->pickedEvent(eventId);
    return;
    }

  // Pick against tracks
  vtkIdType trackId = this->tryPick(d->NormalGraph.TrackLabelRepresentation,
                                    d->NormalGraph.TrackHeadRepresentation,
                                    d->NormalGraph.TrackRepresentation, x, y);
  if (trackId != -1)
    {
    // TODO handle ctrl to multi-select?
    d->TrackTreeSelectionModel->selectTrack(trackId);
    emit this->pickedTrack(trackId);
    return;
    }

  // Pick against GT tracks
  trackId = this->tryPick(d->GroundTruthGraph.TrackLabelRepresentation,
                          d->GroundTruthGraph.TrackHeadRepresentation,
                          d->GroundTruthGraph.TrackRepresentation, x, y);
  if (trackId != -1)
    {
    // TODO handle ctrl to multi-select?
    d->TrackTreeSelectionModel->selectTrack(trackId);
    emit this->pickedTrack(trackId);
    return;
    }
}

//-----------------------------------------------------------------------------
void vsScene::onRightClick()
{
  QTE_D(vsScene);

  if (d->LastManualEventCreated != -1)
    {
    // Note: this doesn't actually "stop" the timer, but instead sets the
    // EndTime for use with GetElapsedTime; StartTime is unchanged and thus
    // another call to StopTimer will get cumulative ElapsedTime.
    d->TimerLog->StopTimer();
    if (d->MaxTimeForQuickNote > d->TimerLog->GetElapsedTime())
      {
      bool ok = false;
      QString note;
      note = vgTextEditDialog::getText(0, "Add Event Note", note, &ok);
      if (ok)
        {
        d->Core->setEventNote(d->LastManualEventCreated, note);
        d->EventTreeModel->update();
        // Assumption with quick note is that no note exists; thus reset
        // the event id so we can't edit this note again.
        d->LastManualEventCreated = -1;
        }
      return;
      }
    }
  vtkRenderWindowInteractor* interactor = d->RenderWindow->GetInteractor();

  int x, y;
  interactor->GetEventPosition(x, y);
  const vtkIdType trackId =
    this->tryPick(d->NormalGraph.TrackLabelRepresentation,
                  d->NormalGraph.TrackHeadRepresentation,
                  d->NormalGraph.TrackRepresentation, x, y);
  const vtkIdType eventId =
    this->tryPick(d->NormalGraph.EventLabelRepresentation,
                  d->NormalGraph.EventHeadRepresentation,
                  d->NormalGraph.EventRepresentation, x, y);

  // Ctrl-RMB to give negative feedback and reject the event
  if (interactor->GetControlKey())
    {
    if (eventId != -1)
      {
      d->Core->setEventRating(eventId, vgObjectStatus::Excluded);
      QModelIndex idx = d->EventTreeModel->indexOfEvent(eventId);
      d->EventTreeModel->setData(
        idx, vs::RejectedEvent, vsEventTreeModel::StatusRole);
      }
    return;
    }

  // Get widget mouse position (adjusted for zero-based value)
  QPoint widgetPos(x, d->RenderWindow->GetInteractor()->GetSize()[1] - y - 1);

  // Prepare context menu
  QMenu menu(d->RenderWidget);
  QAction* choice = 0;

  QAction* copyLocation = 0;
  if (d->CurrentFrameTime.IsValid())
    {
    copyLocation = menu.addAction("&Copy Location");
    }

  // Handle context menu for tracks
  QAction* followTrack = 0;
  if (trackId != -1)
    {
    menu.addSeparator();
    followTrack = menu.addAction("&Follow Track");
    }

  // Handle context menu for events
  if (eventId != -1)
    {
    menu.addSeparator();
    vsEventRatingMenu::buildMenu(&menu);

    QAction* unrated =
      vsEventRatingMenu::getAction(&menu, vsEventRatingMenu::Unrated);
    QAction* relevant =
      vsEventRatingMenu::getAction(&menu, vsEventRatingMenu::Relevant);
    QAction* excluded =
      vsEventRatingMenu::getAction(&menu, vsEventRatingMenu::Excluded);
    QAction* rejected =
      vsEventRatingMenu::getAction(&menu, vsEventRatingMenu::Rejected);

    // Show the menu
    choice = menu.exec(d->RenderWidget->mapToGlobal(widgetPos));
    if (!choice)
      {
      // Menu was dismissed; return without doing anything
      return;
      }

    int rating =
      (choice == unrated  ? vgObjectStatus::None :
       choice == relevant ? vgObjectStatus::Adjudicated :
       choice == excluded ||
       choice == rejected ? vgObjectStatus::Excluded :
                            -1);

    int status =
      (choice == rejected ? vs::RejectedEvent : vs::VerifiedEvent);

    // If an event-related action was picked, handle it now and return;
    // otherwise fall through to general action handling below
    if (rating != -1)
      {
      d->Core->setEventRating(eventId, rating);

      // Update the status
      QModelIndex idx = d->EventTreeModel->indexOfEvent(eventId);
      d->EventTreeModel->setData(idx, status, vsEventTreeModel::StatusRole);
      return;
      }
    }
  else if (!menu.actions().isEmpty())
    {
    choice = menu.exec(d->RenderWidget->mapToGlobal(widgetPos));
    }

  // Handle general actions
  if (choice == copyLocation)
    {
    // Generate location text and copy to clipboard
    qApp->clipboard()->setText(d->buildLocationTextFromDisplay(x, y));
    }
  else if (choice == followTrack)
    {
    d->Core->startFollowingTrack(trackId);
    }
}

//-----------------------------------------------------------------------------
void vsScene::vtkSceneMouseMoveEvent()
{
  QTE_D(vsScene);

  // Always get cursor position, so that we show location when we start getting
  // video (if mouse is over the view)
  int x, y;
  d->RenderWidget->GetInteractor()->GetEventPosition(x, y);
  d->LastCursorPosition = QPoint(x, y);

  if (d->CurrentFrameTime.IsValid())
    {
    // Only emit location text if we have video
    emit this->locationTextUpdated(d->buildLocationTextFromDisplay(x, y));
    }
}

//-----------------------------------------------------------------------------
void vsScene::vtkSceneLeaveEvent()
{
  QTE_D(vsScene);
  d->LastCursorPosition = QPoint(-1, -1);
  emit this->locationTextUpdated(QString());
}

//-----------------------------------------------------------------------------
vtkIdType vsScene::tryPick(vtkVgRepresentationBase& labelRep,
                           vtkVgRepresentationBase& headRep,
                           vtkVgRepresentationBase& lineRep, int x, int y)
{
  QTE_D(vsScene);

  // Pick against labels
  vtkIdType pickType;
  vtkIdType pickedId = labelRep.Pick(x, y, d->Renderer, pickType);
  if (pickedId != -1)
    {
    return pickedId;
    }

  // Pick against head/region
  pickedId = headRep.Pick(x, y, d->Renderer, pickType);
  if (pickedId != -1)
    {
    return pickedId;
    }

  // Pick against lines
  return lineRep.Pick(x, y, d->Renderer, pickType);
}

//-----------------------------------------------------------------------------
void vsScene::setEventStatus(vtkVgEvent* event, int newStatus)
{
  QTE_D(vsScene);

  QModelIndex idx = d->EventTreeModel->indexOfEvent(event->GetId());
  d->EventTreeModel->setData(idx, newStatus, vsEventTreeModel::StatusRole);
}

//-----------------------------------------------------------------------------
void vsScene::setFocusOnTarget(bool state)
{
  QTE_D(vsScene);

  d->FocusOnTargetEnabled = state;
}

//-----------------------------------------------------------------------------
void vsScene::setTrackDisplayState(vtkIdType trackId, bool state)
{
  QTE_D(vsScene);

  d->NormalGraph.TrackModel->SetTrackDisplayState(trackId, state);
  d->GroundTruthGraph.TrackModel->SetTrackDisplayState(trackId, state);
  this->postUpdate();
}

//-----------------------------------------------------------------------------
bool vsScene::trackDisplayState(vtkIdType trackId)
{
  QTE_D(vsScene);

  vtkVgTrackInfo trackInfo = d->trackInfo(trackId);
  return (!!trackInfo.GetTrack() && trackInfo.GetDisplayTrack());
}

//-----------------------------------------------------------------------------
bool vsScene::trackVisibility(vtkIdType trackId)
{
  QTE_D(vsScene);

  vtkVgTrackInfo trackInfo = d->trackInfo(trackId);
  if (trackInfo.GetTrack())
    {
    vtkVgTrack* const track = trackInfo.GetTrack();
    const int bestClassifier = d->TrackFilter->GetBestClassifier(track);
    return trackInfo.GetDisplayTrack() && (bestClassifier != -1);
    }
  return false; // Track not found
}

//-----------------------------------------------------------------------------
vsDisplayInfo vsScene::trackDisplayInfo(vtkIdType trackId)
{
  QTE_D(vsScene);

  vsDisplayInfo di;
  vtkVgTrackInfo trackInfo = d->trackInfo(trackId);
  vtkVgTrack* const track = trackInfo.GetTrack();
  if (track)
    {
    di.DisplayState = trackInfo.GetDisplayTrack();
    di.BestClassification = d->TrackFilter->GetBestClassifier(track);
    di.Visible = di.DisplayState && (di.BestClassification != -1);

    if (track->GetUseCustomColor())
      {
      double color[3];
      track->GetCustomColor(color);
      di.Color = vgColor(color);
      }
    else
      {
      const vsTrackInfo* const dti = d->trackColorHelper()->infoForTrack(track);
      if (dti)
        {
        di.Color = vgColor(dti->pcolor);
        }
      else if (di.BestClassification != -1)
        {
        const vgColor& fallbackColor = d->TrackColors[vtkVgTrack::Unclassified];
        di.Color = d->TrackColors.value(di.BestClassification, fallbackColor);
        }
      }

    if (di.BestClassification < 0 ||
        di.BestClassification >= vtkVgTrack::Unclassified)
      {
      di.Confidence = -1.0;
      }
    else
      {
      di.Confidence = track->GetPVO()[di.BestClassification];
      }
    }

  return di;
}

//-----------------------------------------------------------------------------
void vsScene::setEventDisplayState(vtkIdType eventId, bool state)
{
  QTE_D(vsScene);

  d->NormalGraph.EventModel->SetEventDisplayState(eventId, state);
  d->GroundTruthGraph.EventModel->SetEventDisplayState(eventId, state);
  this->postUpdate();
}

//-----------------------------------------------------------------------------
bool vsScene::eventDisplayState(vtkIdType eventId)
{
  QTE_D(vsScene);

  vtkVgEventInfo eventInfo = d->eventInfo(eventId);
  return (!!eventInfo.GetEvent() && eventInfo.GetDisplayEvent());
}

//-----------------------------------------------------------------------------
bool vsScene::eventVisibility(vtkIdType eventId)
{
  QTE_D(vsScene);

  vtkVgEventInfo eventInfo = d->eventInfo(eventId);
  if (eventInfo.GetEvent())
    {
    vtkVgEvent* const event = eventInfo.GetEvent();
    const int bestClassifier = d->EventFilter->GetBestClassifier(event);
    return eventInfo.GetDisplayEvent() && (bestClassifier != -1);
    }
  return false; // Event not found
}

//-----------------------------------------------------------------------------
vsDisplayInfo vsScene::eventDisplayInfo(vtkIdType eventId)
{
  QTE_D(vsScene);

  vsDisplayInfo di;
  vtkVgEventInfo eventInfo = d->eventInfo(eventId);
  vtkVgEvent* const event = eventInfo.GetEvent();
  if (event)
    {
    const int type = d->EventFilter->GetBestClassifier(event);
    di.DisplayState = eventInfo.GetDisplayEvent();
    di.BestClassification = type;
    di.Visible = di.DisplayState && (di.BestClassification != -1);
    di.Confidence = (type == -1 ? 0.0 : event->GetProbability(type));

    double color[3];
    if (event->GetUseCustomColor())
      {
      event->GetCustomColor(color);
      di.Color = vgColor(color);
      }
    else if (type != -1)
      {
      const vgEventType& et =
        d->Core->eventTypeRegistry()->GetTypeById(di.BestClassification);
      et.GetColor(color[0], color[1], color[2]);
      di.Color = vgColor(color);
      }
    }

  return di;
}

//-----------------------------------------------------------------------------
vtkVgTrackRepresentationBase::TrackColorMode vsScene::trackColorMode()
{
  QTE_D(vsScene);
  return d->NormalGraph.TrackRepresentation->GetColorMode();
}

//-----------------------------------------------------------------------------
QString vsScene::trackColorDataId()
{
  QTE_D(vsScene);
  return qtString(d->NormalGraph.TrackModel->GetActiveScalars());
}

//-----------------------------------------------------------------------------
void vsScene::setTrackColorMode(
  vtkVgTrackRepresentationBase::TrackColorMode mode, const QString& dataId)
{
  QTE_D(vsScene);

  if (mode == vtkVgTrackRepresentationBase::TCM_Scalars)
    {
    if (dataId.isEmpty())
      {
      qWarning() << "Track coloring mode TCM_Scalars requested, but dataId"
                    " is empty; this won't work... ignoring the request";
      return;
      }

    vtkSmartPointer<vtkLookupTable> lookupTable =
      d->NormalGraph.TrackRepresentation->GetLookupTable();

    if (!lookupTable)
      {
      this->updateTrackColors(mode);
      vtkSmartPointer<vtkLookupTable> lookupTable =
        d->NormalGraph.TrackRepresentation->GetLookupTable();
      }

    vtkVgTrackModel::ScalarsRange range =
      d->NormalGraph.TrackModel->GetScalarsRange(stdString(dataId));
    lookupTable->SetRange(range.lower, range.upper);

    d->NormalGraph.TrackModel->SetActiveScalars(stdString(dataId));
    }

  d->NormalGraph.TrackRepresentation->SetColorMode(mode);
  d->NormalGraph.TrackHeadRepresentation->SetColorMode(mode);

  this->postUpdate();
}

//-----------------------------------------------------------------------------
void vsScene::updateTrackColors(
  vtkVgTrackRepresentationBase::TrackColorMode mode)
{
  QTE_D(vsScene);

  if (mode == vtkVgTrackRepresentationBase::TCM_Scalars)
    {
    vtkSmartPointer<vtkLookupTable> lookupTable =
      d->NormalGraph.TrackRepresentation->GetLookupTable();

    // Create LUT if it doesn't exist yet
    if (!lookupTable)
      {
      static const int numberOfColors = 256; // Hard coded for now
      lookupTable = vtkSmartPointer<vtkLookupTable>::New();
      lookupTable->SetNumberOfTableValues(numberOfColors);

      d->NormalGraph.TrackRepresentation->SetLookupTable(lookupTable);
      d->NormalGraph.TrackHeadRepresentation->SetLookupTable(lookupTable);
      }

    // Get color gradient
    qtGradient gradient;
    vsSettings settings;
    gradient.insertStop(0.0, settings.dataMaxColor());
    gradient.insertStop(1.0, settings.dataMinColor());

    // Fill LUT
    static const int numberOfColors =
      static_cast<int>(lookupTable->GetNumberOfTableValues());
    QList<QColor> colors = gradient.render(numberOfColors);
    for (int i = 0; i < numberOfColors; ++i)
      {
      vgColor color(colors[i]);
      lookupTable->SetTableValue(i, color.data().array);
      }
    }

  d->updateTrackColors();
  this->postUpdate();
}

//-----------------------------------------------------------------------------
QList<vtkVgTrack*> vsScene::trackList() const
  {
  QTE_D_CONST(vsScene);
  return d->TrackTreeModel->trackList();
  }

//-----------------------------------------------------------------------------
QList<vsEventUserInfo> vsScene::eventList() const
{
  QTE_D_CONST(vsScene);
  return d->EventTreeModel->eventList();
}

//-----------------------------------------------------------------------------
void vsScene::writeRenderedImages()
{
  QTE_D(vsScene);

  if (d->WriteRenderedImages)
    {
    d->WindowToImageFilter->Modified();
    QChar zero('0');
    const QString outputFileName =
      d->ImageOutputDirectory +
      QString("/vsPlayImage%1.png").arg(d->ImageCounter++, 6, 10, zero);
    d->PngWriter->SetFileName(qPrintable(outputFileName));
    d->PngWriter->Write();
    }
  if (d->SaveScreenShot)
    {
    d->SaveScreenShot = false; // this is one shot
    d->WindowToImageFilter->Modified();
    d->PngWriter->SetFileName(qPrintable(d->ScreenShotFileName));
    d->PngWriter->Write();
    }
}

//-----------------------------------------------------------------------------
void vsScene::setWriteRenderedImages(bool state, QString* outputDir/*=0*/)
{
  QTE_D(vsScene);

  d->WriteRenderedImages = state;
  if (outputDir)
    {
    d->ImageOutputDirectory = *outputDir;
    }
}

//-----------------------------------------------------------------------------
void vsScene::saveScreenShot(QString* filePath)
{
  QTE_D(vsScene);

  d->ScreenShotFileName = *filePath;
  d->SaveScreenShot = true;

  // cause render
  d->RenderWindow->Render();
}

//-----------------------------------------------------------------------------
bool vsScene::setMaskImage(const QString& fileName)
{
  QTE_D(vsScene);

  vtkVgInstance<vtkPNGReader> reader;
  if (!reader->CanReadFile(qPrintable(fileName)))
    {
    return false;
    }

  reader->SetFileName(qPrintable(fileName));
  reader->Update();
  d->TrackingMask->SetInputData(reader->GetOutput());
  return true;
}
