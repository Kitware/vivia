/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsRandomAlertDescriptor_h
#define __vsRandomAlertDescriptor_h

#include <vsLiveDescriptor.h>

class vsRandomAlertDescriptorPrivate;

class vsRandomAlertDescriptor : public vsLiveDescriptor
{
  Q_OBJECT

public:
  vsRandomAlertDescriptor();
  virtual ~vsRandomAlertDescriptor();

  virtual vsDescriptorInput::Types inputAccepted() const QTE_OVERRIDE;

protected:
  virtual QString name() const QTE_OVERRIDE;

private:
  QTE_DECLARE_PRIVATE(vsRandomAlertDescriptor)
  QTE_DISABLE_COPY(vsRandomAlertDescriptor)
};

#endif
