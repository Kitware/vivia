/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgStringLiteral_h
#define __vgStringLiteral_h

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

#include <QStringLiteral>

#else

#include <QString>

template <int N>
static inline QString vgUtf16StringLiteral(char16_t const (&data)[N])
{
  return QString::fromUtf16(reinterpret_cast<ushort const*>(data), N - 1);
}

#define QStringLiteral(...) vgUtf16StringLiteral(u ## __VA_ARGS__)

#endif

#endif
