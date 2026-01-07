// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
