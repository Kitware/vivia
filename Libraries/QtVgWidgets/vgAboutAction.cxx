/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgAboutAction.h"

#include <QApplication>

#include "vgAboutDialog.h"

//-----------------------------------------------------------------------------
vgAboutAction::vgAboutAction(QObject* parent) : QAction(parent)
{
  this->setText("&About " + qApp->applicationName());
  this->setIcon(qApp->windowIcon());
  connect(this, SIGNAL(triggered()), this, SLOT(showDialog()));
}

//-----------------------------------------------------------------------------
vgAboutAction::~vgAboutAction()
{
}

//-----------------------------------------------------------------------------
void vgAboutAction::showDialog()
{
  vgAboutDialog dialog;
  dialog.exec();
}
