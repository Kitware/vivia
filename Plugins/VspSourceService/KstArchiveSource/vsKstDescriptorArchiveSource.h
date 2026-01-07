// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
