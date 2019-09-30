/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVdfEventIO_h
#define __vpVdfEventIO_h

#include "vpEventIO.h"

#include <qtGlobal.h>

#include <QScopedPointer>

template <typename Key, typename Value> class QHash;

class QUrl;

class vpVdfEventIOPrivate;

class vpVdfEventIO : public vpEventIO
{
public:
  vpVdfEventIO(
    QHash<long long, vtkIdType>& eventSourceIdToModelIdMap,
    const QHash<long long, vtkIdType>& trackSourceIdToModelIdMap,
    vtkVgEventModel* eventModel, vtkVgEventTypeRegistry* eventTypes);

  virtual ~vpVdfEventIO();

  void SetEventsUri(const QUrl&);

  virtual bool ReadEvents() override;
  virtual bool WriteEvents(const char*) const override
    { return false; }

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpVdfEventIO);

private:
  QTE_DECLARE_PRIVATE(vpVdfEventIO);
  QTE_DISABLE_COPY(vpVdfEventIO);
};

#endif
