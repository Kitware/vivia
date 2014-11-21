/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsLiveDescriptorPrivate_h
#define __vsLiveDescriptorPrivate_h

#include <qtThread.h>

#include <vgExport.h>

#include "vsLiveDescriptor.h"

class QTimer;

class VSP_DATA_EXPORT vsLiveDescriptorPrivate : public qtThread
{
  Q_OBJECT

public:
  vsLiveDescriptorPrivate(vsLiveDescriptor*);
  virtual ~vsLiveDescriptorPrivate();

signals:
  void eventAvailable(vsEvent event);
  void eventRevoked(vtkIdType eventId);

protected slots:
  virtual void injectInput(qint64 id, vsDescriptorInputPtr input)
    { Q_UNUSED(id); Q_UNUSED(input); }

  virtual void revokeInput(qint64 id, bool revokeEvents)
    { Q_UNUSED(id); Q_UNUSED(revokeEvents); }

  virtual void revokeAllInput(bool revokeEvents)
    { Q_UNUSED(revokeEvents); }

  virtual void suicide();

protected:
  QTE_DECLARE_PUBLIC_PTR(vsLiveDescriptor)

  virtual void run() QTE_OVERRIDE;
  void setSuicideTimer(int timeoutMilliseconds = 30000,
                       bool setInactiveStatus = true);
  void cancelSuicideTimer(bool setActiveStatus = true);

  vsDataSource::Status status;

private:
  QTE_DECLARE_PUBLIC(vsLiveDescriptor)
  QTE_DISABLE_COPY(vsLiveDescriptorPrivate)

  QScopedPointer<QTimer> suicideTimer;
};

#endif
