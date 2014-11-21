/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqVideoPlayer.h"

#include <QIcon>

#include <qtUtil.h>

#include <vgCheckArg.h>

#include <vtkVgVideoNode.h>
#include <vtkVgVideoRepresentation0.h>

#include <vvMakeId.h>

#include <vvClipVideoRepresentation.h>
#include <vvVideoPlayerModel.h>
#include <vvVideoPlayerPrivate.h>

// viqui includes
#include "vqArchiveVideoSource.h"

//-----------------------------------------------------------------------------
class vqVideoPlayerPrivate : public vvVideoPlayerPrivate
{
public:
  QAction* actionFormulateQuery;
  QAction* actionOpenInExternal;
};

QTE_IMPLEMENT_D_FUNC(vqVideoPlayer)

//-----------------------------------------------------------------------------
vqVideoPlayer::vqVideoPlayer(QWidget* parent)
  : vvVideoPlayer(new vqVideoPlayerPrivate, parent)
{
  QTE_D(vqVideoPlayer);

  d->actionFormulateQuery =
    new QAction(qtUtil::standardActionIcon("new-query"),
                "Formulate Query from Result...", this);
  d->actionOpenInExternal =
    new QAction(qtUtil::standardActionIcon("open-video-external"),
                "Open in External Player", this);
  d->actionOpenInExternal->setToolTip(
    "Open result video in external video player application");

  d->UI.toolBar->addSeparator();
  d->UI.toolBar->addAction(d->actionFormulateQuery);
  d->UI.toolBar->addAction(d->actionOpenInExternal);

  connect(d->actionFormulateQuery, SIGNAL(triggered()),
          this, SLOT(formulateQuery()));
  connect(d->actionOpenInExternal, SIGNAL(triggered()),
          this, SLOT(openExternally()));
}

//-----------------------------------------------------------------------------
vqVideoPlayer::~vqVideoPlayer()
{
}

//-----------------------------------------------------------------------------
vtkVgVideoRepresentationBase0* vqVideoPlayer::buildVideoRepresentation(
  vtkVgVideoNode& videoNode)
{
  vvClipVideoRepresentation* videoRepresentation =
    vvClipVideoRepresentation::New();
  vvVideoPlayerModel* videoModel =
    vvVideoPlayerModel::New();

  // Enable the auto center mode.
  videoRepresentation->SetAutoCenter(1);
  connect(videoRepresentation, SIGNAL(areaOfInterest(double*, double, double)),
          this, SLOT(onAreaOfInterest(double*, double, double)));

  vqArchiveVideoSource* videoSource  = vqArchiveVideoSource::SafeDownCast(
                                         videoNode.GetVideoRepresentation()->GetVideoModel()->GetVideoSource());
  videoModel->SetVideoSource(videoSource);
  connect(videoSource, SIGNAL(frameAvailable()),
          videoModel, SLOT(OnFrameAvailable()));
  connect(videoModel, SIGNAL(FrameAvailable()),
          this, SLOT(onFrameAvailable()));
  videoModel->SetUseInternalTimeStamp(1);
  videoModel->SetLooping(1);
  videoModel->SetSharingSource(1);
  videoModel->SetId(videoNode.GetInstanceId());

  videoRepresentation->SetVideoModel(videoModel);
  videoModel->FastDelete();
  return videoRepresentation;
}

//-----------------------------------------------------------------------------
void vqVideoPlayer::forceUpdateVideoRepresentation()
{
  if (!this->d_ptr->CurrentVideo)
    {
    return;
    }

  vtkVgVideoModel0* model =
    this->d_ptr->CurrentVideo->GetVideoRepresentation()->GetVideoModel();

  vvVideoPlayerModel::SafeDownCast(model)->OnFrameAvailable();
}

//-----------------------------------------------------------------------------
void vqVideoPlayer::setAnalysisToolsEnabled(bool enable)
{
  QTE_D(vqVideoPlayer);
  d->actionFormulateQuery->setEnabled(enable);
}

//-----------------------------------------------------------------------------
void vqVideoPlayer::formulateQuery()
{
  // No need to anything if we don't have a video selected
  CHECK_ARG(this->currentPickedVideoModel());

  QTE_D(vqVideoPlayer);

  double timeRange[2];
  this->currentPickedVideoModel()->GetVideoSource()->GetTimeRange(timeRange);
  const double start = timeRange[0];
  const double end = timeRange[1];
  const double currentTime = d->CurrentTime;

  // Construct the formulation query
  vvProcessingRequest request;
  request.QueryId = vvMakeId("VIQUI-QF");
  request.StartTime = start;
  request.EndTime = end;
  request.VideoUri = std::string(d->CurrentVideoPicked->GetStreamId());

  emit this->queryFormulationRequested(request, qRound64(currentTime));
}

//-----------------------------------------------------------------------------
void vqVideoPlayer::openExternally()
{
  // No need to anything if we don't have a video selected
  CHECK_ARG(this->currentPickedVideoModel());
  vqArchiveVideoSource* videoSource  = vqArchiveVideoSource::SafeDownCast(
                                         this->currentPickedVideoModel()->GetVideoSource());
  CHECK_ARG(videoSource);

  QTE_D(vqVideoPlayer);

  // Get clip URI and video source URI
  const QUrl clipUri = videoSource->GetClipUri();
  const QString streamId =
    QString::fromLocal8Bit(d->CurrentVideoPicked->GetStreamId());

  // Ask for external player to be launched
  emit this->externalOpenRequested(clipUri, streamId, d->CurrentTime);
}
