/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QUrl>

#include <qtStlUtil.h>
#include <qtUtil.h>

#include <vgAttributeSet.h>

// VpView includes.
#include "vpApplication.h"
#include "vpComputeColorRangeDialog.h"

#include "vpConfigureDialog.h"
#include "vpSettings.h"
#include "vpViewCore.h"

//-----------------------------------------------------------------------------
vpConfigureDialog::vpConfigureDialog(QWidget* parent, vpViewCore* core,
                                     bool enableDataRequiredControls) :
  QDialog(parent), TrackAttributes(0)
{
  this->Core = core;
  this->Settings = new vpSettings;
  this->UI.setupUi(this);
  this->UI.settingsPager->setCurrentIndex(0);
  this->UI.pageChooser->item(0)->setSelected(true);

  this->UI.colorWindowCompute->setEnabled(enableDataRequiredControls);

  qtUtil::setStandardIcons(this->UI.buttons);

  connect(this->UI.databaseHostname, SIGNAL(textChanged(QString)),
          this, SLOT(databaseHostnameChanged(QString)));
  connect(this->UI.databaseServerPort, SIGNAL(valueChanged(QString)),
          this, SLOT(databaseServerPortChanged(QString)));
  connect(this->UI.databaseDriverComboBox, SIGNAL(editTextChanged(QString)),
          this, SLOT(databaseDriverChanged(QString)));
  connect(this->UI.databaseNameComboBox, SIGNAL(editTextChanged(QString)),
          this, SLOT(databaseNameChanged(QString)));
  connect(this->UI.databaseUsername, SIGNAL(textChanged(QString)),
          this, SLOT(databaseUsernameChanged(QString)));
  connect(this->UI.databasePassword, SIGNAL(textChanged(QString)),
          this, SLOT(databasePasswordChanged(QString)));
  connect(this->UI.databaseUseSourceImages, SIGNAL(toggled(bool)),
          this, SLOT(databaseUseSourceImagesToggled(bool)));

  connect(this->UI.buttons->button(QDialogButtonBox::Apply),
          SIGNAL(pressed()), this, SLOT(apply()));
  connect(this->UI.buttons->button(QDialogButtonBox::Reset),
          SIGNAL(pressed()), this, SLOT(reset()));

  connect(this->UI.displayUseTimeStampData, SIGNAL(toggled(bool)),
          this, SLOT(displayUseTimeStampDataToggled(bool)));

  connect(this->UI.displayForceFrameBasedVideoControls, SIGNAL(toggled(bool)),
          this, SLOT(displayForceFrameBasedVideoControlsToggled(bool)));

  connect(this->UI.displayAOIRelativeCoords, SIGNAL(toggled(bool)),
          this, SLOT(displayAOIRelativeCoordinatesToggled(bool)));
  connect(this->UI.displayNoImageFiltering, SIGNAL(toggled(bool)),
          this, SLOT(displayNoImageFilteringToggled(bool)));

  connect(this->UI.colorWindowWidth, SIGNAL(valueChanged(double)),
          parent, SLOT(colorWindowWidthChanged(double)));
  connect(this->UI.colorWindowCenter, SIGNAL(valueChanged(double)),
          parent, SLOT(colorWindowCenterChanged(double)));
  connect(this->UI.colorWindowCompute, SIGNAL(clicked()),
          this, SLOT(computeColorRange()));

  connect(this->UI.displayEnableWorldIfAvailable, SIGNAL(toggled(bool)),
          this, SLOT(displayEnableWorldIfAvailableToggled(bool)));
  connect(this->UI.displayTranslateImage, SIGNAL(toggled(bool)),
          this, SLOT(displayTranslateImageToggled(bool)));

  connect(this->UI.trackAttributeConfig, SIGNAL(clicked()),
          this, SLOT(configureTrackAttributes()));

  connect(this->UI.uiUseZeroBasedFrameNumbers, SIGNAL(toggled(bool)),
          this, SLOT(uiUseZeroBasedFrameNumbersToggled(bool)));

  connect(this->UI.uiRightClickToEdit, SIGNAL(toggled(bool)),
          this, SLOT(uiRightClickToEditToggled(bool)));

  connect(this->UI.uiAutoAdvanceFrameDuringCreation, SIGNAL(toggled(bool)),
          this, SLOT(uiAutoAdvanceFrameDuringCreationToggled(bool)));

  connect(this->UI.streamingUpdateInterval, SIGNAL(valueChanged(int)),
          this, SLOT(streamingUpdateIntervalChanged(int)));
  connect(this->UI.streamingTrackUpdateChunkSize, SIGNAL(valueChanged(int)),
          this, SLOT(streamingTrackUpdateChunkSizeChanged(int)));

  connect(this->UI.videoSequentialPlayback, SIGNAL(toggled(bool)),
          this, SLOT(videoSequentialPlaybackToggled(bool)));
  connect(this->UI.videoSuggestedFps, SIGNAL(valueChanged(double)),
          this, SLOT(videoSuggestedFpsChanged(double)));

  this->reset();
}

//-----------------------------------------------------------------------------
vpConfigureDialog::~vpConfigureDialog()
{
  delete this->Settings; this->Settings = 0;
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::setTrackAttributes(vgAttributeSet* attribs)
{
  this->TrackAttributes = attribs;
  this->UI.trackAttributeConfig->setEnabled(attribs != 0);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::setColorWindowWidth(double width)
{
  this->UI.colorWindowWidth->setValue(width);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::setColorWindowCenter(double center)
{
  this->UI.colorWindowCenter->setValue(center);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::computeColorRange()
{
  auto dialog = new vpComputeColorRangeDialog(this, this->Core);
  if (dialog->exec() == QDialog::Accepted &&
      (dialog->getComputedWidth() != this->UI.colorWindowWidth->value() ||
       dialog->getComputedCenter() != this->UI.colorWindowCenter->value()))
    {
    // block signals checking for changed values, and thus two potential
    // renders, and force render only after updating both width and center
    this->UI.colorWindowWidth->blockSignals(true);
    this->setColorWindowWidth(dialog->getComputedWidth());
    this->Core->setColorWindowWidth(dialog->getComputedWidth(), false);
    this->UI.colorWindowWidth->blockSignals(false);

    this->UI.colorWindowCenter->blockSignals(true);
    this->setColorWindowCenter(dialog->getComputedCenter());
    this->Core->setColorWindowCenter(dialog->getComputedCenter(), true);
    this->UI.colorWindowCenter->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::accept()
{
  this->apply();
  QDialog::accept();
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::reject()
{
  if (this->Settings->wasCommitted() &&
      this->AcquiredSettings.contains(this->RequiredSettings))
    {
    QDialog::accept();
    }
  else
    {
    QDialog::reject();
    }
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::apply()
{
  this->Settings->commit();
  this->setModified();

  // Notify settings changed.
  vpApplication::instance()->notifySettingsChanged(vpSettings::DatabaseSettingsKey);

  // Reload settings in case they were changed externally during the apply
  delete this->Settings;
  this->Settings = new vpSettings;
  this->reset();
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::reset()
{
  this->Settings->discard();
  this->UI.databaseHostname->setText(
    this->Settings->databaseHostname().toString());

  this->UI.databaseServerPort->setValue(
    this->Settings->databaseServerPort().toInt());

  this->UI.databaseDriverComboBox->setEditText(
    this->Settings->databaseDriver());

  this->UI.databaseNameComboBox->setEditText(
    this->Settings->databaseName());

  this->UI.databaseUsername->setText(
    this->Settings->databaseUsername());

  this->UI.databasePassword->setText(
    vpSettings::databasePassword());

  this->UI.databaseUseSourceImages->setChecked(
    this->Settings->databaseUseSourceImages());

  this->UI.displayUseTimeStampData->setChecked(
    this->Settings->useTimeStampData());

  this->UI.displayForceFrameBasedVideoControls->setChecked(
    this->Settings->forceFrameBasedVideoControls());

  this->UI.displayAOIRelativeCoords->setChecked(
    this->Settings->coordinateDisplayMode() == vpSettings::AOIRelativeCoords);

  this->UI.displayImageRelativeCoords->setChecked(
    this->Settings->coordinateDisplayMode() == vpSettings::ImageRelativeCoords);

  this->UI.displayNoImageFiltering->setChecked(
    this->Settings->imageFilteringMode() == vpSettings::NoFiltering);

  this->UI.displayLinearImageFiltering->setChecked(
    this->Settings->imageFilteringMode() == vpSettings::LinearFiltering);

  this->UI.displayEnableWorldIfAvailable->setChecked(
    this->Settings->worldDisplayEnabled());

  this->UI.displayTranslateImage->setChecked(
    this->Settings->translateImageEnabled());

  this->UI.uiUseZeroBasedFrameNumbers->setChecked(
    this->Settings->useZeroBasedFrameNumbers());

  this->UI.uiRightClickToEdit->setChecked(
    this->Settings->rightClickToEdit());

  this->UI.uiAutoAdvanceFrameDuringCreation->setChecked(
    this->Settings->autoAdvanceDuringCreation());

  this->UI.streamingUpdateInterval->setValue(
    this->Settings->streamingUpdateInterval());

  this->UI.streamingTrackUpdateChunkSize->setValue(
    this->Settings->streamingTrackUpdateChunkSize());

  this->UI.videoSequentialPlayback->setChecked(
    this->Settings->videoPlaybackMode() == vpSettings::SequentialPlayback);

  this->UI.videoRealTimePlayback->setChecked(
    this->Settings->videoPlaybackMode() == vpSettings::RealTimePlayback);

  this->UI.videoSuggestedFps->setValue(this->Settings->videoSuggestedFps());

  this->setModified();
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::setRequired(vpConfigureDialog::INPUT_KEY key)
{
  QWidget* focusWidget = 0;
  switch (key)
    {
    case DATABASE_SETTINGS_KEY:
      this->addRequiredSettings(this->UI.databaseHostname, focusWidget);
      this->addRequiredSettings(this->UI.databaseServerPort, focusWidget);
      this->addRequiredSettings(this->UI.databaseNameComboBox, focusWidget);
      this->addRequiredSettings(this->UI.databaseUsername, focusWidget);
      this->addRequiredSettings(this->UI.databasePassword, focusWidget);

      if (focusWidget)
        {
        focusWidget->setFocus();
        }
      break;

    default:
      break;
    }

  // enable/disable okay button depending on required settings filled in
  this->setModified();
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::setModified(QObject* object, bool isValid)
{
  bool modified = this->Settings->hasUncommittedChanges();
  this->UI.buttons->button(QDialogButtonBox::Apply)->setEnabled(modified);
  if (object)
    {
    if (isValid)
      {
      this->AcquiredSettings.insert(object);
      }
    else
      {
      this->AcquiredSettings.remove(object);
      }
    }
  bool haveRequired = this->AcquiredSettings.contains(this->RequiredSettings);
  this->UI.buttons->button(QDialogButtonBox::Ok)->setEnabled(haveRequired);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::databaseHostnameChanged(QString hostname)
{
  this->Settings->setDatabaseHostname(hostname);
  this->setModified(this->UI.databaseHostname, !hostname.isEmpty());
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::databaseServerPortChanged(QString port)
{
  this->Settings->setDatabaseServerPort(port);
  this->setModified(this->UI.databaseServerPort, !port.isEmpty());
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::databaseDriverChanged(QString driver)
{
  this->Settings->setDatabaseDriver(driver);
  this->setModified(this->UI.databaseDriverComboBox, !driver.isEmpty());
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::databaseNameChanged(QString databaseName)
{
  this->Settings->setDatabaseName(databaseName);
  this->setModified(this->UI.databaseNameComboBox, !databaseName.isEmpty());
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::databaseUsernameChanged(QString username)
{
  this->Settings->setDatabaseUsername(username);
  this->setModified(this->UI.databaseUsername, !username.isEmpty());
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::databasePasswordChanged(QString password)
{
  vpSettings::setDatabasePassword(password);
  this->setModified(this->UI.databasePassword, !password.isEmpty());
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::databaseUseSourceImagesToggled(bool state)
{
  this->Settings->setDatabaseUseSourceImages(state);
  this->setModified(0, true);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::uiUseZeroBasedFrameNumbersToggled(bool state)
{
  this->Settings->setUseZeroBasedFrameNumbers(state);
  this->setModified(0, true);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::uiRightClickToEditToggled(bool state)
{
  this->Settings->setRightClickToEdit(state);
  this->setModified(0, true);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::uiAutoAdvanceFrameDuringCreationToggled(bool state)
{
  this->Settings->setAutoAdvanceDuringCreation(state);
  this->setModified(0, true);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::streamingUpdateIntervalChanged(int val)
{
  this->Settings->setStreamingUpdateInterval(val);
  this->setModified(0, true);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::streamingTrackUpdateChunkSizeChanged(int val)
{
  this->Settings->setStreamingTrackUpdateChunkSize(val);
  this->setModified(0, true);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::addRequiredSettings(QWidget* widget, QWidget*& focusWidget)
{
  this->RequiredSettings.insert(widget);

  bool isValid = true;
  QLineEdit* lineEdit(0);
  QComboBox* comboBox(0);

  if ((lineEdit = qobject_cast<QLineEdit*>(widget)))
    {
    isValid = !lineEdit->text().isEmpty();
    }
  else if ((comboBox = qobject_cast<QComboBox*>(widget)))
    {
    isValid = !comboBox->currentText().isEmpty();
    }

  if (!isValid)
    {
    focusWidget || (focusWidget = widget);
    }
  else
    {
    this->AcquiredSettings.insert(widget);
    }
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::displayUseTimeStampDataToggled(bool state)
{
  this->Settings->setUseTimeStampData(state);
  this->setModified(0, true);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::displayForceFrameBasedVideoControlsToggled(bool state)
{
  this->Settings->setForceFrameBasedVideoControls(state);
  this->setModified(0, true);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::displayAOIRelativeCoordinatesToggled(bool state)
{
  this->Settings->setCoordinateDisplayMode(
    state ? vpSettings::AOIRelativeCoords : vpSettings::ImageRelativeCoords);
  this->setModified(0, true);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::displayNoImageFilteringToggled(bool state)
{
  this->Settings->setImageFilteringMode(
    state ? vpSettings::NoFiltering : vpSettings::LinearFiltering);
  this->setModified(0, true);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::displayEnableWorldIfAvailableToggled(bool state)
{
  this->Settings->setWorldDisplayEnabled(state);
  this->setModified(0, true);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::displayTranslateImageToggled(bool state)
{
  this->Settings->setTranslateImageEnabled(state);
  this->setModified(0, true);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::configureTrackAttributes()
{
  QDialog dlg;
  dlg.setWindowTitle("Set Enabled Attributes");

  QVBoxLayout* layout = new QVBoxLayout;
  QListWidget* attrList = new QListWidget;
  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                     QDialogButtonBox::Cancel);
  layout->addWidget(attrList);
  layout->addWidget(buttonBox);
  dlg.setLayout(layout);

  connect(buttonBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), &dlg, SLOT(reject()));

  std::vector<std::string> groups = this->TrackAttributes->GetGroups();
  for (size_t i = 0, size = groups.size(); i < size; ++i)
    {
    QListWidgetItem* item = new QListWidgetItem(qtString(groups[i]));
    item->setCheckState(this->TrackAttributes->IsGroupEnabled(groups[i])
                          ? Qt::Checked : Qt::Unchecked);
    attrList->addItem(item);
    }

  if (dlg.exec() == QDialog::Accepted)
    {
    QSettings settings;
    settings.beginGroup("ShowTrackAttribute");

    for (int i = 0, count = attrList->count(); i < count; ++i)
      {
      QListWidgetItem* item = attrList->item(i);
      bool enabled = item->checkState() == Qt::Checked;
      this->TrackAttributes->SetGroupEnabled(stdString(item->text()), enabled);
      settings.setValue(item->text(), enabled);
      }
    }
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::videoSequentialPlaybackToggled(bool state)
{
  this->Settings->setVideoPlaybackMode(
    state ? vpSettings::SequentialPlayback : vpSettings::RealTimePlayback);
  this->setModified(0, true);
}

//-----------------------------------------------------------------------------
void vpConfigureDialog::videoSuggestedFpsChanged(double val)
{
  this->Settings->setVideoSuggestedFps(val);
  this->setModified(0, true);
}
