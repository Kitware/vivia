// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsMainWindowPrivate.h"

#include <QComboBox>
#include <QLabel>
#include <QToolTip>

#include <qtMath.h>
#include <qtPrioritizedMenuProxy.h>
#include <qtPrioritizedToolBarProxy.h>
#include <qtScopedValueChange.h>

#include "vsContourWidget.h"
#include "vsScene.h"

//-----------------------------------------------------------------------------
vsMainWindowPrivate::vsMainWindowPrivate(vsMainWindow* q)
  : QObject(q), q_ptr(q),
    Scene(0),
    TestUtility(0),
    IgnoreCancelDrawing(false),
    OldVideoPlaybackRate(1.0),
    OldVideoPlaybackLive(false),
    BlockFrameUpdates(false),
    NextSeekRequestId(0),
    LastScrubberSeek(-1),
    LastSpinBoxSeek(-1)
{
}

//-----------------------------------------------------------------------------
vsMainWindowPrivate::~vsMainWindowPrivate()
{
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::createStatusLabel(
  StatusSlot slot, Qt::Alignment alignment, bool permanent, int stretch)
{
  QLabel* label = new QLabel(this->UI.statusbar);
  label->setAlignment(alignment);

  (permanent
   ? this->UI.statusbar->addPermanentWidget(label, stretch)
   : this->UI.statusbar->addWidget(label, stretch));

  this->StatusLabels.insert(slot, label);
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::createSourceStatusLabels(
  StatusSlot iconSlot, StatusSlot textSlot)
{
  this->createStatusLabel(iconSlot, Qt::AlignCenter, true, 0);
  this->createStatusLabel(textSlot, Qt::AlignCenter, true, 1);
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::setupStatusBar()
{
  this->createSourceStatusLabels(vsMainWindowPrivate::StatusVideoIcon,
                                 vsMainWindowPrivate::StatusVideoText);
  this->createSourceStatusLabels(vsMainWindowPrivate::StatusTrackIcon,
                                 vsMainWindowPrivate::StatusTrackText);
  this->createSourceStatusLabels(vsMainWindowPrivate::StatusDescriptorIcon,
                                 vsMainWindowPrivate::StatusDescriptorText);

  this->createStatusLabel(vsMainWindowPrivate::StatusText,
                          Qt::AlignCenter, false, 4);
  this->createStatusLabel(vsMainWindowPrivate::StatusCursorLocation,
                          Qt::AlignCenter, false, 1);
  this->createStatusLabel(vsMainWindowPrivate::StatusFrameGsd,
                          Qt::AlignCenter, false, 1);
  this->createStatusLabel(vsMainWindowPrivate::StatusFrameTimestamp,
                          Qt::AlignLeft, false, 1);
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::setStatusVisible(StatusSlot slot, bool visible)
{
  if (this->StatusLabels.contains(slot))
    this->StatusLabels[slot]->setVisible(visible);
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::setStatusText(StatusSlot slot, const QString& text)
{
  if (this->StatusLabels.contains(slot))
    this->StatusLabels[slot]->setText(text);
}

//-----------------------------------------------------------------------------
QString vsMainWindowPrivate::iconFromStatus(
  const QString& baseIcon, vsDataSource::Status status)
{
  switch (status)
    {
    case vsDataSource::StreamingPending:
      return baseIcon + "-stream-pending";
    case vsDataSource::StreamingActive:
      return baseIcon + "-stream-active";
    case vsDataSource::StreamingIdle:
      return baseIcon + "-stream-idle";
    case vsDataSource::StreamingStopped:
      return baseIcon + "-stream-stopped";
    case vsDataSource::InProcessIdle:
      return baseIcon + "-live-idle";
    case vsDataSource::InProcessActive:
      return baseIcon + "-live-active";
    case vsDataSource::ArchivedActive:
      return baseIcon + "-archived-active";
    case vsDataSource::ArchivedSuspended:
      return baseIcon + "-archived-suspended";
    case vsDataSource::ArchivedIdle:
      return baseIcon + "-archived";
    case vsDataSource::MultipleSourcesStreaming:
      return baseIcon + "-multiple-sources-stream";
    case vsDataSource::MultipleSourcesInProcess:
      return baseIcon + "-multiple-sources-live";
    case vsDataSource::MultipleSourcesStreamingInProcess:
      return baseIcon + "-multiple-sources-stream-live";
    case vsDataSource::MultipleSourcesArchived:
      return baseIcon + "-multiple-sources-archived";
    case vsDataSource::MultipleSourcesStreamingArchived:
      return baseIcon + "-multiple-sources-stream-archived";
    case vsDataSource::MultipleSourcesInProcessArchived:
      return baseIcon + "-multiple-sources-live-archived";
    case vsDataSource::MultipleSourcesAll:
      return baseIcon + "-multiple-sources";
    default:
      break;
    }
  return baseIcon;
}

//-----------------------------------------------------------------------------
QString vsMainWindowPrivate::iconHtmlFromStatus(
  const QString& baseIcon, vsDataSource::Status status)
{
  if (status == vsDataSource::NoSource)
    return "&nbsp;";
  const QString icon = vsMainWindowPrivate::iconFromStatus(baseIcon, status);
  return QString("<img src='%1'/>").arg(icon);
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::setSourceStatus(
  StatusSlot iconSlot, StatusSlot textSlot,
  const QString& icon, QList<vsDataSource::Status> status,
  const QStringList& text, const QStringList& toolTip)
{
  QString baseIcon = ":/icons/source/16x16/" + icon;
  QString mergedToolTip;
  QString mergedText;
  vsDataSource::Status mergedStatus = vsDataSource::NoSource;

  QFontMetrics fm(QToolTip::font());
  int width = 32;

  // Generate merged status and tool tip
  for (int i = 0; i < status.size(); ++i)
    {
    if (mergedStatus == vsDataSource::NoSource)
      {
      mergedText = text[i];
      mergedStatus = status[i];
      }
    else
      {
      int si = ((static_cast<int>(mergedStatus) | static_cast<int>(status[i]))
                & vsDataSource::SourceTypeMask)
               | static_cast<int>(vsDataSource::MultipleSources);
      mergedText = "(multiple sources)";
      mergedStatus = static_cast<vsDataSource::Status>(si);
      }
    const QString iconHtml =
      vsMainWindowPrivate::iconHtmlFromStatus(baseIcon, status[i]);
    mergedToolTip +=
      "<tr><td>" + iconHtml + "</td><td>" + toolTip[i] + "</td></tr>";
    int tw = fm.width(toolTip[i]);
    width = qMax(width, qMin(400, tw + 32));
    }
  mergedToolTip =
    QString("<table width=%1>").arg(width) + mergedToolTip + "</table>";

  // Set status icon, text, and tool tips
  QLabel* iconLabel = this->StatusLabels.value(iconSlot, 0);
  QLabel* textLabel = this->StatusLabels.value(textSlot, 0);
  if (iconLabel)
    {
    if (mergedStatus == vsDataSource::NoSource)
      {
      QPixmap icon = QIcon(baseIcon).pixmap(16, 16, QIcon::Disabled);
      iconLabel->setPixmap(icon);
      }
    else
      {
      const QString icon =
        vsMainWindowPrivate::iconFromStatus(baseIcon, mergedStatus);
      iconLabel->setPixmap(QPixmap(icon));
      }
    iconLabel->setToolTip(mergedToolTip);
    }
  if (textLabel)
    {
    textLabel->setText(mergedText);
    textLabel->setToolTip(mergedToolTip);
    }
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::updateFrameSpinBox()
{
  this->BlockFrameUpdates = false;
  vgTimeStamp ts = this->UI.scrubber->value();
  if (ts.HasFrameNumber())
    {
    qtScopedBlockSignals bs(this->UI.frame);
    this->UI.frame->setValue(static_cast<int>(ts.FrameNumber));
    }
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::enableInputs(
  vsDescriptorInput::Types acceptedInputs)
{
  bool enableAlerts = acceptedInputs.testFlag(vsDescriptorInput::Query);

  this->UI.actionAlertCreate->setEnabled(enableAlerts);
  this->UI.actionAlertLoad->setEnabled(enableAlerts);
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::enableRegionClose()
{
  this->UI.actionRegionClose->setEnabled(true);
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::disableRegionClose()
{
  this->UI.actionRegionClose->setEnabled(false);
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::completeDrawing()
{
  if (QSettings().value("PersistentDrawing", false).toBool())
    {
    // Keep drawing until told to stop
    this->Scene->beginDrawing(this->currentDrawingType());
    }
  else
    {
    // End drawing mode
    this->UI.actionRegionDraw->setChecked(false);
    }
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::cancelDrawing()
{
  if (!this->IgnoreCancelDrawing)
    {
    this->UI.actionRegionDraw->setChecked(false);
    }
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::updateDrawingType()
{
  this->Scene->setDrawingType(this->currentDrawingType());
  this->UI.actionRegionClose->setEnabled(
    this->Scene->contourState() == vsContourWidget::Begin);
}

//-----------------------------------------------------------------------------
vsContour::Type vsMainWindowPrivate::currentDrawingType() const
{
  const QVariant typeData =
    this->RegionType->itemData(this->RegionType->currentIndex());
  return typeData.value<vsContour::Type>();
}

//-----------------------------------------------------------------------------
int vsMainWindowPrivate::resampleMode(QAction* activeMode) const
{
  if (activeMode == this->UI.actionVideoSampleBicubic)
    {
    return 2;
    }
  else if (activeMode == this->UI.actionVideoSampleLinear)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//-----------------------------------------------------------------------------
QAction* vsMainWindowPrivate::resampleAction(int resampleMode)
{
  switch (resampleMode)
    {
    default:
    case 0:
      return this->UI.actionVideoSampleNearest;
    case 1:
      return this->UI.actionVideoSampleLinear;
    case 2:
      return this->UI.actionVideoSampleBicubic;
    }
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::setVideoPlaybackSlow(qreal direction)
{
  const qreal rate = fabs(this->VideoPlaybackRate);
  const bool sameDirection = (qtSign(this->VideoPlaybackRate) == direction);

  QSettings settings;
  if (sameDirection && this->VideoPlaybackMode != vgVideoPlayer::Live &&
      settings.value("PlaybackPlaySlows", false).toBool())
    {
    const qreal min = settings.value("PlaybackMinSpeed", 8).toReal();
    (rate > (1.0 / (min - 0.5)) && rate < 1.1
     ? this->VideoPlaybackRate *= 0.5
     : this->VideoPlaybackRate = direction);
    }
  else
    {
    this->VideoPlaybackRate = direction;
    }

  this->Scene->setVideoPlaybackSpeed(vgVideoPlayer::Playing,
                                     this->VideoPlaybackRate);
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::setVideoPlaybackFast(qreal direction)
{
  const qreal rate = fabs(this->VideoPlaybackRate);
  const bool sameDirection = (qtSign(this->VideoPlaybackRate) == direction);

  QSettings settings;
  if (rate > 1.1 && sameDirection)
    {
    const qreal max = settings.value("PlaybackMaxSpeed", 32).toReal();
    (rate > 1.1 && rate < (max - 0.5)
     ? this->VideoPlaybackRate *= 2.0
     : this->VideoPlaybackRate = direction * 2.0);
    }
  else
    {
    this->VideoPlaybackRate =
      direction * settings.value("PlaybackInitialFastSpeed", 2).toReal();
    }

  this->UI.actionVideoPlay->setChecked(true);
  this->Scene->setVideoPlaybackSpeed(vgVideoPlayer::Playing,
                                     this->VideoPlaybackRate);
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::setVideoPlaybackSpeed(qreal rate)
{
  if (this->VideoPlaybackMode != vgVideoPlayer::Playing &&
      this->VideoPlaybackMode != vgVideoPlayer::Buffering)
    {
    return;
    }

  QSettings settings;
  const qreal min = settings.value("PlaybackMinSpeed", 8).toReal();
  const qreal max = settings.value("PlaybackMaxSpeed", 32).toReal();
  const qreal direction = qtSign(rate);

  this->VideoPlaybackRate = direction * qBound(1.0 / min, fabs(rate), max);

  this->UI.actionVideoPlay->setChecked(true);
  this->Scene->setVideoPlaybackSpeed(vgVideoPlayer::Playing,
                                     this->VideoPlaybackRate);
}

//-----------------------------------------------------------------------------
void vsMainWindowPrivate::updatePlaybackControlsState(
  vgTimeStamp currentVideoTime)
{
  vgVideoPlayer::PlaybackMode mode = this->VideoPlaybackMode;
  bool playing =
    (mode == vgVideoPlayer::Playing ||
     mode == vgVideoPlayer::Buffering ||
     mode == vgVideoPlayer::Live);
  bool paused = (mode == vgVideoPlayer::Paused);

  bool enableForward = (currentVideoTime < this->AvailableVideoRange.upper);
  bool enableBackward = (currentVideoTime > this->AvailableVideoRange.lower);

  this->UI.actionVideoFrameForward->setEnabled(!playing && enableForward);
  this->UI.actionVideoFrameBackward->setEnabled(!playing && enableBackward);
  this->UI.actionVideoSkipForward->setEnabled(!playing && enableForward);
  this->UI.actionVideoSkipBackward->setEnabled(!playing && enableBackward);

  this->UI.actionVideoFastBackward->setEnabled(playing);
  this->UI.actionVideoFastForward->setEnabled(playing);
  this->UI.actionVideoResume->setEnabled(paused);

  if (playing && mode != vgVideoPlayer::Live)
    {
    QSettings settings;
    const qreal min = settings.value("PlaybackMinSpeed", 8).toReal();
    const qreal max = settings.value("PlaybackMaxSpeed", 32).toReal();
    const qreal rate = fabs(this->VideoPlaybackRate);
    this->UI.actionVideoSpeedDecrease->setEnabled(
      rate > (1.0 / (min - 0.5)));
    this->UI.actionVideoSpeedIncrease->setEnabled(
      rate < (max - 0.5));
    }
  else
    {
    this->UI.actionVideoSpeedDecrease->setEnabled(false);
    this->UI.actionVideoSpeedIncrease->setEnabled(false);
    }

}
