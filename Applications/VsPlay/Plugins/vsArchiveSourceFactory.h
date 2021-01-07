// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsArchiveSourceFactory_h
#define __vsArchiveSourceFactory_h

#include <qtGlobal.h>

#include <vsArchiveSourceInfo.h>
#include <vsSourceFactory.h>

class vsArchiveSourceFactoryPrivate;

class vsArchiveSourceFactory : public vsSourceFactory
{
public:
  explicit vsArchiveSourceFactory(vsArchiveSourceType);
  virtual ~vsArchiveSourceFactory();

  virtual bool initialize(QWidget* dialogParent) QTE_OVERRIDE;
  virtual bool initialize(const QUrl& uri) QTE_OVERRIDE;

  virtual vsVideoSourcePtr videoSource() const QTE_OVERRIDE;
  virtual QList<vsTrackSourcePtr> trackSources() const QTE_OVERRIDE;
  virtual QList<vsDescriptorSourcePtr> descriptorSources() const QTE_OVERRIDE;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsArchiveSourceFactory)

  virtual bool initialize(const QUrl& uri, QWidget* dialogParent);

private:
  QTE_DECLARE_PRIVATE(vsArchiveSourceFactory)
  QTE_DISABLE_COPY(vsArchiveSourceFactory)
};

#endif
