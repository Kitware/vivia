// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsSourceFactory.h"

#include <QDebug>
#include <QMessageBox>

//-----------------------------------------------------------------------------
vsSourceFactory::vsSourceFactory()
{
}

//-----------------------------------------------------------------------------
vsSourceFactory::~vsSourceFactory()
{
}

//-----------------------------------------------------------------------------
void vsSourceFactory::warn(
  QWidget* dialogParent, const QString& dialogTitle, const QString& message)
{
  if (dialogParent)
    {
    QMessageBox::warning(dialogParent, dialogTitle, message);
    }
  else
    {
    qWarning() << "vsArchiveSourceFactory:" << message;
    }
}
