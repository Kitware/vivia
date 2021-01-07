// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
