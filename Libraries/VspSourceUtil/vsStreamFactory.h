/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
