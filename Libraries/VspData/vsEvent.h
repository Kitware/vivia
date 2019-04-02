/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsEvent_h
#define __vsEvent_h

#include <QMetaType>
#include <QUuid>

#include <vtkVgEventBase.h>
#include <vtkVgSharedInstance.h>

class vsDescriptorSource;

class vsEvent : public vtkVgSharedInstance<vtkVgEventBase>
{
public:
  vsEvent(const QUuid& u) : UniqueId(u) {}

  QUuid GetUniqueId() const { return this->UniqueId; }
  void SetUniqueId(const QUuid& u) { this->UniqueId = u; }

protected:
  // Default construction allowed only through Qt's meta-type system, to allow
  // use as QVariant; direct users must specify a UUID
  template <typename, bool>
  friend struct QtMetaTypePrivate::QMetaTypeFunctionHelper;

  vsEvent() {}

  QUuid UniqueId;
};

struct vsEventId
{
  vsEventId()
    : GlobalId(-1), SourceId(-1), Source(0) {}
  vsEventId(const QUuid& uid, vtkIdType gid,
            vtkIdType sid, const vsDescriptorSource* s)
    : UniqueId(uid), GlobalId(gid), SourceId(sid), Source(s) {}

  QUuid UniqueId;
  vtkIdType GlobalId;
  vtkIdType SourceId;
  const vsDescriptorSource* Source;
};

Q_DECLARE_METATYPE(QUuid)
Q_DECLARE_METATYPE(vgTimeStamp)
Q_DECLARE_METATYPE(vsEventId)

#endif
