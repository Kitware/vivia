// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
