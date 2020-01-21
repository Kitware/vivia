/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "moc_vgVideoPlayerPrivate.cpp"

#include <QtCore>

#include <qtDebug.h>
#include <qtDebugHelper.h>
#include <qtOnce.h>

#include <vgCheckArg.h>

#include "vgVideoAnimation.h"
#include "vgVideoPlayerRequestor.h"
#include "vgVideoSource.h"
#include "vgVideoPlayerBufferedRequestor.h"

QTE_IMPLEMENT_D_FUNC(vgVideoPlayer)

//BEGIN QDebug helpers

//-----------------------------------------------------------------------------
QDEBUG_ENUM_HANDLER_BEGIN(vgVideoPlayer::PlaybackMode)
QDEBUG_HANDLE_ENUM(vgVideoPlayer::Stopped)
QDEBUG_HANDLE_ENUM(vgVideoPlayer::Paused)
QDEBUG_HANDLE_ENUM(vgVideoPlayer::Playing)
QDEBUG_HANDLE_ENUM(vgVideoPlayer::Buffering)
QDEBUG_HANDLE_ENUM(vgVideoPlayer::Live)
QDEBUG_ENUM_HANDLER_END(vgVideoPlayer)

//-----------------------------------------------------------------------------
QDEBUG_ENUM_HANDLER_BEGIN(QAbstractAnimation::State)
QDEBUG_HANDLE_ENUM(QAbstractAnimation::Stopped)
QDEBUG_HANDLE_ENUM(QAbstractAnimation::Paused)
QDEBUG_HANDLE_ENUM(QAbstractAnimation::Running)
QDEBUG_ENUM_HANDLER_END(QAbstractAnimation)

//-----------------------------------------------------------------------------
QDEBUG_ENUM_HANDLER_BEGIN(vg::SeekMode)
QDEBUG_HANDLE_ENUM(vg::SeekExact)
QDEBUG_HANDLE_ENUM(vg::SeekNearest)
QDEBUG_HANDLE_ENUM(vg::SeekLowerBound)
QDEBUG_HANDLE_ENUM(vg::SeekUpperBound)
QDEBUG_HANDLE_ENUM(vg::SeekUnspecified)
QDEBUG_ENUM_HANDLER_END(vg::SeekMode)

//-----------------------------------------------------------------------------
QDebug& operator<<(QDebug& dbg, const vgVideoSeekRequest& r)
{
  dbg.nospace()
    << "(time = "
    << (r.TimeStamp.HasTime()
        ? qPrintable(QString::number(r.TimeStamp.GetTime(), 'f', 2))
        : "<invalid>")
    << ", frame number = "
    << (r.TimeStamp.HasFrameNumber()
        ? qPrintable(QString::number(r.TimeStamp.GetFrameNumber()))
        : "<invalid>")
    << ", direction = " << r.Direction
    << ", requestor = " << r.Requestor
    << ", request id = " << r.RequestId
    << ')';
  return dbg.space();
}

//END QDebug helpers

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgVideoPlayerStatic

namespace vgVideoPlayerStatic
{

QTE_ONCE(once);

//-----------------------------------------------------------------------------
void registerMetaTypes()
{
  QTE_REGISTER_METATYPE(vgVideoSource*);
  QTE_REGISTER_METATYPE(vgVideoPlayer::PlaybackMode);
  QTE_REGISTER_METATYPE(vgVtkVideoFramePtr);
  QTE_REGISTER_METATYPE(vtkVgTimeStamp);
  QTE_REGISTER_METATYPE(vg::SeekMode);
  QTE_REGISTER_METATYPE(qtDebugAreaAccessor);
}

} // namespace vgVideoPlayerStatic

//END vgVideoPlayerStatic

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgVideoPlayerPrivate::SuppressStateUpdates

//-----------------------------------------------------------------------------
class vgVideoPlayerPrivate::SuppressStateUpdates
{
public:
  SuppressStateUpdates(vgVideoPlayerPrivate* q);

  ~SuppressStateUpdates();

protected:
  QTE_DECLARE_PUBLIC_PTR(vgVideoPlayerPrivate)

  bool UpdateSuppressed;

private:
  QTE_DECLARE_PUBLIC(vgVideoPlayerPrivate)
};

//-----------------------------------------------------------------------------
vgVideoPlayerPrivate::SuppressStateUpdates::SuppressStateUpdates(
  vgVideoPlayerPrivate* q) :
  q_ptr(q),
  UpdateSuppressed(false)
{
  q->StateUpdateSuppressor = this;
}

//-----------------------------------------------------------------------------
vgVideoPlayerPrivate::SuppressStateUpdates::~SuppressStateUpdates()
{
  QTE_Q(vgVideoPlayerPrivate);

  if (this->UpdateSuppressed)
    {
    // If an update was suppressed, push it to the public vgVideoPlayer now
    q->updatePublicPlaybackMode();
    }

  q->StateUpdateSuppressor = 0;
}

//END vgVideoPlayerPrivate::SuppressStateUpdates

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgVideoPlayerPrivate

//-----------------------------------------------------------------------------
vgVideoPlayerPrivate::vgVideoPlayerPrivate(vgVideoPlayer* q) :
  q_ptr(q),
  Animation(0),
  StateUpdateSuppressor(0),
  DebugArea(qtDebug::InvalidArea),
  AutoPlay(true),
  AutoPause(false),
  Streaming(false),
  LivePlaybackOffset(0.0)
{
  this->setSource(0);

  this->PlaybackMode = vgVideoPlayer::Stopped;
  this->PlaybackRate = 1.0;
  this->debugPlayback("set initial mode");
}

//-----------------------------------------------------------------------------
vgVideoPlayerPrivate::~vgVideoPlayerPrivate()
{
}

//-----------------------------------------------------------------------------
void vgVideoPlayerPrivate::debugPlayback(const char* message)
{
  qtDebug(this->DebugArea)
    << message << this->PlaybackMode << this->PlaybackRate;
}

//-----------------------------------------------------------------------------
void vgVideoPlayerPrivate::setDebugArea(qtDebugAreaAccessor area)
{
  this->DebugArea = area;
}

//-----------------------------------------------------------------------------
void vgVideoPlayerPrivate::run()
{
  qtDebug(this->DebugArea) << "playback thread executing";

  // Start up the animation framework
  this->Animation = new vgVideoAnimation(this);
  connect(this->Animation, SIGNAL(stateChanged(QAbstractAnimation::State,
                                               QAbstractAnimation::State)),
          this, SLOT(updateAnimationStatus(QAbstractAnimation::State)));
  qtThread::run();

  qtDebug(this->DebugArea) << "playback thread halting";

  // Make sure animation is stopped (or Qt is unhappy when it is destroyed)
  this->Animation->setState(QAbstractAnimation::Stopped);

  // Detach and release our requestor
  this->Requestor->release();

  // Done
  qtDebug(this->DebugArea) << "playback thread done";
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vgVideoPlayerPrivate::findTime(
  unsigned int frameNumber, vg::SeekMode direction)
{
  CHECK_ARG(this->VideoSource, vtkVgTimeStamp());
  return this->VideoSource->findTime(frameNumber, direction);
}

//-----------------------------------------------------------------------------
void vgVideoPlayerPrivate::requestFrame(vgVideoSeekRequest request)
{
  this->Requestor->requestFrame(this->VideoSource, request);
}

//-----------------------------------------------------------------------------
void vgVideoPlayerPrivate::setSource(vgVideoSource* source)
{
  // Reset the source and available frame range
  this->VideoSource = source;
  this->FirstFrameTimestamp.Reset();
  this->LastFrameTimestamp.Reset();

  // Reset the animation (but test existence; it won't exist yet when we are
  // called during construction)
  if (this->Animation)
    {
    this->Animation->setState(QAbstractAnimation::Stopped);
    this->Animation->setFrameRange(vtkVgTimeStamp(), vtkVgTimeStamp());
    }
}

//-----------------------------------------------------------------------------
void vgVideoPlayerPrivate::setStreaming(bool streaming)
{
  this->Streaming = streaming;

  if (!streaming)
    {
    if (this->PlaybackMode == vgVideoPlayer::Buffering)
      {
      // If we were waiting on more data, there won't be any; change to stopped
      this->setPlaybackSpeed(vgVideoPlayer::Stopped, this->PlaybackRate);
      }
    else if (this->PlaybackMode == vgVideoPlayer::Live)
      {
      // If we were in live playback mode, switch to regular playback (which
      // may switch immediately to stopped if we have no more video)
      this->setPlaybackSpeed(vgVideoPlayer::Playing, this->PlaybackRate);
      }
    }
}

//-----------------------------------------------------------------------------
void vgVideoPlayerPrivate::setFrameRange(
  vtkVgTimeStamp first, vtkVgTimeStamp last)
{
  vtkVgTimeStamp oldBufferEnd = this->LastFrameTimestamp;

  // Update internal state
  this->FirstFrameTimestamp = first;
  this->LastFrameTimestamp = last;

  // Update animation
  this->Animation->setFrameRange(first, last);

  if (this->PlaybackMode == vgVideoPlayer::Live)
    {
    const double t = last.GetTime() - this->LivePlaybackOffset;
    this->Animation->seek(vgTimeStamp::fromTime(t), vg::SeekUpperBound);
    }
  // Else if we were waiting on more data, resume playback
  else if (this->AutoPlay && (!oldBufferEnd.IsValid() || oldBufferEnd < last) &&
           this->PlaybackMode == vgVideoPlayer::Buffering)
    {
    // Temporarily suppress playback status updates until we have resolved our
    // final resulting state
    SuppressStateUpdates su(this);

    this->Animation->seek(oldBufferEnd);
    this->setPlaybackSpeed(vgVideoPlayer::Playing, this->PlaybackRate);
    }
  // If we are paused at the previous end of the video, the actual animation
  // state is stopped, so we need to transition back to 'actual' paused
  else if (this->AutoPause && this->PlaybackMode == vgVideoPlayer::Paused)
    {
    // Temporarily suppress playback status updates until we have resolved our
    // final resulting state
    SuppressStateUpdates su(this);

    this->setPlaybackSpeed(vgVideoPlayer::Paused, this->PlaybackRate);
    this->Animation->seek(oldBufferEnd);
    }
}

//-----------------------------------------------------------------------------
void vgVideoPlayerPrivate::setPlaybackSpeed(
  vgVideoPlayer::PlaybackMode mode, qreal rate)
{
  qtDebug(this->DebugArea) << "requesting mode" << mode << rate;

  switch (mode)
    {
    case vgVideoPlayer::Playing:
      this->AutoPlay = true;
      this->Animation->setRate(rate);
      this->Requestor->setPlaybackRate(rate);
      if (this->PlaybackMode == vgVideoPlayer::Paused)
        {
        if (this->AutoPause)
          {
          // If the current state is reported as Paused, and auto-pause is set,
          // then the animation is actually stopped; we don't need to make any
          // request of the animation, but need to transition our internal
          // state to Buffering and set auto-play
          this->AutoPlay = true;
          this->AutoPause = false;
          mode = vgVideoPlayer::Buffering;
          }
        else
          {
          this->Animation->setState(QAbstractAnimation::Running);
          }
        }
      else if (this->PlaybackMode == vgVideoPlayer::Live)
        {
        // If we're in Live, things get a little interesting; if we're at the
        // head (animation state == Stopped) and we're asked to play forward,
        // we need to switch immediately to Buffering to prevent the animation
        // from resetting to the beginning, otherwise we can start the
        // animation running normally
        if (rate > 0.0 && this->Animation->state() == QAbstractAnimation::Stopped)
          {
          this->AutoPlay = true;
          this->AutoPause = false;
          mode = vgVideoPlayer::Buffering;
          }
        else
          {
          this->Animation->setState(QAbstractAnimation::Running);
          }
        }
      else if (this->PlaybackMode != vgVideoPlayer::Buffering)
        {
        this->Animation->setState(QAbstractAnimation::Running);
        }
      else
        {
        // If we were previously in Buffering, we need to stay in Buffering,
        // because we aren't really playing just because the speed changed
        // (note that a transition out of Buffering goes through Paused due to
        // the animation seek, so we can know that that isn't happening here)
        mode = vgVideoPlayer::Buffering;
        }
      this->updatePlaybackMode(mode, rate);
      break;
    case vgVideoPlayer::Live:
      this->AutoPause = false;
      this->updatePlaybackMode(mode, rate);
      this->Animation->setState(QAbstractAnimation::Paused);
      // Set offset to itself as a mechanism for seeking the playback position
      // to the right initial spot for live playback
      this->setLivePlaybackOffset(this->LivePlaybackOffset);
    case vgVideoPlayer::Paused:
      this->AutoPause = false;
      this->Requestor->setPlaybackRate(0.0);
      if (this->PlaybackMode == vgVideoPlayer::Playing)
        {
        this->Animation->setState(QAbstractAnimation::Paused);
        }
      else if (this->PlaybackMode == vgVideoPlayer::Buffering)
        {
        this->AutoPlay = false;
        this->AutoPause = true;
        this->PlaybackMode = mode;
        this->updatePublicPlaybackMode();
        }
      else if (this->PlaybackMode == vgVideoPlayer::Live)
        {
        this->AutoPause =
          (this->Animation->state() == QAbstractAnimation::Stopped);
        this->updatePlaybackMode(mode, rate);
        }
      break;
    case vgVideoPlayer::Stopped:
      this->AutoPause = false;
      this->Requestor->setPlaybackRate(0.0);
      if (this->PlaybackMode != vgVideoPlayer::Stopped)
        {
        // Stop playing, reset to the beginning, and transition the animation
        // to stopped
        this->AutoPlay = false;
        this->Animation->setState(QAbstractAnimation::Paused);
        this->Animation->seek(vgTimeStamp::fromTime(-qInf()));
        this->Animation->setState(QAbstractAnimation::Stopped);
        if (this->PlaybackMode == vgVideoPlayer::Live)
          {
          // In live playback mode, the animation may already be stopped, and
          // so we won't get our mode transition that way, so instead force it
          // here and now (and not via updatePlaybackMode, which tries to guess
          // if we want to switch to Buffering, which we don't because we were
          // told to be Stopped)
          this->PlaybackMode = vgVideoPlayer::Stopped;
          this->updatePublicPlaybackMode();
          }

        // Clear auto-pause (again), as it will have been affected by our
        // changes to the animation's time and playback state
        this->AutoPause = false;
        }
      break;
    default:
      // May not set playback speed to Buffering
      qDebug() << "warning: setting playback mode to" << mode
               << "is not permitted";
      break;
    }
}

//-----------------------------------------------------------------------------
void vgVideoPlayerPrivate::updatePlaybackMode(
  vgVideoPlayer::PlaybackMode mode, qreal rate)
{
  if (this->Streaming)
    {
    if (mode == vgVideoPlayer::Stopped)
      {
      qtDebug(this->DebugArea)
        << "requested" << mode << "but may adjust due to streaming"
        << "( auto-play" << this->AutoPlay
        << "/ auto-pause" << this->AutoPause << ')';

      if (this->AutoPlay && (this->Animation->currentTime() > 1e-4))
        {
        // If we are playing, and the animation stops (because it is at the
        // end), switch to Buffering mode
        mode = vgVideoPlayer::Buffering;
        }
      else if (this->AutoPause)
        {
        // If we are paused, and the animation stops (because a seek caused us
        // to be at the end), switch to Paused mode
        mode = vgVideoPlayer::Paused;
        }
      }
    else if (mode == vgVideoPlayer::Playing)
      {
      if (this->Animation->duration() < 1e-4)
        {
        qtDebug(this->DebugArea)
          << "request" << mode << "being adjusted due to tiny duration";
        mode = vgVideoPlayer::Buffering;
        }
      }
    }

  // Update internal mode
  this->PlaybackMode = mode;
  this->PlaybackRate = rate;
  this->debugPlayback("update mode");

  if (this->StateUpdateSuppressor)
    {
    // If updates are being blocked, update our internal state (necessary for
    // transition from Buffering to Playing to work correctly), but hold off
    // updating public class (which will also suppress emission of the state
    // changed signal)
    this->StateUpdateSuppressor->UpdateSuppressed = true;
    }
  else
    {
    // Update public class (so its getters have the current values)
    this->updatePublicPlaybackMode();
    }
}

//-----------------------------------------------------------------------------
void vgVideoPlayerPrivate::updatePublicPlaybackMode()
{
  QTE_Q(vgVideoPlayer);
  QMetaObject::invokeMethod(
    q, "updatePlaybackMode",
    Q_ARG(vgVideoPlayer::PlaybackMode, this->PlaybackMode),
    Q_ARG(qreal, this->PlaybackRate));
}

//-----------------------------------------------------------------------------
void vgVideoPlayerPrivate::seekToTimestamp(
  qint64 requestId, vtkVgTimeStamp ts, vg::SeekMode direction)
{
  // Temporarily suppress playback status updates until we have resolved our
  // final resulting state
  SuppressStateUpdates su(this);

  // Before we do anything, check if we are Buffering; if so, we want to be
  // Playing after the seek, but the animation is actually stopped (and so the
  // seek is going to leave us in Paused)
  bool autoPlay =
    this->PlaybackMode == vgVideoPlayer::Buffering && this->AutoPlay;

  vgVideoSeekRequest request;
  request.RequestId = requestId;
  request.TimeStamp = ts;
  request.Direction = direction;

  qtDebug(this->DebugArea) << "requesting seek" << request;

  this->Animation->seek(request);

  if (autoPlay)
    {
    // Start playing the animation, if we determined that we need to do so
    // (see above comment)
    this->Animation->setState(QAbstractAnimation::Running);
    }
}

//-----------------------------------------------------------------------------
void vgVideoPlayerPrivate::setLivePlaybackOffset(double offset)
{
  this->LivePlaybackOffset = offset;

  if (this->PlaybackMode == vgVideoPlayer::Live)
    {
    const double t = this->LastFrameTimestamp.GetTime() - offset;
    this->Animation->seek(vgTimeStamp::fromTime(t), vg::SeekUpperBound);
    }
}

//-----------------------------------------------------------------------------
void vgVideoPlayerPrivate::updateAnimationStatus(QAbstractAnimation::State s)
{
  qtDebug(this->DebugArea) << "animation status changed to" << s;

  switch (s)
    {
    case QAbstractAnimation::Running:
      if (this->PlaybackMode != vgVideoPlayer::Live)
        {
        this->updatePlaybackMode(vgVideoPlayer::Playing, this->PlaybackRate);
        }
      break;
    case QAbstractAnimation::Paused:
      if (this->PlaybackMode != vgVideoPlayer::Live)
        {
        // When transitioning to Paused, clear auto-pause, or else we can get
        // wrongly stuck in Buffering when the user asks to play (even on
        // non-streaming video!)
        this->AutoPause = false;
        this->updatePlaybackMode(vgVideoPlayer::Paused, this->PlaybackRate);
        }
      break;
    default:
      if (this->PlaybackMode == vgVideoPlayer::Paused)
        {
        // If the animation transitions to Stopped when our previous state was
        // Paused, it is because a: we were asked to stop, or b: a seek moved
        // us to the end of the animation; set auto-play/auto-pause accordingly
        // so we don't think we are in Buffering mode and start playing the
        // animation again when new video arrives (in case (a), we will clear
        // auto-pause after vgVideoAnimation::setState returns)
        this->AutoPlay = false;
        this->AutoPause = true;
        }
      else if (this->PlaybackMode != vgVideoPlayer::Live)
        {
        this->updatePlaybackMode(vgVideoPlayer::Stopped, this->PlaybackRate);
        }
      break;
    }
}

//END vgVideoPlayerPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgVideoPlayer

//-----------------------------------------------------------------------------
vgVideoPlayer::vgVideoPlayer(QObject* parent) :
  QObject(parent),
  d_ptr(new vgVideoPlayerPrivate(this))
{
  QTE_D(vgVideoPlayer);

  // Register our metatypes
  using namespace vgVideoPlayerStatic;
  qtOnce(once, &registerMetaTypes);

  d->ParentDebugArea = qtDebug::InvalidArea;
  d->ParentPlaybackMode = vgVideoPlayer::Stopped;
  d->ParentPlaybackRate = 1.0;
  d->ParentLivePlaybackOffset = 0.0;

  // Get video buffering settings
  QSettings settings;
  settings.beginGroup("VideoPlayback");
  const double bufferTime = settings.value("BufferTime", 5.0).toDouble();
  const double bufferFrames = settings.value("BufferFrames", 50).toInt();
  const qreal bufferRateCap = settings.value("BufferRateLimit", 2.2).toReal();

  // If buffering is enabled (both size limits > 0)...
  if (bufferFrames > 0 && bufferTime > 0)
    {
    // ...then create a buffered requestor...
    vgVideoPlayerBufferedRequestor* requestor =
      vgVideoPlayerBufferedRequestor::create();
    requestor->setBufferLimit(bufferTime, bufferFrames, bufferRateCap);
    d->Requestor = requestor;
    }
  else
    {
    // ...otherwise create a plain requestor
    d->Requestor = vgVideoPlayerRequestor::create();
    }

  connect(d->Requestor, SIGNAL(frameAvailable(vgVtkVideoFramePtr, qint64)),
          this, SLOT(emitFrameAvailable(vgVtkVideoFramePtr, qint64)));
  connect(d->Requestor, SIGNAL(seekRequestDiscarded(qint64, vtkVgTimeStamp)),
          this, SIGNAL(seekRequestDiscarded(qint64, vtkVgTimeStamp)));
  d->Requestor->moveToThread(d->thread());

  d->start();
}

//-----------------------------------------------------------------------------
vgVideoPlayer::~vgVideoPlayer()
{
  QTE_D(vgVideoPlayer);
  qtDebug(d->ParentDebugArea) << "stopping playback thread";
  d->quit();
  d->wait();
}

//-----------------------------------------------------------------------------
void vgVideoPlayer::setDebugArea(qtDebugAreaAccessor area)
{
  QTE_D(vgVideoPlayer);
  d->ParentDebugArea = area;
  QMetaObject::invokeMethod(d, "setDebugArea",
                            Q_ARG(qtDebugAreaAccessor, area));
}

//-----------------------------------------------------------------------------
void vgVideoPlayer::setVideoSource(vgVideoSource* source)
{
  QTE_D(vgVideoPlayer);
  QMetaObject::invokeMethod(d, "setSource", Qt::BlockingQueuedConnection,
                            Q_ARG(vgVideoSource*, source));
}

//-----------------------------------------------------------------------------
vgVideoPlayer::PlaybackMode vgVideoPlayer::playbackMode() const
{
  QTE_D_CONST(vgVideoPlayer);
  return d->ParentPlaybackMode;
}

//-----------------------------------------------------------------------------
qreal vgVideoPlayer::playbackSpeed() const
{
  QTE_D_CONST(vgVideoPlayer);
  return d->ParentPlaybackRate;
}

//-----------------------------------------------------------------------------
double vgVideoPlayer::livePlaybackOffset() const
{
  QTE_D_CONST(vgVideoPlayer);
  return d->ParentLivePlaybackOffset;
}

//-----------------------------------------------------------------------------
void vgVideoPlayer::updatePlaybackMode(
  vgVideoPlayer::PlaybackMode mode, qreal rate)
{
  QTE_D(vgVideoPlayer);

  d->ParentPlaybackMode = mode;
  d->ParentPlaybackRate = rate;

  emit this->playbackSpeedChanged(mode, rate);
}

//-----------------------------------------------------------------------------
void vgVideoPlayer::setSourceStreaming(bool streaming)
{
  QTE_D(vgVideoPlayer);
  QMetaObject::invokeMethod(d, "setStreaming", Q_ARG(bool, streaming));
}

//-----------------------------------------------------------------------------
void vgVideoPlayer::setSourceFrameRange(
  vtkVgTimeStamp first, vtkVgTimeStamp last)
{
  QTE_D(vgVideoPlayer);
  QMetaObject::invokeMethod(d, "setFrameRange",
                            Q_ARG(vtkVgTimeStamp, first),
                            Q_ARG(vtkVgTimeStamp, last));
}


//-----------------------------------------------------------------------------
void vgVideoPlayer::setPlaybackSpeed(
  vgVideoPlayer::PlaybackMode mode, qreal rate)
{
  QTE_D(vgVideoPlayer);
  QMetaObject::invokeMethod(d, "setPlaybackSpeed",
                            Qt::BlockingQueuedConnection,
                            Q_ARG(vgVideoPlayer::PlaybackMode, mode),
                            Q_ARG(qreal, rate));
}

//-----------------------------------------------------------------------------
void vgVideoPlayer::seekToTimestamp(
  qint64 requestId, vtkVgTimeStamp ts, vg::SeekMode direction)
{
  CHECK_ARG(ts.IsValid());

  QTE_D(vgVideoPlayer);
  QMetaObject::invokeMethod(d, "seekToTimestamp", Qt::BlockingQueuedConnection,
                            Q_ARG(qint64, requestId),
                            Q_ARG(vtkVgTimeStamp, ts),
                            Q_ARG(vg::SeekMode, direction));
}

//-----------------------------------------------------------------------------
void vgVideoPlayer::setLivePlaybackOffset(double offset)
{
  // Figure out effective offset, adjusting for bad input and converting from
  // a possibly negative 'additive offset' to a magnitude, which is how we want
  // to use it internally
  offset = (qIsFinite(offset) ? qAbs(offset) : 0.0);

  QTE_D(vgVideoPlayer);
  QMetaObject::invokeMethod(d, "setLivePlaybackOffset", Q_ARG(double, offset));
  d->ParentLivePlaybackOffset = offset;
}

//-----------------------------------------------------------------------------
void vgVideoPlayer::emitFrameAvailable(
  vgVtkVideoFramePtr frame, qint64 seekRequestId)
{
  emit this->frameAvailable(*frame, seekRequestId);
}

//END vgVideoPlayer
