/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsDescriptor_h
#define __vsDescriptor_h

#include <qtTransferablePointer.h>
#include <qtTransferablePointerArray.h>

struct vvDescriptor;

typedef qtTransferablePointer<vvDescriptor> vsDescriptor;
typedef qtTransferablePointerArray<vvDescriptor> vsDescriptorList;

#endif
