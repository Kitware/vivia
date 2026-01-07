// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsFakeStreamSource_h
#define __vsFakeStreamSource_h

#include <vsStreamSource.h>

class QUrl;

class vsFakeStreamSourcePrivate;

class vsFakeStreamSource : public vsStreamSource
{
  Q_OBJECT

public:
  vsFakeStreamSource(const QUrl& streamUri);
  virtual ~vsFakeStreamSource();

private:
  QTE_DECLARE_PRIVATE(vsFakeStreamSource)
  QTE_DISABLE_COPY(vsFakeStreamSource)
};

#endif
