/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpSettings_h
#define __vpSettings_h

#include <QUrl>
#include <QStringList>

#include <qtSettings.h>

class vpSettings : public qtSettings
{
public:

  enum SettingsKey
    {
    DatabaseSettingsKey = 0x1,
    AllSettingsKey      = 0xffff
    };

  enum CoordDisplayMode
    {
    AOIRelativeCoords,
    ImageRelativeCoords
    };

  enum ImageFilteringMode
    {
    NoFiltering,
    LinearFiltering
    };

  enum VideoPlaybackMode
    {
    SequentialPlayback,
    RealTimePlayback
    };

  Q_DECLARE_FLAGS(SettingsKeys, SettingsKey)

  vpSettings();

  qtSettings_declare(QUrl,    databaseHostname,    setDatabaseHostname);
  qtSettings_declare(QString, databaseServerPort,  setDatabaseServerPort);
  qtSettings_declare(QString, databaseDriver,      setDatabaseDriver);
  qtSettings_declare(QString, databaseName,        setDatabaseName);
  qtSettings_declare(QString, databaseUsername,    setDatabaseUsername);
  qtSettings_declare(bool, databaseUseSourceImages, setDatabaseUseSourceImages);

  qtSettings_declare(QString,
                     coordinateDisplayMode_,
                     setCoordinateDisplayMode_);

  qtSettings_declare(QString,
                     imageFilteringMode_,
                     setImageFilteringMode_);

  qtSettings_declare(int, trackTrailLengthFrames, setTrackTrailLengthFrames);
  qtSettings_declare(double, trackTrailLengthSeconds, setTrackTrailLengthSeconds);

  qtSettings_declare(bool, useTimeStampData, setUseTimeStampData);
  qtSettings_declare(bool, forceFrameBasedVideoControls,
                           setForceFrameBasedVideoControls);
  qtSettings_declare(bool, worldDisplayEnabled, setWorldDisplayEnabled);
  qtSettings_declare(bool, translateImageEnabled, setTranslateImageEnabled);

  qtSettings_declare(bool, useZeroBasedFrameNumbers, setUseZeroBasedFrameNumbers);
  qtSettings_declare(bool, rightClickToEdit, setRightClickToEdit);
  qtSettings_declare(bool, autoAdvanceDuringCreation, setAutoAdvanceDuringCreation);
  qtSettings_declare(bool, interpolateToGround, setInterpolateToGround);
  qtSettings_declare(double, sceneElementLineWidth, setSceneElementLineWidth);

  qtSettings_declare(int, streamingUpdateInterval, setStreamingUpdateInterval);
  qtSettings_declare(int, streamingTrackUpdateChunkSize, setStreamingTrackUpdateChunkSize);

  qtSettings_declare(QString,
                     videoPlaybackMode_,
                     setVideoPlaybackMode_);

  qtSettings_declare(double, videoSuggestedFps, setVideoSuggestedFps);

  qtSettings_declare(QString, externalProcessProgram, setExternalProcessProgram);
  qtSettings_declare(QStringList, externalProcessArgs, setExternalProcessArgs);
  qtSettings_declare(QString, externalProcessIOPath, setExternalProcessIOPath);
  qtSettings_declare(double, externalProcessThreshold, setExternalProcessThreshold);
  qtSettings_declare(bool, externalProcessThresholdState, setExternalProcessThresholdState);
  qtSettings_declare(double, externalProcessCellSize, setExternalProcessCellSize);
  qtSettings_declare(bool, externalProcessCellSizeState, setExternalProcessCellSizeState);

  static void    setDatabasePassword(const QString& password);
  static QString databasePassword();

  void setCoordinateDisplayMode(CoordDisplayMode mode);
  CoordDisplayMode coordinateDisplayMode();

  void setImageFilteringMode(ImageFilteringMode mode);
  ImageFilteringMode imageFilteringMode();

  void setVideoPlaybackMode(VideoPlaybackMode mode);
  VideoPlaybackMode videoPlaybackMode();

private:
  const char* coordinateDisplayModeString(CoordDisplayMode mode);
  const char* imageFilteringModeString(ImageFilteringMode mode);
  const char* videoPlaybackModeString(VideoPlaybackMode mode);

  static QString Password;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(vpSettings::SettingsKeys)

#endif
