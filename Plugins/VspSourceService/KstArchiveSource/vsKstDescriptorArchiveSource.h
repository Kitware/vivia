/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsKstDescriptorArchiveSource_h
#define __vsKstDescriptorArchiveSource_h

#include <vsDescriptorSource.h>

#include <vsArchiveSource.h>

class vsKstDescriptorArchiveSourcePrivate;

class vsKstDescriptorArchiveSource :
  public vsArchiveSource<vsDescriptorSource>
{
  Q_OBJECT

public:
  vsKstDescriptorArchiveSource(const QUrl& uri);
  virtual ~vsKstDescriptorArchiveSource();

private:
  QTE_DECLARE_PRIVATE(vsKstDescriptorArchiveSource)
  QTE_DISABLE_COPY(vsKstDescriptorArchiveSource)

  typedef vsArchiveSource<vsDescriptorSource> super;
};

#endif
