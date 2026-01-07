// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
