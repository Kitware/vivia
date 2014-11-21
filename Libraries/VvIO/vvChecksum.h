/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvChecksum_h
#define __vvChecksum_h

#include <QList>

#include <vgExport.h>

#include "vvDescriptor.h"

extern VV_IO_EXPORT quint16 vvChecksum(const vvDescriptor&);
extern VV_IO_EXPORT quint16 vvChecksum(const QList<vvDescriptor>&);
extern VV_IO_EXPORT quint16 vvChecksum(const std::vector<vvDescriptor>&);

#endif
