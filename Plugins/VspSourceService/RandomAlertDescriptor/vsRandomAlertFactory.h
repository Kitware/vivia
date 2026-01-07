// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
