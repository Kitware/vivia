// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsDescriptor_h
#define __vsDescriptor_h

#include <qtTransferablePointer.h>
#include <qtTransferablePointerArray.h>

struct vvDescriptor;

typedef qtTransferablePointer<vvDescriptor> vsDescriptor;
typedef qtTransferablePointerArray<vvDescriptor> vsDescriptorList;

#endif
