// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpExternalProcessDialog.h"

// QT includes.
#include <QMessageBox>

// VisGUI includes.
#include "vpSettings.h"

//----------------------------------------------------------------------------
vpExternalProcessDialog::vpExternalProcessDialog(QWidget* parent) :
  QDialog(parent)
{
  this->Settings = new vpSettings;
  this->UI.setupUi(this);
  this->UI.editArguments->setWordWrapMode(QTextOption::WrapAnywhere);

  connect(this->UI.executeButton, SIGNAL(released()),
          this, SLOT(executeProcess()));

  connect(this->UI.saveButton, SIGNAL(released()),
          this, SLOT(saveWithoutExecuting()));

  connect(this->UI.cancelButton, SIGNAL(released()),
          this, SLOT(cancel()));

  this->restore();
}

//----------------------------------------------------------------------------
vpExternalProcessDialog::~vpExternalProcessDialog()
{
  delete this->Settings;
  this->Settings = 0;
}

//----------------------------------------------------------------------------
void vpExternalProcessDialog::save()
{
  this->Settings->setExternalProcessProgram(this->UI.editProgram->text());
  this->Settings->setExternalProcessArgs(
    this->UI.editArguments->toPlainText().split(";"));
  this->Settings->setExternalProcessIOPath(this->UI.editIOPath->text());

  this->Settings->setExternalProcessThreshold(
    this->UI.thresholdDoubleSpinBox->value());
  this->Settings->setExternalProcessThresholdState(
    this->UI.thresholdCheckBox->isChecked());

  this->Settings->setExternalProcessCellSize(
    this->UI.cellSizeDoubleSpinBox->value());
  this->Settings->setExternalProcessCellSizeState(
    this->UI.cellSizeCheckBox->isChecked());

  // Now commit all of the changes
  this->Settings->commit();
}

//----------------------------------------------------------------------------
void vpExternalProcessDialog::restore()
{
  this->UI.editProgram->setText(this->Settings->externalProcessProgram());
  this->UI.editArguments->setText(
    this->Settings->externalProcessArgs().join(";"));
  this->UI.editIOPath->setText(this->Settings->externalProcessIOPath());

  this->UI.thresholdCheckBox->setChecked(
    this->Settings->externalProcessThresholdState());
  this->UI.thresholdDoubleSpinBox->setValue(
      this->Settings->externalProcessThreshold());

  this->UI.cellSizeCheckBox->setChecked(
    this->Settings->externalProcessCellSizeState());
  this->UI.cellSizeDoubleSpinBox->setValue(
      this->Settings->externalProcessCellSize());
}

//----------------------------------------------------------------------------
void vpExternalProcessDialog::executeProcess()
{
  if (this->UI.editProgram->text().isEmpty())
    {
    QMessageBox::warning(this, "Warning!",
                         "No program specified; unable to execute!");
    return;
    }
  this->save();

  QDialog::accept();
}

//----------------------------------------------------------------------------
void vpExternalProcessDialog::saveWithoutExecuting()
{
  this->save();
  QDialog::reject();
}

//----------------------------------------------------------------------------
void vpExternalProcessDialog::cancel()
{
  QDialog::reject();
}

//----------------------------------------------------------------------------
QString vpExternalProcessDialog::getProgram() const
{
  return this->UI.editProgram->text();
}

//----------------------------------------------------------------------------
QStringList vpExternalProcessDialog::getArguments() const
{
  QStringList args = this->UI.editArguments->toPlainText().split(";");
  if (this->UI.cellSizeCheckBox->isChecked())
    {
    args.append(QString("-cellsize %1")
                  .arg(this->UI.cellSizeDoubleSpinBox->value()));
    }
  if (this->UI.cellSizeCheckBox->isChecked())
    {
    args.append(QString("-threshold %1")
              .arg(this->UI.thresholdDoubleSpinBox->value()));
    }
  return args;
}

//----------------------------------------------------------------------------
QString vpExternalProcessDialog::getIOPath() const
{
  return this->UI.editIOPath->text();
}
