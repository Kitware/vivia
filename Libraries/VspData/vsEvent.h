// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
