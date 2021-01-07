// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgKwaUtil.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#define die(...) do { \
  qDebug() << "vgKwaUtil:" << __VA_ARGS__; \
  return false; \
  } while (0)

//-----------------------------------------------------------------------------
bool vgKwaUtil::resolvePath(
  QString& name, const QString& relativeTo, const char* what)
{
  // Look for the file in four places:
  // 0. absolute path (if <name> is absolute)
  // 1. relative to the current working directory
  // 2. relative to <relativeTo>
  // 3. relative to the canonical location of <relativeTo>
  //
  // Most likely, <name> is given as a path relative to the canonical location
  // of <relativeTo>. Since the path we received may be a symlink, we should
  // try (3) as well as (2).
  QFileInfo fi(name);
  if (fi.isAbsolute())
    {
    if (fi.exists())
      {
      return true;
      }
    die("Unable to find" << what << "file" << name);
    }

  fi = QFileInfo(QDir(), name);
  QString try1 = fi.absoluteFilePath();
  if (fi.exists())
    {
    name = try1;
    return true;
    }

  QFileInfo rfi(relativeTo);
  fi = QFileInfo(rfi.path(), name);
  QString try2 = fi.absoluteFilePath();
  if (fi.exists())
    {
    name = try2;
    return true;
    }

  fi = QFileInfo(rfi.canonicalPath(), name);
  QString try3 = fi.absoluteFilePath();
  if (fi.exists())
    {
    name = try3;
    return true;
    }

  die("Unable to find" << what << "file" << name
      << "\n  Tried:" << try1
      << "\n        " << try2
      << "\n        " << try3);
}
