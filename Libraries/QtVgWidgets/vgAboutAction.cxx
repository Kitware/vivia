// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
