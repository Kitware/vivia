/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
