/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsStreamSource_h
#define __vsStreamSource_h

#include <vgExport.h>

#include <vsSimpleSourceFactory.h>
#include <vsVideoSource.h>

class vsStreamSourcePrivate;

class VSP_SOURCEUTIL_EXPORT vsStreamSource : public vsVideoSource
{
  Q_OBJECT

public:
  virtual ~vsStreamSource();

  virtual QString text() const QTE_OVERRIDE;
  virtual QString toolTip() const QTE_OVERRIDE;

protected:
  friend class vsStreamFactory;

  vsStreamSource(vsStreamSourcePrivate*);

  virtual vsTrackSourcePtr trackSource();
  virtual vsDescriptorSourcePtr descriptorSource();

private:
  QTE_DECLARE_PRIVATE(vsStreamSource)
  QTE_DISABLE_COPY(vsStreamSource)
};

#endif
