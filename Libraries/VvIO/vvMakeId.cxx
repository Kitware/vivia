// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
