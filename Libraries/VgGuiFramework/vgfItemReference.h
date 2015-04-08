/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
