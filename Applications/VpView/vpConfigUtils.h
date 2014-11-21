/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
