// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvChecksum_h
#define __vvChecksum_h

#include <QList>

#include <vgExport.h>

#include "vvDescriptor.h"

extern VV_IO_EXPORT quint16 vvChecksum(const vvDescriptor&);
extern VV_IO_EXPORT quint16 vvChecksum(const QList<vvDescriptor>&);
extern VV_IO_EXPORT quint16 vvChecksum(const std::vector<vvDescriptor>&);

#endif
