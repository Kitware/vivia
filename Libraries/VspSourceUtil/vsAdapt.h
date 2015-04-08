/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
