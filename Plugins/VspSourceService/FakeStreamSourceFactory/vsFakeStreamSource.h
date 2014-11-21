/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
