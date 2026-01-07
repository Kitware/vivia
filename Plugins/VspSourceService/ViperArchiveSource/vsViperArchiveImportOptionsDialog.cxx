// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsViperArchiveImportOptionsDialog.h"
#include "ui_viperOptions.h"

#include <QFileInfo>
#include <QPushButton>
#include <QSettings>
#include <QUrl>

#include <qtUiState.h>
#include <qtUtil.h>

#include <vgFileDialog.h>

QTE_IMPLEMENT_D_FUNC(vsViperArchiveImportOptionsDialog)

//-----------------------------------------------------------------------------
class vsViperArchiveImportOptionsDialogPrivate
{
public:
  Ui::vsViperArchiveImportOptionsDialog UI;
  qtUiState UiState;
};

//-----------------------------------------------------------------------------
vsViperArchiveImportOptionsDialog::vsViperArchiveImportOptionsDialog(
  QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags),
    d_ptr(new vsViperArchiveImportOptionsDialogPrivate)
{
  QTE_D(vsViperArchiveImportOptionsDialog);
  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);

  connect(d->UI.videoBrowse, SIGNAL(clicked(bool)),
          this, SLOT(browseForVideo()));
  connect(d->UI.timeArchive, SIGNAL(toggled(bool)),
          this, SLOT(checkVideo()));
  connect(d->UI.videoFile, SIGNAL(textChanged(QString)),
          this, SLOT(checkVideo()));

  // Restore last-used options
  d->UiState.setCurrentGroup("ViPER");
  d->UiState.mapValue("FrameOffset", d->UI.frameOffset);
  d->UiState.mapValue("FrameRateIn", d->UI.frameRateIn);
  d->UiState.mapValue("FrameRateOut", d->UI.frameRateOut);
  d->UiState.mapChecked("ImportEvents", d->UI.importEvents);
  d->UiState.restore();
}

//-----------------------------------------------------------------------------
vsViperArchiveImportOptionsDialog::~vsViperArchiveImportOptionsDialog()
{
}

//-----------------------------------------------------------------------------
void vsViperArchiveImportOptionsDialog::accept()
{
  QTE_D_CONST(vsViperArchiveImportOptionsDialog);

  // Save options
  d->UiState.save();

  QDialog::accept();
}

//-----------------------------------------------------------------------------
QUrl vsViperArchiveImportOptionsDialog::metaDataSource() const
{
  QTE_D_CONST(vsViperArchiveImportOptionsDialog);
  return (d->UI.timeArchive->isChecked()
          ? QUrl::fromLocalFile(d->UI.videoFile->text())
          : QUrl());
}

//-----------------------------------------------------------------------------
int vsViperArchiveImportOptionsDialog::frameOffset() const
{
  QTE_D_CONST(vsViperArchiveImportOptionsDialog);
  return d->UI.frameOffset->value();
}

//-----------------------------------------------------------------------------
double vsViperArchiveImportOptionsDialog::frameRate() const
{
  QTE_D_CONST(vsViperArchiveImportOptionsDialog);
  return d->UI.frameRateOut->value() / d->UI.frameRateIn->value();
}

//-----------------------------------------------------------------------------
bool vsViperArchiveImportOptionsDialog::importEvents() const
{
  QTE_D_CONST(vsViperArchiveImportOptionsDialog);
  return d->UI.importEvents->isChecked();
}

//-----------------------------------------------------------------------------
void vsViperArchiveImportOptionsDialog::browseForVideo()
{
  QTE_D(vsViperArchiveImportOptionsDialog);

  QString fileName = vgFileDialog::getOpenFileName(
                       this, "Choose video archive...", QString(),
                       "KWA video archive (*.index);;"
                       "All files (*)");

  if (!fileName.isEmpty())
    {
    d->UI.videoFile->setText(fileName);
    }
}

//-----------------------------------------------------------------------------
void vsViperArchiveImportOptionsDialog::checkVideo()
{
  QTE_D(vsViperArchiveImportOptionsDialog);

  QPushButton* acceptButton =
    d->UI.buttonBox->button(QDialogButtonBox::Ok);

  if (d->UI.timeArchive->isChecked())
    {
    QFileInfo fi(d->UI.videoFile->text());
    acceptButton->setEnabled(fi.exists());
    }
  else
    {
    acceptButton->setEnabled(true);
    }
}
