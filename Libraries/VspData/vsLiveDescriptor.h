/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsLiveDescriptor_h
#define __vsLiveDescriptor_h

#include <qtThread.h>

#include <vgExport.h>

#include "vsDescriptorSource.h"

class vsLiveDescriptorPrivate;

class VSP_DATA_EXPORT vsLiveDescriptor : public vsDescriptorSource
{
  Q_OBJECT

public:
  virtual ~vsLiveDescriptor();

  virtual void start();

  virtual Status status() const QTE_OVERRIDE;
  virtual QString text() const QTE_OVERRIDE;
  virtual QString toolTip() const QTE_OVERRIDE;

protected slots:
  void updateStatus(vsDataSource::Status);

protected:
  QTE_DECLARE_PRIVATE_PTR(vsLiveDescriptor)

  vsLiveDescriptor(vsLiveDescriptorPrivate*);

  virtual QString name() const = 0;

  virtual void injectInput(qint64 id, vsDescriptorInputPtr input) QTE_OVERRIDE;
  virtual void revokeInput(qint64 id, bool revokeEvents) QTE_OVERRIDE;
  virtual void revokeAllInput(bool revokeEvents) QTE_OVERRIDE;

private:
  QTE_DECLARE_PRIVATE(vsLiveDescriptor)
  QTE_DISABLE_COPY(vsLiveDescriptor)
};

#endif
