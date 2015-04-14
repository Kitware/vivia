/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgUserManualAction.h"

#include "vgApplication.h"

#include <qtUtil.h>

#include <QDesktopServices>
#include <QUrl>

//-----------------------------------------------------------------------------
vgUserManualAction::vgUserManualAction(QObject* parent) : QAction(parent)
{
  this->setText(qApp->applicationName() + " User &Manual");
  this->setIcon(qtUtil::standardActionIcon("help-manual"));
  this->setShortcut(QKeySequence::HelpContents);
  connect(this, SIGNAL(triggered()), this, SLOT(showUserManual()));
}

//-----------------------------------------------------------------------------
vgUserManualAction::~vgUserManualAction()
{
}

//-----------------------------------------------------------------------------
void vgUserManualAction::showUserManual()
{
  const QString& path = vgApplication::userManualLocation();
  if (!path.isEmpty())
    {
    const QUrl& uri = QUrl::fromLocalFile(path);
    QDesktopServices::openUrl(uri);
    }
}
