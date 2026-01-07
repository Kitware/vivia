// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpFileUtil_h
#define __vpFileUtil_h

class QDir;
class QString;
class QStringList;

extern QStringList vpGlobFiles(const QDir& base, const QString& pattern);

#endif
