/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgSwatchCache_h
#define __vgSwatchCache_h

#include <QScopedPointer>

#include <qtGlobal.h>

#include <vgExport.h>

class QColor;
class QPixmap;

class vgSwatchCachePrivate;

class QTVG_COMMON_EXPORT vgSwatchCache
{
public:
  explicit vgSwatchCache(int swatchSize = 10, int cacheSize = 100);
  ~vgSwatchCache();

  QPixmap swatch(const QColor&) const;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgSwatchCache)

private:
  QTE_DECLARE_PRIVATE(vgSwatchCache)
  Q_DISABLE_COPY(vgSwatchCache)
};

#endif
