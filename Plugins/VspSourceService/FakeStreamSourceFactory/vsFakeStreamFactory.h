/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsFakeStreamFactory_h
#define __vsFakeStreamFactory_h

#include <vsStreamFactory.h>

class vsFakeStreamFactory : public vsStreamFactory
{
public:
  vsFakeStreamFactory();
  virtual ~vsFakeStreamFactory();

  virtual bool initialize(QWidget* dialogParent) QTE_OVERRIDE;
  virtual bool initialize(const QUrl& uri) QTE_OVERRIDE;

protected:
  bool initialize(const QUrl& uri, QWidget* dialogParent);

private:
  QTE_DISABLE_COPY(vsFakeStreamFactory)
};

#endif
