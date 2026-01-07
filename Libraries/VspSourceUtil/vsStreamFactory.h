// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsStreamFactory_h
#define __vsStreamFactory_h

#include <vgExport.h>

#include <vsSourceFactory.h>

class vsStreamFactoryPrivate;
class vsStreamSource;

class VSP_SOURCEUTIL_EXPORT vsStreamFactory : public vsSourceFactory
{
public:
  vsStreamFactory();
  virtual ~vsStreamFactory();

  virtual vsVideoSourcePtr videoSource() const QTE_OVERRIDE;
  virtual QList<vsTrackSourcePtr> trackSources() const QTE_OVERRIDE;
  virtual QList<vsDescriptorSourcePtr> descriptorSources() const QTE_OVERRIDE;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsStreamFactory)

  void setSource(vsStreamSource*);

private:
  QTE_DECLARE_PRIVATE(vsStreamFactory)
  QTE_DISABLE_COPY(vsStreamFactory)
};

#endif
