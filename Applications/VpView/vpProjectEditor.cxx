/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpProjectEditor.h"
#include "ui_vpProjectEditor.h"

#include "vpProject.h"

#include <vgFileDialog.h>

#include <qtConfirmationDialog.h>

#include <QDir>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryFile>

//-----------------------------------------------------------------------------
class vpProjectEditorPrivate
{
public:
  Ui_vpProjectEditor UI;

  QString ProjectPath;
  QScopedPointer<QTemporaryFile> TemporaryProject;
};

QTE_IMPLEMENT_D_FUNC(vpProjectEditor)

//-----------------------------------------------------------------------------
vpProjectEditor::vpProjectEditor(QWidget* parent, Qt::WindowFlags flags) :
  QDialog{parent, flags}, d_ptr{new vpProjectEditorPrivate}
{
  QTE_D();

  d->UI.setupUi(this);

  auto* const okayButton = d->UI.buttons->button(QDialogButtonBox::Ok);
  auto* const saveButton = d->UI.buttons->button(QDialogButtonBox::Save);

  // Remove and re-add button; this breaks the button's association as a
  // "standard button" and prevents the QDialogButtonBox from resetting the
  // button's text
  d->UI.buttons->removeButton(saveButton);
  saveButton->setText("&Save As...");
  d->UI.buttons->addButton(saveButton, QDialogButtonBox::AcceptRole);

  connect(okayButton, SIGNAL(clicked()), this, SLOT(acceptProject()));
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveProject()));

  auto* const pickDatasetMenu = new QMenu{this};
  pickDatasetMenu->addAction(d->UI.actionDatasetPickFile);
  pickDatasetMenu->addAction(d->UI.actionDatasetPickDirectory);
  d->UI.pickDataset->setMenu(pickDatasetMenu);

  connect(d->UI.actionDatasetPickFile, SIGNAL(triggered()),
          this, SLOT(browseForDatasetFile()));
  connect(d->UI.actionDatasetPickDirectory, SIGNAL(triggered()),
          this, SLOT(browseForDatasetDirectory()));
  connect(d->UI.pickTracks, SIGNAL(clicked()),
          this, SLOT(browseForTracksFile()));
}

//-----------------------------------------------------------------------------
vpProjectEditor::~vpProjectEditor()
{
}

//-----------------------------------------------------------------------------
void vpProjectEditor::setDataset(const QString& path)
{
  QTE_D();
  d->UI.dataset->setText(path);
}

//-----------------------------------------------------------------------------
QString vpProjectEditor::projectPath() const
{
  QTE_D();
  return d->ProjectPath;
}

//-----------------------------------------------------------------------------
void vpProjectEditor::acceptProject()
{
  QTE_D();

  // Check if project has been saved
  if (d->ProjectPath.isEmpty())
  {
    if (!qtConfirmationDialog::getConfirmation(
      this, "acceptTemporaryProject",
      "Your project has not been saved. If you continue, "
      "your project will be lost when you exit the application.",
      "Are you sure?", "&Continue"))
    {
      return;
    }

    d->TemporaryProject.reset(new QTemporaryFile);
    if (!d->TemporaryProject->open())
    {
      QMessageBox::critical(
        this, "Error",
        "The temporary project file could not be created. "
        "Please save the project or try again. "
        "(" + d->TemporaryProject->errorString() + ")");
      d->TemporaryProject.reset();
      return;
    }
    d->ProjectPath = d->TemporaryProject->fileName();
  }

  // Write project file
  QSettings project{d->ProjectPath, QSettings::IniFormat};
  project.clear();
  project.setValue(vpProject::DataSetSpecifierTag, d->UI.dataset->text());
  project.setValue(vpProject::TracksFileTag, d->UI.tracks->text());
  project.sync();

  this->accept();
}

//-----------------------------------------------------------------------------
void vpProjectEditor::saveProject()
{
  const auto& path = vgFileDialog::getSaveFileName(
    this, "Save Project", {}, "Project files (*.prj);;");

  if (!path.isEmpty())
  {
    this->saveProject(path);
  }
}

//-----------------------------------------------------------------------------
void vpProjectEditor::saveProject(const QString& path)
{
  QTE_D();
  d->ProjectPath = path;
  d->UI.saveInfo->setText(QString{"Project will be saved to %1."}.arg(path));
}

//-----------------------------------------------------------------------------
void vpProjectEditor::browseForDatasetFile()
{
  QTE_D();

  const auto& path = vgFileDialog::getOpenFileName(this);
  if (!path.isEmpty())
  {
    d->UI.dataset->setText(path);
  }
}

//-----------------------------------------------------------------------------
void vpProjectEditor::browseForDatasetDirectory()
{
  QTE_D();

  const auto& path = vgFileDialog::getExistingDirectory(this);
  if (!path.isEmpty())
  {
    d->UI.dataset->setText(QDir{path}.filePath("*"));
  }
}

//-----------------------------------------------------------------------------
void vpProjectEditor::browseForTracksFile()
{
  QTE_D();

  const auto& path = vgFileDialog::getOpenFileName(this);
  if (!path.isEmpty())
  {
    d->UI.tracks->setText(path);
  }
}
