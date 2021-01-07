// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
