/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsPvoDescriptorArchiveSource_h
#define __vsPvoDescriptorArchiveSource_h

#include <vsDescriptorSource.h>

#include <vsArchiveSource.h>

class vsPvoDescriptorArchiveSourcePrivate;

class vsPvoDescriptorArchiveSource :
  public vsArchiveSource<vsDescriptorSource>
{
  Q_OBJECT

public:
  vsPvoDescriptorArchiveSource(const QUrl& uri);
  virtual ~vsPvoDescriptorArchiveSource();

private:
  QTE_DECLARE_PRIVATE(vsPvoDescriptorArchiveSource)
  QTE_DISABLE_COPY(vsPvoDescriptorArchiveSource)

  typedef vsArchiveSource<vsDescriptorSource> super;
};

#endif
