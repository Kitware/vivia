// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "ui_qtRenderWindowDialog.h"

#include "vpRenderWindowDialog.h"
#include "QVTKWidget.h"

//-----------------------------------------------------------------------------
vpRenderWindowDialog::vpRenderWindowDialog(QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags)
{
  this->InternalWidget = new Ui::qtRenderWindowDialog;
  this->InternalWidget->setupUi(this);
}

//-----------------------------------------------------------------------------
vpRenderWindowDialog::~vpRenderWindowDialog()
{
  delete this->InternalWidget;
}

//-----------------------------------------------------------------------------
vtkRenderWindow* vpRenderWindowDialog::GetRenderWindow()
{
  return this->InternalWidget->qvtkRenderWidget->GetRenderWindow();
}