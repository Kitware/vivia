/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#define QTSETTINGS_SUBCLASS_NAME vpSettings
#include <qtSettingsImpl.h>

#include "vpSettings.h"

const char keyDatabaseHostname[]         = "DatabaseHostname";
const char keyDatabaseServerPort[]       = "DatabaseServerPort";
const char keyDatabaseDriver[]           = "DatabaseDriver";
const char keyDatabaseName[]             = "DatabaseName";
const char keyDatabaseUsername[]         = "DatabaseUsername";
const char keyDatabaseUseSourceImages[]  = "DatabaseUseSourceImages";

const char keyCoordinateDisplayMode_[]   = "CoordinateDisplayMode";
const char keyImageFilteringMode_[]      = "ImageFilteringMode";

const char keyTrackTrailLengthFrames[]   = "TrackTrailLengthFrames";
const char keyTrackTrailLengthSeconds[]  = "TrackTrailLengthSeconds";

const char keyUseTimeStampData[]         = "UseTimeStampData";
const char keyForceFrameBasedVideoControls[] = "ForceFrameBasedVideoControls";
const char keyWorldDisplayEnabled[]      = "WorldDisplayEnabled";
const char keyTranslateImageEnabled[]    = "TranslateImageEnabled";

const char keyUseZeroBasedFrameNumbers[] = "UseZeroBasedFrameNumbers";
const char keyRightClickToEdit[]         = "RightClickToEdit";
const char keyAutoAdvanceDuringCreation[] = "AutoAdvanceDuringCreation";
const char keyInterpolateToGround[]      = "InterpolateToGround";

const char keySceneElementLineWidth[]    = "SceneElementLineWidth";

const char keyStreamingUpdateInterval[]  = "StreamingUpdateInterval";
const char keyStreamingTrackUpdateChunkSize[] = "StreamingTrackUpdateChunkSize";

const char keyVideoPlaybackMode_[]       = "VideoPlaybackMode";
const char keyVideoSuggestedFps[]        = "VideoSuggestedFps";

const char keyExternalProcessProgram[]   = "ExternalProcessProgram";
const char keyExternalProcessArgs[]      = "ExternalProcessArgs";
const char keyExternalProcessIOPath[]    = "ExternalProcessIOPath";
const char keyExternalProcessThreshold[]   = "ExternalProcessThreshold";
const char keyExternalProcessThresholdState[]      = "ExternalProcessThresholdState";
const char keyExternalProcessCellSize[]   = "ExternalProcessCellSize";
const char keyExternalProcessCellSizeState[]      = "ExternalProcessCellSizeState";

QString vpSettings::Password = "";

//-----------------------------------------------------------------------------
vpSettings::vpSettings()
{
  this->declareSetting(keyDatabaseHostname);
  this->declareSetting(keyDatabaseServerPort, QVariant("5432"));
  this->declareSetting(keyDatabaseDriver);
  this->declareSetting(keyDatabaseName);
  this->declareSetting(keyDatabaseUsername);
  this->declareSetting(keyDatabaseUseSourceImages, QVariant(false));

  this->declareSetting(keyCoordinateDisplayMode_);
  this->declareSetting(keyImageFilteringMode_);

  this->declareSetting(keyTrackTrailLengthFrames);
  this->declareSetting(keyTrackTrailLengthSeconds);

  this->declareSetting(keyUseTimeStampData, QVariant(true));
  this->declareSetting(keyForceFrameBasedVideoControls, QVariant(false));
  this->declareSetting(keyWorldDisplayEnabled, QVariant(true));
  this->declareSetting(keyTranslateImageEnabled, QVariant(true));

  this->declareSetting(keyUseZeroBasedFrameNumbers, QVariant(false));
  this->declareSetting(keyRightClickToEdit, QVariant(true));
  this->declareSetting(keyAutoAdvanceDuringCreation, QVariant(true));
  this->declareSetting(keyInterpolateToGround, QVariant(false));

  this->declareSetting(keySceneElementLineWidth, QVariant(4));

  this->declareSetting(keyStreamingUpdateInterval, QVariant(5));
  this->declareSetting(keyStreamingTrackUpdateChunkSize, QVariant(10));

  this->declareSetting(keyVideoPlaybackMode_);
  this->declareSetting(keyVideoSuggestedFps, QVariant(2.0));

  this->declareSetting(keyExternalProcessProgram);
  this->declareSetting(keyExternalProcessArgs);
  this->declareSetting(keyExternalProcessIOPath);
  this->declareSetting(keyExternalProcessThreshold);
  this->declareSetting(keyExternalProcessThresholdState);
  this->declareSetting(keyExternalProcessCellSize);
  this->declareSetting(keyExternalProcessCellSizeState);
}

//-----------------------------------------------------------------------------
void vpSettings::setDatabasePassword(const QString& password)
{
  Password = password;
}

//-----------------------------------------------------------------------------
QString vpSettings::databasePassword()
{
  return Password;
}

//-----------------------------------------------------------------------------
void vpSettings::setCoordinateDisplayMode(CoordDisplayMode mode)
{
  this->setCoordinateDisplayMode_(this->coordinateDisplayModeString(mode));
}

//-----------------------------------------------------------------------------
vpSettings::CoordDisplayMode vpSettings::coordinateDisplayMode()
{
  QString str = this->coordinateDisplayMode_();

  if (str == coordinateDisplayModeString(ImageRelativeCoords))
    {
    return ImageRelativeCoords;
    }

  return AOIRelativeCoords;
}

//-----------------------------------------------------------------------------
void vpSettings::setImageFilteringMode(ImageFilteringMode mode)
{
  this->setImageFilteringMode_(this->imageFilteringModeString(mode));
}

//-----------------------------------------------------------------------------
vpSettings::ImageFilteringMode vpSettings::imageFilteringMode()
{
  QString str = this->imageFilteringMode_();

  if (str == imageFilteringModeString(LinearFiltering))
    {
    return LinearFiltering;
    }

  return NoFiltering;
}

//-----------------------------------------------------------------------------
void vpSettings::setVideoPlaybackMode(VideoPlaybackMode mode)
{
  this->setVideoPlaybackMode_(this->videoPlaybackModeString(mode));
}

//-----------------------------------------------------------------------------
vpSettings::VideoPlaybackMode vpSettings::videoPlaybackMode()
{
  QString str = this->videoPlaybackMode_();

  if (str == videoPlaybackModeString(SequentialPlayback))
    {
    return SequentialPlayback;
    }

  return RealTimePlayback;
}

//-----------------------------------------------------------------------------
const char* vpSettings::coordinateDisplayModeString(CoordDisplayMode mode)
{
  switch (mode)
    {
    case AOIRelativeCoords:   return "AOI";
    case ImageRelativeCoords: return "Image";
    }

  return 0;
}

//-----------------------------------------------------------------------------
const char* vpSettings::imageFilteringModeString(ImageFilteringMode mode)
{
  switch (mode)
    {
    case NoFiltering:     return "None";
    case LinearFiltering: return "Linear";
    }

  return 0;
}

//-----------------------------------------------------------------------------
const char* vpSettings::videoPlaybackModeString(VideoPlaybackMode mode)
{
  switch (mode)
    {
    case SequentialPlayback: return "Sequential";
    case RealTimePlayback:   return "RealTime";
    }

  return 0;
}

//-----------------------------------------------------------------------------
qtSettings_implement(QUrl,    databaseHostname,          DatabaseHostname)
qtSettings_implement(QString, databaseServerPort,        DatabaseServerPort)
qtSettings_implement(QString, databaseDriver,            DatabaseDriver)
qtSettings_implement(QString, databaseName,              DatabaseName)
qtSettings_implement(QString, databaseUsername,          DatabaseUsername)
qtSettings_implement(bool,    databaseUseSourceImages,   DatabaseUseSourceImages)
qtSettings_implement(QString, coordinateDisplayMode_,    CoordinateDisplayMode_)
qtSettings_implement(QString, imageFilteringMode_,       ImageFilteringMode_)
qtSettings_implement(int,     trackTrailLengthFrames,    TrackTrailLengthFrames)
qtSettings_implement(double,  trackTrailLengthSeconds,   TrackTrailLengthSeconds)
qtSettings_implement(bool,    useTimeStampData,          UseTimeStampData)
qtSettings_implement(bool,    forceFrameBasedVideoControls, ForceFrameBasedVideoControls)
qtSettings_implement(bool,    worldDisplayEnabled,       WorldDisplayEnabled)
qtSettings_implement(bool,    translateImageEnabled,     TranslateImageEnabled)
qtSettings_implement(bool,    useZeroBasedFrameNumbers,  UseZeroBasedFrameNumbers)
qtSettings_implement(int,     streamingUpdateInterval,   StreamingUpdateInterval)
qtSettings_implement(int,     streamingTrackUpdateChunkSize, StreamingTrackUpdateChunkSize)
qtSettings_implement(bool,    rightClickToEdit,          RightClickToEdit)
qtSettings_implement(bool,    autoAdvanceDuringCreation, AutoAdvanceDuringCreation)
qtSettings_implement(bool,    interpolateToGround,       InterpolateToGround)
qtSettings_implement(double,  sceneElementLineWidth,     SceneElementLineWidth)
qtSettings_implement(QString, videoPlaybackMode_,        VideoPlaybackMode_)
qtSettings_implement(double,  videoSuggestedFps,         VideoSuggestedFps)

qtSettings_implement(QString,     externalProcessProgram,         ExternalProcessProgram)
qtSettings_implement(QStringList, externalProcessArgs,            ExternalProcessArgs)
qtSettings_implement(QString,     externalProcessIOPath,          ExternalProcessIOPath)
qtSettings_implement(double,      externalProcessThreshold,       ExternalProcessThreshold)
qtSettings_implement(bool,        externalProcessThresholdState,  ExternalProcessThresholdState)
qtSettings_implement(double,      externalProcessCellSize,        ExternalProcessCellSize)
qtSettings_implement(bool,        externalProcessCellSizeState,   ExternalProcessCellSizeState)

//END vpSettings
