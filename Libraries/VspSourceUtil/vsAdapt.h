// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsAdapt_h
#define __vsAdapt_h

#include <QList>

#include <vtkSmartPointer.h>

#include <vgExport.h>

class vtkImageData;

class vgImage;

struct vvDescriptor;
struct vvQueryResult;

struct vsTrackId;

class vsEvent;

extern VSP_SOURCEUTIL_EXPORT vsEvent
vsAdapt(const vvQueryResult&);

extern VSP_SOURCEUTIL_EXPORT vsTrackId
vsAdaptTrackId(unsigned int);

extern VSP_SOURCEUTIL_EXPORT bool
vsExtractClassifier(const vvDescriptor&, QList<vsEvent>&);

#endif
