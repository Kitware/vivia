/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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