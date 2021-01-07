// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgfItemReference_h
#define __vgfItemReference_h

#include "vgfNamespace.h"

/// Item reference.
///
/// This struct provides a unique internal reference to an item. It is used to
/// communicate the identity of an item between internal processes where the
/// logical ID is not needed, such as between any combination of item views,
/// scenes and/or utility widgets (e.g. vgfContextMenu).
struct vgfItemReference
{
  explicit vgfItemReference(vgf::ItemType type = vgf::InvalidItemType,
                            qint64 internalId = -1) :
    Type(type), InternalId(internalId) {}

  vgf::ItemType Type;
  qint64 InternalId;
};

Q_DECLARE_METATYPE(vgfItemReference)

#endif
