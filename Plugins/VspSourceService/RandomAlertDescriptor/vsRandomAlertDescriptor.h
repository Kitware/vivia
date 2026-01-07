// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
