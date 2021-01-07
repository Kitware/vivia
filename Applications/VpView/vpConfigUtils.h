// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpConfigUtils_h
#define __vpConfigUtils_h

class QColor;
class QSettings;
class QString;

namespace vpConfigUtils
{
QColor ReadColor(const QString& key, QSettings& settings);
void  WriteColor(const QString& key, const QColor& color, QSettings& settings);
};

#endif // __vpConfigUtils_h
