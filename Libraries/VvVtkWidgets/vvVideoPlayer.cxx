// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvVideoPlayer.h"
#include "vvVideoPlayerPrivate.h"

#include "ui_videoView.h"

// C++ includes
#include <limits>

// Qt includes
#include <QActionGroup>
#include <QList>
#include <QSettings>
#include <QTimer>

// Qt Extensions includes
#include <qtDoubleInputDialog.h>
#include <qtScopedValueChange.h>
#include <qtUiState.h>
#include <qtUtil.h>

// visgui includes
#include <vgCheckArg.h>

#include <vgUnixTime.h>

#include <vtkVgQtUtil.h>

#include <vtkVgRendererUtils.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgGroupNode.h>
#include <vtkVgInteractorStyleRubberBand2D.h>
#include <vtkVgTrackRepresentationBase.h>
#include <vtkVgVideoMetadata.h>
#include <vtkVgVideoModel0.h>
#include <vtkVgVideoNode.h>
#include <vtkVgVideoRepresentation0.h>
#include <vtkVgVideoProviderBase.h>
#include <vtkVgVideoViewer.h>

#include "vvVideoPlayerModel.h"

// VTK includes
#include <vtkActor.h>
#include <vtkBoundingBox.h>
#include <vtkCamera.h>
#include <vtkLine.h>
#include <vtkProperty.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTimerLog.h>

QTE_IMPLEMENT_D_FUNC(vvVideoPlayer)

//-----------------------------------------------------------------------------
vvVideoPlayerPrivate::vvVideoPlayerPrivate()
  : IsExternal(false), AutoZoomEnabled(false), AllowPicking(false),
    ResetView(1), HasUpdated(false), UpdatingFrameScrubber(false),
    Seeking(false), UpdateIntervalMSec(100), PlaybackSpeed(1.0)
{
  this->TimeStamp.SetTime(1.0);
  this->CurrentTime = 0.0;

  this->FrameExtents[0] = this->FrameExtents[1] = 0.0;
  this->FrameExtents[2] = this->FrameExtents[3] = 0.0;

  this->CurrentExtents[0] = this->CurrentExtents[1] = 0.0;
  this->CurrentExtents[2] = this->CurrentExtents[3] = 0.0;

  this->CurrentCenter[0] = 0.0;
  this->CurrentCenter[1] = 0.0;
  this->CurrentCenter[2] = 0.0;

  this->TimerLog = vtkSmartPointer<vtkTimerLog>::New();
  this->PlayerInitTime = this->TimerLog->GetUniversalTime();
}

//-----------------------------------------------------------------------------
vvVideoPlayerPrivate::~vvVideoPlayerPrivate()
{
}

//-----------------------------------------------------------------------------
vvVideoPlayer::vvVideoPlayer(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f), d_ptr(new vvVideoPlayerPrivate)
{
  this->construct();
}

//-----------------------------------------------------------------------------
vvVideoPlayer::vvVideoPlayer(
  vvVideoPlayerPrivate* d, QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f), d_ptr(d)
{
  this->construct();
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::construct()
{
  QTE_D(vvVideoPlayer);

  d->Viewer = vtkSmartPointer<vtkVgVideoViewer>::New();

  d->UI.setupUi(this);

  QLabel* label = new QLabel(this);
  label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  d->UI.toolBar->insertWidget(d->UI.actionResetView, label);

  // Disable everything and clear playback controls state until we have a video
  this->setEnabled(false);
  d->UI.actionPlay->setChecked(false);
  d->UI.actionPause->setChecked(false);
  d->UI.actionStop->setChecked(false);

  QActionGroup* playbackGroup = new QActionGroup(this);
  playbackGroup->addAction(d->UI.actionPlay);
  playbackGroup->addAction(d->UI.actionPause);
  playbackGroup->addAction(d->UI.actionStop);

  connect(this, SIGNAL(videoPlaying(vtkVgNodeBase&)),
          this, SLOT(onVideoPlaying()));
  connect(this, SIGNAL(videoPaused(vtkVgNodeBase&)),
          this, SLOT(onVideoPaused()));
  connect(this, SIGNAL(videoStopped(vtkVgNodeBase&)),
          this, SLOT(onVideoStopped()));

  d->UiState.reset(new qtUiState);
  d->UiState->setCurrentGroup("VideoPlayer");
  d->UiState->mapChecked("ShowEventRegion", d->UI.actionShowRegion);
  d->UiState->mapChecked("FollowEvent", d->UI.actionFollowEvent);
  d->UiState->restore();

  connect(d->UI.actionShowRegion, SIGNAL(toggled(bool)),
          this, SLOT(setEventRegionVisible(bool)));

  connect(d->UI.actionFollowEvent, SIGNAL(toggled(bool)),
          this, SLOT(setEventFollowingEnabled(bool)));

  connect(d->UI.actionIncreaseSpeed, SIGNAL(triggered(bool)) ,
          this, SLOT(doubleThePlaybackSpeed()));
  connect(d->UI.actionDecreaseSpeed, SIGNAL(triggered(bool)) ,
          this, SLOT(reducePlaybackSpeedByHalf()));

  this->updatePlaybackSpeedDisplay();
}

//-----------------------------------------------------------------------------
vvVideoPlayer::~vvVideoPlayer()
{
  QTE_D(vvVideoPlayer);

  if (!d->IsExternal)
    {
    d->UiState->save();
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onExternalPlay(vtkVgVideoNode& videoNode, bool usingSelection)
{
  QTE_D(vvVideoPlayer);

  if (&videoNode != d->CurrentVideoPicked)
    {
    // Stop the last video.
    this->onActionStop();

    // Create a video node and share video sources.
    d->CurrentVideo = vtkVgVideoNode::SmartPtr::New();
    d->CurrentVideoPicked = &videoNode;

    if (d->UI.actionFollowEvent->isChecked())
      {
      // Block automatic update until we are done setting up the video; prevents
      // flicker of video zoomed to extents rather than follow mode.
      qtScopedBlockUpdates bu(this);
      // Request zoom on next update, and disable panning.
      d->AutoZoomEnabled = true;
      this->setEventFollowingEnabled(true);
      }

    vtkVgVideoRepresentationBase0* videoRep =
      this->buildVideoRepresentation(videoNode);

    // Set current playback speed to the newly created model
    if (vtkVgVideoModel0* newVideoModel = videoRep->GetVideoModel())
      {
      newVideoModel->SetPlaybackSpeed(d->PlaybackSpeed);
      }

    // Show events - reuse the original event model but create a new representation.
    vtkVgEventModel* eventModel =
      videoNode.GetVideoRepresentation()->GetVideoModel()->GetEventModel();

    vtkVgEventRegionRepresentation* eventRep =
      vtkVgEventRegionRepresentation::New();
    eventRep->SetLineWidth(2.0f);

    videoRep->GetVideoModel()->SetEventModel(eventModel);
    eventRep->SetEventModel(eventModel);

    videoRep->SetEventRepresentation(eventRep);

    this->updateState(eventRep);

    eventRep->FastDelete();

    d->CurrentVideo->SetVideoRepresentation(videoRep);
    videoRep->FastDelete();

    d->Viewer->SetSceneRoot(d->CurrentVideo);

    vtkVgVideoProviderBase* vs = videoRep->GetVideoModel()->GetVideoSource();
    this->setupFrameScrubber(vs);

    vtkVgVideoFrameData frameData;
    vs->GetCurrentFrame(&frameData);
    int* dim = frameData.VideoImage->GetDimensions();

    d->FrameExtents[0] = 0.0;
    d->FrameExtents[1] = dim[0] - 1.0;
    d->FrameExtents[2] = 0.0;
    d->FrameExtents[3] = dim[1] - 1.0;

    this->updateCurrentTime();

    d->ResetView = 1;
    this->update();

    // FIXME: Why do we need this?
    if (usingSelection)
      {
      this->forceUpdateVideoRepresentation();
      }
    }

  // Enable UI
  this->setEnabled(true);
  d->UI.actionPause->setEnabled(true);

  // Synchronize state.
  vtkVgVideoModel0* videoModel = this->currentPickedVideoModel();
  videoModel->SetPlaybackSpeed(d->PlaybackSpeed);
  if (videoModel->IsPlaying())
    {
    d->UI.actionPlay->setChecked(true);
    emit this->videoPlaying(videoNode);
    }
  else if (videoModel->IsPaused())
    {
    d->UI.actionPause->setChecked(true);
    emit this->videoPaused(videoNode);
    }
  else if (videoModel->IsStopped())
    {
    d->UI.actionStop->setChecked(true);
    emit this->videoStopped(videoNode);
    }

  emit this->currentVideoChanged(d->CurrentVideoPicked);
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onExternalPlay(vtkVgNodeBase& node)
{
  QTE_D(vvVideoPlayer);

  vtkVgVideoNode* videoNode = vtkVgVideoNode::SafeDownCast(&node);
  if (videoNode && videoNode != d->CurrentVideoPicked)
    {
    this->onExternalPlay(*videoNode, true);
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onExternalStop(vtkVgVideoNode& videoNode)
{
  QTE_D(vvVideoPlayer);

  if (d->CurrentVideoPicked == &videoNode)
    {
    d->UI.actionStop->setChecked(true);
    d->UI.actionPause->setEnabled(false);

    if (d->CurrentVideoPicked->GetVideoRepresentation()
        ->GetVideoModel()->IsStopped())
      {
      emit this->videoStopped(videoNode);
      }
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onExternalPause(vtkVgVideoNode& videoNode)
{
  QTE_D(vvVideoPlayer);

  if (d->CurrentVideoPicked == &videoNode)
    {
    d->UI.actionPause->setChecked(true);

    if (d->CurrentVideoPicked->GetVideoRepresentation()
        ->GetVideoModel()->IsPaused())
      {
      emit this->videoPaused(videoNode);
      }
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onExternalSelect(const QList<vtkVgNodeBase*>& nodes)
{
  // Do nothing if the selection is empty or is multiple selection.
  CHECK_ARG(nodes.count() == 1);

  if (vtkVgVideoModel0* videoModel = this->currentPickedVideoModel())
    {
    if (videoModel->IsPlaying())
      {
      return;
      }
    }

  QTE_D(vvVideoPlayer);

  // Select only the last one.
  vtkVgVideoNode* videoNode = vtkVgVideoNode::SafeDownCast(nodes[0]);
  if (!videoNode)
    {
    return;
    }

  if (!videoNode->GetHasVideoData())
    {
    this->reset();
    }
  else if (videoNode != d->CurrentVideoPicked)
    {
    this->onExternalPlay(*videoNode, true);
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onFrameAvailable()
{
  QTE_D(vvVideoPlayer);

  CHECK_ARG(d->CurrentVideoPicked);
  CHECK_ARG(d->CurrentVideo);

  if (d->Seeking)
    {
    d->Seeking = false;
    this->updateCurrentTime();
    }

  vtkVgVideoModel0* model =
    d->CurrentVideoPicked->GetVideoRepresentation()->GetVideoModel();

  (model->IsPlaying() ? emit this->videoPlaying(*d->CurrentVideoPicked) :
   model->IsPaused() ? emit this->videoPaused(*d->CurrentVideo) :
   qt_noop());

  this->update();
}

//-----------------------------------------------------------------------------
vtkVgVideoModel0* vvVideoPlayer::currentPickedVideoModel()
{
  QTE_D(vvVideoPlayer);

  vtkVgVideoNode* node = d->CurrentVideoPicked;
  return (node ? node->GetVideoRepresentation()->GetVideoModel() : 0);
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onActionPlay()
{
  QTE_D(vvVideoPlayer);

  d->UI.actionPlay->setChecked(true);
  d->UI.actionPause->setEnabled(true);

  if (d->CurrentVideoPicked && !this->currentPickedVideoModel()->IsPlaying())
    {
    this->renderLoopOn();

    this->currentPickedVideoModel()->PlayFromBeginningOff();
    this->currentPickedVideoModel()->SetPlaybackSpeed(d->PlaybackSpeed);
    this->currentPickedVideoModel()->Play();
    emit this->videoPlaying(*d->CurrentVideoPicked);
    emit this->played(*d->CurrentVideoPicked);
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onActionPause()
{
  QTE_D(vvVideoPlayer);

  d->UI.actionPause->setChecked(true);

  if (d->CurrentVideoPicked && !this->currentPickedVideoModel()->IsPaused())
    {
    this->currentPickedVideoModel()->Pause();
    emit this->videoPaused(*d->CurrentVideoPicked);
    }

  this->renderLoopOff();
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onActionStop()
{
  QTE_D(vvVideoPlayer);

  d->UI.actionStop->setChecked(true);
  d->UI.actionPause->setEnabled(false);

  if (d->CurrentVideoPicked && !this->currentPickedVideoModel()->IsStopped())
    {
    this->currentPickedVideoModel()->Stop();
    this->seekTo(this->currentPickedVideoModel()->GetCurrentSeekTime());
    emit this->stopped(*d->CurrentVideoPicked);
    emit this->videoStopped(*d->CurrentVideoPicked);
    }

  if (d->CurrentVideo)
    {
    d->CurrentVideo->GetVideoRepresentation()->GetVideoModel()->Stop();
    }

  this->renderLoopOff();
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onActionNext()
{
  QTE_D(vvVideoPlayer);

  vtkVgVideoModel0::SmartPtr videoModel = this->currentPickedVideoModel();

  if (videoModel)
    {
    if (videoModel->IsPlaying())
      {
      d->UI.actionPause->setChecked(true);
      videoModel->Pause();
      }

    if (videoModel->IsPaused())
      {
      emit this->videoPaused(*d->CurrentVideoPicked);
      }

    videoModel->Next();
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onActionPrev()
{
  QTE_D(vvVideoPlayer);

  vtkVgVideoModel0::SmartPtr videoModel = this->currentPickedVideoModel();

  if (videoModel)
    {
    if (videoModel->IsPlaying())
      {
      d->UI.actionPause->setChecked(true);
      videoModel->Pause();
      }

    if (videoModel->IsPaused())
      {
      emit this->videoPaused(*d->CurrentVideoPicked);
      }

    videoModel->Previous();
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::calculateExtentsScale(double* center, double maxX, double maxY,
                                          double& scaleX, double& scaleY)
{
  // Default is fast path, meaning calculate extents based on a fast algorithm
  // that mostly (99% times) going to perform well in all situations. This algorithm
  // calculates the area overlap assuming that region should mostly be on the video
  // clip.
  bool fastPath = true;

  // Default is 4.0
  scaleX = 4.0;
  scaleY = 4.0;

  if (!center)
    {
    return;
    }

  int* extents;
  extents = this->currentPickedVideoModel()->GetFrameData()->VideoImage->GetExtent();
  double imageExtents[4] =
    {
    static_cast<double>(extents[0]),
    static_cast<double>(extents[1]),
    static_cast<double>(extents[2]),
    static_cast<double>(extents[3])
    };
  double imageArea =
    (imageExtents[1] - imageExtents[0]) * (imageExtents[3] - imageExtents[2]);

  // Area of the region
  double area = maxX * maxY;

  if (!fastPath)
    {
    double aoiExtents[4];
    aoiExtents[0] = center[0] - (maxX / 2.0);
    aoiExtents[1] = center[0] + (maxX / 2.0) - 1.0;
    aoiExtents[2] = center[1] - (maxY / 2.0);
    aoiExtents[3] = center[1] + (maxY / 2.0) - 1.0;

    double imageLines[4][6] =
      {
        { imageExtents[0], imageExtents[2], 0.0, imageExtents[1], imageExtents[2], 0.0 },
        { imageExtents[0], imageExtents[2], 0.0, imageExtents[0], imageExtents[3], 0.0 },
        { imageExtents[0], imageExtents[3], 0.0, imageExtents[1], imageExtents[3], 0.0 },
        { imageExtents[1], imageExtents[2], 0.0, imageExtents[1], imageExtents[3], 0.0 }
      };

    double aoiLines[4][6] =
      {
        { aoiExtents[0], aoiExtents[2], 0.0, aoiExtents[1], aoiExtents[2], 0.0 },
        { aoiExtents[0], aoiExtents[2], 0.0, aoiExtents[0], aoiExtents[3], 0.0 },
        { aoiExtents[0], aoiExtents[3], 0.0, aoiExtents[1], aoiExtents[3], 0.0 },
        { aoiExtents[1], aoiExtents[2], 0.0, aoiExtents[1], aoiExtents[3], 0.0 }
      };

    std::vector<std::pair<double, double> > intersectedPoints;

    // First find all intersected points
    for (int i = 0; i < 4; ++i)
      {
      for (int j = 0; j < 4; ++j)
        {
        double u, v;
        if (vtkLine::Intersection(&imageLines[i][0], &imageLines[i][3],
                                  &aoiLines[j][0], &aoiLines[j][3], u, v) == 2)
          {
          double point[2] =
            {
            imageLines[i][0] + (imageLines[i][3] - imageLines[i][0])* u,
            imageLines[i][1] + (imageLines[i][4] - imageLines[i][1])* u
            };

          intersectedPoints.push_back(std::pair<double, double>(point[0], point[1]));
          }
        }
      }

    // Now check if points are contained within eacth other
    vtkBoundingBox imageBB;
    imageBB.SetBounds(imageExtents[0], imageExtents[1],
                      imageExtents[2], imageExtents[3], 0.0, 0.0);

    vtkBoundingBox aoiBB;
    aoiBB.SetBounds(aoiExtents[0], aoiExtents[1], aoiExtents[2],
                    aoiExtents[3], 0.0, 0.0);

    // Points
    double aoiPoints[4][3] =
      {
        { aoiExtents[0], aoiExtents[2], 0.0 },
        { aoiExtents[1], aoiExtents[2], 0.0 },
        { aoiExtents[1], aoiExtents[3], 0.0 },
        { aoiExtents[0], aoiExtents[3], 0.0 },
      };

    double imagePoints[4][3] =
      {
        { imageExtents[0], imageExtents[2], 0.0 },
        { imageExtents[1], imageExtents[2], 0.0 },
        { imageExtents[1], imageExtents[3], 0.0 },
        { imageExtents[0], imageExtents[3], 0.0 },
      };

    for (int i = 0; i < 4; ++i)
      {
      if (imageBB.ContainsPoint(&aoiPoints[i][0]))
        {
        intersectedPoints.push_back(
          std::pair<double, double>(aoiPoints[i][0], aoiPoints[i][1]));
        }
      if (aoiBB.ContainsPoint(&imagePoints[i][0]))
        {
        intersectedPoints.push_back(
          std::pair<double, double>(imagePoints[i][0], imagePoints[i][1]));
        }
      }

    if (intersectedPoints.size() > 2)
      {
      // Find min and max points.
      double min[2], max[2];
      min[0] = max[0] = intersectedPoints[0].first;
      min[1] = max[1] = intersectedPoints[0].second;

      for (size_t i = 1; i < intersectedPoints.size(); ++i)
        {
        if (intersectedPoints[i].first < min[0])
          {
          min[0] = intersectedPoints[i].first;
          }
        if (intersectedPoints[i].second < min[1])
          {
          min[1] = intersectedPoints[i].second;
          }
        if (intersectedPoints[i].first > max[0])
          {
          max[0] = intersectedPoints[i].first;
          }
        if (intersectedPoints[i].second > max[1])
          {
          max[1] = intersectedPoints[i].second;
          }
        }

      // Calculate overlapped area
      area = (max[0] - min[0]) * (max[1] - min[1]);
      }
    }

  double ratio = std::numeric_limits<double>::max();
  if (area > 0.0 && imageArea > 0.0)
    {
    ratio = area / imageArea;
    }

  // Higher the ratio, lesser the zoom we desire.
  if (ratio > 1.0)
    {
    scaleX = 1.0;
    scaleY = scaleX;
    }
  else if (ratio > 0.1 && ratio <= 1.0)
    {
    double initialValue = 1.0;
    double range = 1.0;

    scaleX = initialValue + range * (1.0 - ratio);
    scaleY = scaleX;
    }
  else
    {
    scaleX = 4.0;
    scaleY = scaleX;
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::renderLoopOn()
{
  QTE_D(vvVideoPlayer);

  d->UpdateTimer->start(d->UpdateIntervalMSec);
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::renderLoopOff()
{
  QTE_D(vvVideoPlayer);

  d->UpdateTimer->stop();
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onAreaOfInterest(double* center, double maxX, double maxY)
{
  QTE_D(vvVideoPlayer);

  // Check if the incoming the region formed by center, maxX and maxY covers
  // encloses most of the video space (hence mostly overlaps) if yes then do
  // not scale region of interest.

  double scaleX, scaleY;
  this->calculateExtentsScale(center, maxX, maxY, scaleX, scaleY);

  maxX *= scaleX;
  maxY *= scaleY;

  d->CurrentExtents[0] = center[0] - (maxX / 2.0);
  d->CurrentExtents[1] = center[0] + (maxX / 2.0) - 1.0;
  d->CurrentExtents[2] = center[1] - (maxY / 2.0);
  d->CurrentExtents[3] = center[1] + (maxY / 2.0) - 1.0;

  d->CurrentCenter[0] = center[0];
  d->CurrentCenter[1] = center[1];
  d->CurrentCenter[2] = center[2];
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::seekTo(double value)
{
  QTE_D(vvVideoPlayer);

  if (d->UpdatingFrameScrubber || value == d->CurrentTime)
    {
    return;
    }

  vtkVgVideoModel0::SmartPtr videoModel = this->currentPickedVideoModel();

  if (videoModel)
    {
    if (videoModel->IsPlaying())
      {
      videoModel->Pause();
      emit this->videoPaused(*d->CurrentVideoPicked);
      }

    d->Seeking = true;
    if (videoModel->SeekTo(value) == VTK_OK)
      {
      // The seek completed synchronously.
      d->Seeking = false;
      this->updateCurrentTime();
      this->update();
      }
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::setEventRegionVisible(bool state)
{
  QTE_D(vvVideoPlayer);

  if (d->CurrentVideo)
    {
    vtkVgVideoRepresentationBase0* videoRep =
      d->CurrentVideo->GetVideoRepresentation();
    if (videoRep)
      {
      vtkVgEventRepresentationBase* eventRep
        = videoRep->GetEventRepresentation();
      if (eventRep)
        {
        eventRep->SetVisible(state);
        eventRep->Update();
        this->update();
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::setEventFollowingEnabled(bool state)
{
  QTE_D(vvVideoPlayer);

  vtkVgInteractorStyleRubberBand2D* style =
    vtkVgInteractorStyleRubberBand2D::SafeDownCast(
      d->UI.renderWidget->GetInteractor()->GetInteractorStyle());

  if (!style)
    {
    return;
    }

  // Enable panning iff we are not following an event.
  style->SetAllowPanning(!state);
  d->AutoZoomEnabled = true;
  this->update();
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::initialize(bool allowPicking)
{
  QTE_D(vvVideoPlayer);

  vtkVgInteractorStyleRubberBand2D* style = vtkVgInteractorStyleRubberBand2D::New();
  d->UI.renderWidget->GetInteractor()->SetInteractorStyle(style);
  style->SetRenderer(d->Viewer->GetSceneRenderer());
  style->FastDelete();

  d->Viewer->SetRenderWindowInteractor(
    d->UI.renderWidget->GetInteractor());

  d->UI.renderWidget->SetRenderWindow(d->Viewer->GetRenderWindow());

  if (!d->UpdateTimer)
    {
    d->UpdateTimer.reset(new QTimer());
    connect(d->UpdateTimer.data(), SIGNAL(timeout()),
            this, SLOT(update()));
    }

  d->AllowPicking = allowPicking;

  // rubberband zoom... and/or maybe picking (depending on AllowPicking flag)
  vtkConnect(
    d->UI.renderWidget->GetInteractor()->GetInteractorStyle(),
    vtkVgInteractorStyleRubberBand2D::LeftClickEvent,
    this, SLOT(onLeftButtonClicked()));

  d->Viewer->Run();

  connect(d->UI.actionPlay, SIGNAL(triggered()),
          this, SLOT(onActionPlay()));
  connect(d->UI.actionPause, SIGNAL(triggered()),
          this, SLOT(onActionPause()));
  connect(d->UI.actionStop, SIGNAL(triggered()),
          this, SLOT(onActionStop()));
  connect(d->UI.actionFramePrevious, SIGNAL(triggered()),
          this, SLOT(onActionPrev()));
  connect(d->UI.actionFrameNext, SIGNAL(triggered()),
          this, SLOT(onActionNext()));

  connect(this, SIGNAL(timeChanged(double)),
          d->UI.frameScrubber, SLOT(setTime(double)));
  connect(d->UI.frameScrubber, SIGNAL(timeChanged(double)),
          this, SLOT(seekTo(double)));

  // reset view
  vtkConnect(style, vtkVgInteractorStyleRubberBand2D::KeyPressEvent_R,
             this, SLOT(resetView()));
  connect(d->UI.actionResetView, SIGNAL(triggered()),
          this, SLOT(resetView()));

  d->FrameExtents[0] = d->FrameExtents[1] =
                         d->FrameExtents[2] = d->FrameExtents[3] = 0.0;
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::reset()
{
  QTE_D(vvVideoPlayer);

  d->Viewer->SetSceneRoot(vtkVgGroupNode::SmartPtr::New());
  d->UI.renderWidget->update();

  d->CurrentVideoPicked = 0;
  d->CurrentVideo = 0;
  d->HasUpdated = false;

  // Disable everything and clear playback controls state until we have a video
  this->setEnabled(false);
  d->UI.actionPlay->setChecked(false);
  d->UI.actionPause->setChecked(false);
  d->UI.actionStop->setChecked(false);
  d->UI.frameScrubber->setTimeRange(0.0, 0.0);

  emit this->currentVideoChanged(0);
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::setPlaybackEnabled(bool state)
{
  QTE_D(vvVideoPlayer);

  bool isStopped = d->UI.actionStop->isChecked();
  d->UI.actionPlay->setEnabled(state);
  d->UI.actionPause->setEnabled(state && !isStopped);
  d->UI.actionStop->setEnabled(state);
  d->UI.actionFramePrevious->setEnabled(state);
  d->UI.actionFrameNext->setEnabled(state);
  d->UI.frameScrubber->setEnabled(state);
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::doubleThePlaybackSpeed()
{
  QTE_D(vvVideoPlayer);

  d->PlaybackSpeed *= 2.0;

  d->PlaybackSpeed = (d->PlaybackSpeed > 4.0) ? 4.0 : d->PlaybackSpeed;

  this->currentPickedVideoModel()->SetPlaybackSpeed(d->PlaybackSpeed);

  this->updatePlaybackSpeedDisplay();
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::reducePlaybackSpeedByHalf()
{
  QTE_D(vvVideoPlayer);

  d->PlaybackSpeed *= 0.5;

  d->PlaybackSpeed = (d->PlaybackSpeed < 0.25) ? 0.25 : d->PlaybackSpeed;

  this->currentPickedVideoModel()->SetPlaybackSpeed(d->PlaybackSpeed);

  this->updatePlaybackSpeedDisplay();
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::resetView()
{
  QTE_D(vvVideoPlayer);

  if (d->FrameExtents[0] == 0.0 && d->FrameExtents[1] == 0.0)
    {
    vtkVgVideoProviderBase* videoSource =
      d->CurrentVideoPicked->GetVideoRepresentation()
        ->GetVideoModel()->GetVideoSource();

    vtkVgVideoFrameData frameData;
    videoSource->GetCurrentFrame(&frameData);
    int* dim = frameData.VideoImage->GetDimensions();

    // Don't reset bounds if we haven't received a valid frame yet.
    if (dim[0] == 0)
      {
      return;
      }

    d->FrameExtents[0] = 0.0;
    d->FrameExtents[1] = dim[0] - 1.0;
    d->FrameExtents[2] = 0.0;
    d->FrameExtents[3] = dim[1] - 1.0;
    }

  vtkVgRendererUtils::ZoomToExtents2D(d->Viewer->GetSceneRenderer(),
                                      d->FrameExtents);
  this->update();
  d->ResetView = 0;
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::update()
{
  QTE_D(vvVideoPlayer);

  if (this->updatesEnabled() && d->CurrentVideoPicked)
    {
    qtScopedBlockUpdates bu(this); // prevent recursively updates
    this->updateFrame();
    if (d->ResetView)
      {
      d->Viewer->ResetView();
      this->resetView();
      }
    else
      {
      if (d->UI.actionFollowEvent->isChecked())
        {
        this->autoCenterUpdate();
        }

      d->Viewer->ForceRender(false);
      }
    this->onUpdate();
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::updateFrame()
{
  QTE_D(vvVideoPlayer);

  // Microseconds is what we are using
  d->TimeStamp.SetTime(
    1e6 * (d->TimerLog->GetUniversalTime() - d->PlayerInitTime));

  d->Viewer->Frame(d->TimeStamp);
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onVideoPlaying()
{
  QTE_D(vvVideoPlayer);
  d->UI.actionPlay->setChecked(true);
  d->UI.actionPause->setEnabled(true);
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onVideoPaused()
{
  QTE_D(vvVideoPlayer);
  d->UI.actionPause->setChecked(true);
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onVideoStopped()
{
  QTE_D(vvVideoPlayer);
  d->UI.actionStop->setChecked(true);
  d->UI.actionPause->setEnabled(false);
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onLeftButtonClicked()
{
  QTE_D(vvVideoPlayer);

  CHECK_ARG(d->AllowPicking);
  CHECK_ARG(d->CurrentVideo);

  vtkRenderWindowInteractor* interactor =
    d->Viewer->GetRenderWindow()->GetInteractor();

  int x, y;
  interactor->GetEventPosition(x, y);

  // check for event pick
  vtkVgEventRepresentationBase* eventRep =
    d->CurrentVideo->GetVideoRepresentation()->GetEventRepresentation();

  vtkIdType pickType;
  const vtkIdType eventId =
    eventRep->Pick(x, y, d->Viewer->GetSceneRenderer(), pickType);
  if (eventId != -1)
    {
    emit this->pickedEvent(eventId);
    return;
    }

  // check for track pick
  vtkVgTrackRepresentationBase* trackRep =
    d->CurrentVideo->GetVideoRepresentation()->GetTrackRepresentation();

  const vtkIdType trackId =
    trackRep->Pick(x, y, d->Viewer->GetSceneRenderer(), pickType);
  if (trackId != -1)
    {
    emit this->pickedTrack(trackId);
    return;
    }
}

//-----------------------------------------------------------------------------
// 99% copied from vqCore... which not in love with, but will do for now
bool vvVideoPlayer::doRubberBandZoom()
{
  QTE_D(vvVideoPlayer);

  vtkVgInteractorStyleRubberBand2D* style =
    vtkVgInteractorStyleRubberBand2D::SafeDownCast(
      d->UI.renderWidget->GetInteractor()->GetInteractorStyle());

  int minRubberBand = 5;

  int* startPosition = style->GetStartPosition();
  int* endPosition =  style->GetEndPosition();
  if (abs(startPosition[0] - endPosition[0]) < minRubberBand ||
      abs(startPosition[1] - endPosition[1]) < minRubberBand)
    {
    return false;  // too small... assume NOT a rubberband zoom
    }

  vtkRenderer* renderer = d->Viewer->GetSceneRenderer();

  int* size = renderer->GetSize();
  double halfWidth = size[0] / 2.0;
  double halfHeight = size[1] / 2.0;

  double startWorldPosition[4], endWorldPosition[4];
  renderer->SetViewPoint(
    (startPosition[0] - halfWidth) / halfWidth,
    (startPosition[1] - halfHeight) / halfHeight,
    1);
  renderer->ViewToWorld();
  renderer->GetWorldPoint(startWorldPosition);

  renderer->SetViewPoint(
    (endPosition[0] - halfWidth) / halfWidth,
    (endPosition[1] - halfHeight) / halfHeight,
    1);
  renderer->ViewToWorld();
  renderer->GetWorldPoint(endWorldPosition);

  double targetExtents[4] =
    {
    startWorldPosition[0] < endWorldPosition[0] ? startWorldPosition[0] : endWorldPosition[0],
    startWorldPosition[0] > endWorldPosition[0] ? startWorldPosition[0] : endWorldPosition[0],
    startWorldPosition[1] < endWorldPosition[1] ? startWorldPosition[1] : endWorldPosition[1],
    startWorldPosition[1] > endWorldPosition[1] ? startWorldPosition[1] : endWorldPosition[1]
    };

  vtkVgRendererUtils::ZoomToExtents2D(renderer, targetExtents);
  return true;
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::loadExternal(vtkVgVideoNode& videoNode)
{
  QTE_D(vvVideoPlayer);

  d->IsExternal = true;

  // When loading an external video do not give option to auto center
  // and disable it if it is on and hide it.
  d->UI.actionFollowEvent->setChecked(false);
  d->UI.toolBar->removeAction(d->UI.actionFollowEvent);

  // When loading an external video do not give option to enable / disable
  // event region from the video player.
  d->UI.actionShowRegion->setChecked(false);
  d->UI.toolBar->removeAction(d->UI.actionShowRegion);
  foreach (QAction* a, d->UI.toolBar->actions())
    {
    if (a->isSeparator())
      {
      d->UI.toolBar->removeAction(a);
      delete a;
      }
    }
  d->UI.toolBar->insertSeparator(d->UI.actionFramePrevious);

  // Enable UI
  this->setEnabled(true);
  d->UI.actionStop->setChecked(true);
  d->UI.actionPause->setEnabled(false);

  d->CurrentVideo = &videoNode;
  d->CurrentVideoPicked = d->CurrentVideo;

  d->Viewer->SetSceneRoot(d->CurrentVideo);
  d->Viewer->Frame(d->TimeStamp);

  vtkVgVideoProviderBase* videoSource =
    videoNode.GetVideoRepresentation()->GetVideoModel()->GetVideoSource();

  this->setupFrameScrubber(videoSource);

  this->updateCurrentTime();

  // Render the first frame.
  d->ResetView = 1;

  vvVideoPlayerModel* vpm =
    vvVideoPlayerModel::SafeDownCast(
      videoNode.GetVideoRepresentation()->GetVideoModel());

  // If the video source is asynchronous, post an initial update to it and wait
  // for the first frame to become available before updating the player.
  if (vpm && vpm->GetSharingSource())
    {
    d->UpdateTimer->stop();
    vpm->GetVideoSource()->Update();
    }
  else
    {
    this->update();
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::onUpdate()
{
  this->updateCurrentTime();
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::setupFrameScrubber(vtkVgVideoProviderBase* vs)
{
  QTE_D(vvVideoPlayer);

  double timeRange[2];
  vs->GetTimeRange(timeRange);

  qtScopedValueChange<bool> x(d->UpdatingFrameScrubber, true);
  d->UI.frameScrubber->setTimeRange(timeRange[0], timeRange[1]);
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::updateState(vtkVgEventRepresentationBase* eventRep)
{
  QTE_D(vvVideoPlayer);
  if (eventRep)
    {
    eventRep->SetVisible(d->UI.actionShowRegion->isChecked());
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::updatePlaybackSpeedDisplay()
{
  QTE_D(vvVideoPlayer);

  QString speedStr;

  if (!qFuzzyIsNull(d->PlaybackSpeed) && fabs(d->PlaybackSpeed) < 0.9)
    {
    const int frac = qRound(1.0 / fabs(d->PlaybackSpeed));
    const char* sc = (d->PlaybackSpeed < 0.0 ? "-" : "");
    speedStr = QString("%1<sup>1</sup>/<sub>%2</sub>x").arg(sc).arg(frac);
    }
  else
    {
    speedStr = QString("%1x").arg(d->PlaybackSpeed);
    }

  d->UI.playbackSpeedLabel->setText(speedStr);
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::moveCameraTo(double pos[2])
{
  QTE_D(vvVideoPlayer);
  vtkCamera* camera = d->Viewer->GetSceneRenderer()->GetActiveCamera();

  double lastFocalPt[3], lastPos[3];
  camera->GetFocalPoint(lastFocalPt);
  camera->GetPosition(lastPos);

  camera->SetFocalPoint(pos[0], pos[1], lastFocalPt[2]);
  camera->SetPosition(pos[0], pos[1], lastPos[2]);
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::autoCenterUpdate()
{
  QTE_D(vvVideoPlayer);

  if (d->AutoZoomEnabled)
    {
    vtkVgRendererUtils::ZoomToExtents2D(d->Viewer->GetSceneRenderer(),
                                        d->CurrentExtents);
    d->AutoZoomEnabled = false;
    }
  else
    {
    vtkCamera* camera = d->Viewer->GetSceneRenderer()->GetActiveCamera();
    double cameraCurrentPosition[3];
    double cameraCurrentFocalPoint[3];
    camera->GetPosition(cameraCurrentPosition);
    camera->GetFocalPoint(cameraCurrentFocalPoint);
    camera->SetFocalPoint(d->CurrentCenter[0], d->CurrentCenter[1],
                          cameraCurrentFocalPoint[2]);
    camera->SetPosition(d->CurrentCenter[0], d->CurrentCenter[1],
                        cameraCurrentPosition[2]);
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::updateCurrentTime()
{
  QTE_D(vvVideoPlayer);

  vtkVgVideoModel0* videoModel =
    d->CurrentVideoPicked->GetVideoRepresentation()->GetVideoModel();
  vtkVgVideoProviderBase* videoSource = videoModel->GetVideoSource();

  vtkVgVideoMetadata metadata;
  const vtkVgVideoFrameData* frameData = videoModel->GetFrameData();

  double prevTime = d->CurrentTime;
  double currTime = frameData->TimeStamp.GetTime();

  // update time display if necessary
  if (!d->HasUpdated || currTime != prevTime)
    {
    d->HasUpdated = true;
    d->CurrentTime = currTime;

    d->UI.elapsedTime->setText(vgUnixTime(d->CurrentTime).timeString());

    // Update frame number
    if (videoSource &&
        videoSource->GetCurrentMetadata(&metadata) == VTK_OK)
      {
      const QString frameNumberStr
        = QString("( %1 )").arg(metadata.Time.GetFrameNumber());
      d->UI.frameNumber->setText(frameNumberStr);
      }

    // update the frame scrubber
    qtScopedValueChange<bool> x(d->UpdatingFrameScrubber, true);
    emit this->timeChanged(d->CurrentTime);
    }
}

//-----------------------------------------------------------------------------
vtkRenderWindowInteractor* vvVideoPlayer::interactor()
{
  QTE_D(vvVideoPlayer);
  return d->UI.renderWidget->GetInteractor();
}

//-----------------------------------------------------------------------------
vtkVgVideoRepresentationBase0* vvVideoPlayer::buildVideoRepresentation(
  vtkVgVideoNode& vtkNotUsed(videoNode))
{
  return 0;
}

//-----------------------------------------------------------------------------
void vvVideoPlayer::forceUpdateVideoRepresentation()
{
}
