// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvGeneratePowerPointDialog.h"

#include "ui_generatePowerPoint.h"

#include <QDir>
#include <QMessageBox>
#include <QSettings>

#include <qtUiState.h>
#include <qtUtil.h>

#include <vgFileDialog.h>

QTE_IMPLEMENT_D_FUNC(vvGeneratePowerPointDialog)

//-----------------------------------------------------------------------------
class vvGeneratePowerPointDialogPrivate
{
public:
  Ui::GeneratePowerPointDialog UI;
  qtUiState UiState;
};

//-----------------------------------------------------------------------------
vvGeneratePowerPointDialog::vvGeneratePowerPointDialog(
  QWidget* parent, Qt::WindowFlags f) :
  QDialog(parent, f),
  d_ptr(new vvGeneratePowerPointDialogPrivate)
{
  QTE_D(vvGeneratePowerPointDialog);

  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);

  connect(d->UI.pathBrowse, SIGNAL(clicked()),
          this, SLOT(browseForOutputPath()));
  connect(d->UI.templateBrowse, SIGNAL(clicked()),
          this, SLOT(browseForTemplateFile()));

  d->UiState.setCurrentGroup("PowerPointGeneration");
  d->UiState.mapText("OutputPath", d->UI.path);
  d->UiState.mapText("TemplateFile", d->UI.templateFile);
  d->UiState.mapChecked("GenerateVideo", d->UI.generateVideo);
  d->UiState.restore();
}

//-----------------------------------------------------------------------------
vvGeneratePowerPointDialog::~vvGeneratePowerPointDialog()
{
}

//-----------------------------------------------------------------------------
bool vvGeneratePowerPointDialog::getLastConfig()
{
  QTE_D(vvGeneratePowerPointDialog);

  // Check if configuration is valid
  if (d->UI.path->text().isEmpty() || d->UI.templateFile->text().isEmpty() ||
      !QDir(d->UI.path->text()).exists() ||
      !QFileInfo(d->UI.path->text()).exists())
    {
    // No; user must configure us now
    return (this->exec() == QDialog::Accepted);
    }

  // We have been previously configured; nothing needs to be done
  return true;
}

//-----------------------------------------------------------------------------
bool vvGeneratePowerPointDialog::generateVideo() const
{
  QTE_D_CONST(vvGeneratePowerPointDialog);
  return d->UI.generateVideo->isChecked();
}

//-----------------------------------------------------------------------------
QString vvGeneratePowerPointDialog::outputPath() const
{
  QTE_D_CONST(vvGeneratePowerPointDialog);
  return d->UI.path->text();
}

//-----------------------------------------------------------------------------
QString vvGeneratePowerPointDialog::templateFile() const
{
  QTE_D_CONST(vvGeneratePowerPointDialog);
  return d->UI.templateFile->text();
}

//-----------------------------------------------------------------------------
void vvGeneratePowerPointDialog::browseForOutputPath()
{
  QTE_D(vvGeneratePowerPointDialog);

  QString path =
    vgFileDialog::getExistingDirectory(this, "Output Path", d->UI.path->text());

  if (!path.isEmpty())
    {
    d->UI.path->setText(path);
    }
}

//-----------------------------------------------------------------------------
void vvGeneratePowerPointDialog::browseForTemplateFile()
{
  QTE_D(vvGeneratePowerPointDialog);

  const QString templateFile =
    vgFileDialog::getOpenFileName(this, "Template File",
                                  d->UI.templateFile->text());

  if (!templateFile.isEmpty())
    {
    d->UI.templateFile->setText(templateFile);
    }
}

//-----------------------------------------------------------------------------
void vvGeneratePowerPointDialog::accept()
{
  QTE_D(vvGeneratePowerPointDialog);

  const QDir dir(d->UI.path->text());
  if (!dir.isAbsolute())
    {
    // Only absolute output paths are accepted
    QMessageBox::warning(this, QString(),
                         "The output path is not a valid absolute path.");
    return;
    }

  // Check for whitespace in the output path; the Office interop SaveAs fn
  // fails if there is (whitespace)
  if (d->UI.path->text().contains(QRegExp("\\s")))
    {
    QMessageBox::warning(this, QString(),
                         "The output path may not contain whitespace.");
    return;
    }

  // Try to make directory if it doesn't exist, and fail if unable to do so
  if (!dir.exists() && !dir.mkpath(dir.path()))
    {
    QMessageBox::warning(this, QString(), "Unable to create output directory.");
    return;
    }

  // Check that template file exists
  const QFileInfo templateFile(d->UI.templateFile->text());
  if (!templateFile.exists())
    {
    QMessageBox::warning(this, QString(),
                         "Specified template file does not exist.");
    return;
    }

  // Ensure that we remember template file as an absolute path
  if (!templateFile.isAbsolute())
    {
    d->UI.templateFile->setText(templateFile.absoluteFilePath());
    }

  d->UiState.save();
  QDialog::accept();
}
