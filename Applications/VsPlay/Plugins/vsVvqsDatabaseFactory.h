// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsVvqsDatabaseFactory_h
#define __vsVvqsDatabaseFactory_h

#include <qtGlobal.h>

#include <vsSourceFactory.h>

class vsVvqsDatabaseFactoryPrivate;

class vsVvqsDatabaseFactory : public vsSourceFactory
{
public:
  vsVvqsDatabaseFactory();
  virtual ~vsVvqsDatabaseFactory();

  virtual bool initialize(QWidget* dialogParent) QTE_OVERRIDE;
  virtual bool initialize(const QUrl& uri) QTE_OVERRIDE;

  virtual QList<vsTrackSourcePtr> trackSources() const QTE_OVERRIDE;
  virtual QList<vsDescriptorSourcePtr> descriptorSources() const QTE_OVERRIDE;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsVvqsDatabaseFactory)

  bool initialize(const QUrl& uri, QWidget* dialogParent);

private:
  QTE_DECLARE_PRIVATE(vsVvqsDatabaseFactory)
  QTE_DISABLE_COPY(vsVvqsDatabaseFactory)
};

#endif
