// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgSwatchCache.h"

#include <QCache>
#include <QColor>
#include <QPixmap>

QTE_IMPLEMENT_D_FUNC(vgSwatchCache)

//-----------------------------------------------------------------------------
class vgSwatchCachePrivate
{
public:
  vgSwatchCachePrivate(int ssize, int csize) :
    size(ssize), cache(csize > 0 ? csize : 100) {}

  int size;
  mutable QCache<quint32, QPixmap> cache;
};

//-----------------------------------------------------------------------------
vgSwatchCache::vgSwatchCache(int swatchSize, int cacheSize) :
  d_ptr(new vgSwatchCachePrivate(swatchSize, cacheSize))
{
}

//-----------------------------------------------------------------------------
vgSwatchCache::~vgSwatchCache()
{
}

//-----------------------------------------------------------------------------
QPixmap vgSwatchCache::swatch(const QColor& color) const
{
  QTE_D_CONST(vgSwatchCache);

  quint32 key = color.rgba();
  if (!d->cache.contains(key))
    {
    QPixmap* pixmap = new QPixmap(d->size, d->size);
    pixmap->fill(color);
    d->cache.insert(key, pixmap);
    }
  return *d->cache[key];
}
