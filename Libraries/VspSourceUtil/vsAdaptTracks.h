/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsAdaptTracks_h
#define __vsAdaptTracks_h

#include <QHash>

#include <vgExport.h>

#include <vgRange.h>

class vtkVgTimeStamp;

struct vvDescriptor;
struct vvTrackId;

class vsEvent;

typedef vgRange<vtkVgTimeStamp> vsTimeInterval;
typedef QHash<vvTrackId, QMap<vsTimeInterval, void*> > vsEventTrackInfo;

extern VSP_SOURCEUTIL_EXPORT void
vsAddTracks(vsEvent&, const vsEventTrackInfo&);

#endif
