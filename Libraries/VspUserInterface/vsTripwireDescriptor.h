// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
