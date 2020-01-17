/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFileUtil.h"

#include <QDir>
#include <QStringList>

//-----------------------------------------------------------------------------
QStringList vpGlobFiles(const QDir& base, const QString& pattern)
{
  QStringList matches;

  // TODO handle multi-level globs
  foreach (const auto& p, base.entryList(QDir::Files))
  {
    if (QDir::match(pattern, p))
    {
      matches.append(base.filePath(p));
    }
  }

  return matches;
}
