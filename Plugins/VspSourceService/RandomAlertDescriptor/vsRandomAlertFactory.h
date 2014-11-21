/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsRandomAlertFactory_h
#define __vsRandomAlertFactory_h

#include <qtGlobal.h>

#include <vsSourceFactory.h>

class vsRandomAlertFactoryPrivate;

class vsRandomAlertFactory : public vsSourceFactory
{
public:
  vsRandomAlertFactory();
  virtual ~vsRandomAlertFactory();

  virtual bool initialize(QWidget* dialogParent) QTE_OVERRIDE;
  virtual bool initialize(const QUrl& uri) QTE_OVERRIDE;
  virtual QList<vsDescriptorSourcePtr> descriptorSources() const QTE_OVERRIDE;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsRandomAlertFactory)

  bool initialize();

private:
  QTE_DECLARE_PRIVATE(vsRandomAlertFactory)
  QTE_DISABLE_COPY(vsRandomAlertFactory)
};

#endif
