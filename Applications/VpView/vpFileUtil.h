/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFileUtil_h
#define __vpFileUtil_h

class QDir;
class QString;
class QStringList;

extern QStringList vpGlobFiles(const QDir& base, const QString& pattern);

#endif
