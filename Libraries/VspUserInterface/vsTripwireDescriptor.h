/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTripwireDescriptor_h
#define __vsTripwireDescriptor_h

#include <vsLiveDescriptor.h>

class vsTripwireDescriptorPrivate;

class vsTripwireDescriptor : public vsLiveDescriptor
{
  Q_OBJECT

public:
  vsTripwireDescriptor();
  virtual ~vsTripwireDescriptor();

  virtual vsDescriptorInput::Types inputAccepted() const QTE_OVERRIDE;

protected:
  QTE_DECLARE_PRIVATE(vsTripwireDescriptor)

  virtual QString name() const;
};

#endif
