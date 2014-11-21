/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvGenerateReportDialog.h"

#include "ui_generateReport.h"

#include <QDir>
#include <QMessageBox>
#include <QSettings>

#include <qtUiState.h>
#include <qtUtil.h>

#include <vgFileDialog.h>

QTE_IMPLEMENT_D_FUNC(vvGenerateReportDialog)

//-----------------------------------------------------------------------------
class vvGenerateReportDialogPrivate
{
public:
  Ui::GenerateReportDialog UI;
  qtUiState UiState;
};

//-----------------------------------------------------------------------------
vvGenerateReportDialog::vvGenerateReportDialog(QWidget* parent,
                                               Qt::WindowFlags f)
  : QDialog(parent, f), d_ptr(new vvGenerateReportDialogPrivate)
{
  QTE_D(vvGenerateReportDialog);

  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);

  connect(d->UI.pathBrowse, SIGNAL(clicked()),
          this, SLOT(browseForOutputPath()));

  d->UiState.setCurrentGroup("ReportGeneration");
  d->UiState.mapChecked("GenerateVideo", d->UI.generateVideo);
  d->UiState.mapText("OutputPath", d->UI.path);
  d->UiState.restore();

  // Force absolute path
  QDir dir(d->UI.path->text());
  d->UI.path->setText(dir.absolutePath());
}

//-----------------------------------------------------------------------------
vvGenerateReportDialog::~vvGenerateReportDialog()
{
}

//-----------------------------------------------------------------------------
bool vvGenerateReportDialog::generateVideo()
{
  QTE_D(vvGenerateReportDialog);

  return d->UI.generateVideo->isChecked();
}

//-----------------------------------------------------------------------------
QString vvGenerateReportDialog::outputPath()
{
  QTE_D(vvGenerateReportDialog);

  return d->UI.path->text();
}

//-----------------------------------------------------------------------------
void vvGenerateReportDialog::browseForOutputPath()
{
  QTE_D(vvGenerateReportDialog);

  QString path =
    vgFileDialog::getExistingDirectory(this, "Output Path", d->UI.path->text());

  if (!path.isEmpty())
    {
    d->UI.path->setText(path);
    }
}

//-----------------------------------------------------------------------------
void vvGenerateReportDialog::accept()
{
  QTE_D(vvGenerateReportDialog);

  QDir dir(d->UI.path->text());
  if (!dir.isAbsolute())
    {
    QMessageBox::warning(this, QString(),
                         "The output path is not a valid absolute path.");
    return;
    }
  if (!dir.exists())
    {
    dir.makeAbsolute();
    d->UI.path->setText(dir.path());
    if (QMessageBox::warning(this, QString(),
                             "The requested output directory does not exist. "
                             "Do you wish to create it?",
                             QMessageBox::Ok | QMessageBox::Cancel) ==
        QMessageBox::Ok)
      {
      if (!dir.mkpath(dir.path()))
        {
        QMessageBox::warning(this, QString(), "Unable to create directory.");
        return;
        }
      }
    else
      {
      return;
      }
    }

  d->UiState.save();
  QDialog::accept();
}
