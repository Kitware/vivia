/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpConfigureDialog_h
#define __vpConfigureDialog_h

#include <QDialog>
#include <QSet>

#include "vpSettings.h"

#include "ui_vpConfigure.h"

class vpViewCore;
class vgAttributeSet;

class vpConfigureDialog : public QDialog
{
  Q_OBJECT

public:
  // Enumerations for input fields.
  enum INPUT_KEY
    {
    HOSTNAME_KEY = 0,
    PORT_KEY,
    DATEBASE_KEY,
    DATABASE_DRIVER_KEY,
    USERNAME_KEY,
    PASSWORD_KEY,
    DATABASE_SETTINGS_KEY
    };

  vpConfigureDialog(QWidget* parent, vpViewCore* core, bool enableDataRequiredControls);
  virtual ~vpConfigureDialog();

  void setTrackAttributes(vgAttributeSet* attribs);
  void setColorWindowWidth(double width);
  void setColorWindowCenter(double center);

  Ui::vpConfigureDialog UI;

public slots:
  void accept();
  void reject();
  void apply();
  void reset();

  void setRequired(vpConfigureDialog::INPUT_KEY key);

protected slots:
  void databaseHostnameChanged(QString);
  void databaseServerPortChanged(QString);
  void databaseDriverChanged(QString);
  void databaseNameChanged(QString);
  void databaseUsernameChanged(QString);
  void databasePasswordChanged(QString);
  void databaseUseSourceImagesToggled(bool);

  void displayUseTimeStampDataToggled(bool);
  void displayForceFrameBasedVideoControlsToggled(bool);
  void displayAOIRelativeCoordinatesToggled(bool);
  void displayNoImageFilteringToggled(bool);
  void displayEnableWorldIfAvailableToggled(bool);
  void displayTranslateImageToggled(bool);

  void computeColorRange();

  void configureTrackAttributes();

  void uiUseZeroBasedFrameNumbersToggled(bool);
  void uiRightClickToEditToggled(bool);
  void uiAutoAdvanceFrameDuringCreationToggled(bool);

  void streamingUpdateIntervalChanged(int);
  void streamingTrackUpdateChunkSizeChanged(int);

  void videoSequentialPlaybackToggled(bool);
  void videoSuggestedFpsChanged(double);

protected:
  void addRequiredSettings(QWidget* widget, QWidget*& focusWidget);
  void setModified(QObject* object = 0, bool isValid = false);

  vpViewCore* Core;
  vpSettings* Settings;

  QSet<QObject*> RequiredSettings;
  QSet<QObject*> AcquiredSettings;

  vgAttributeSet* TrackAttributes;
};

#endif
