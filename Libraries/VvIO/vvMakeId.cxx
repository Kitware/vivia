/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvMakeId.h"

#include <QUuid>

#include <qtStlUtil.h>

//-----------------------------------------------------------------------------
std::string vvMakeId(QString prefix)
{
  QString id = QUuid::createUuid().toString();
  id = id.mid(1, id.length() - 2).toUpper();
  if (!prefix.isEmpty())
    id = QString("%2-%1").arg(id).arg(prefix);
  return stdString(id);
}
