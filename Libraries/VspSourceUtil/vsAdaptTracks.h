// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsAdaptTracks_h
#define __vsAdaptTracks_h

#include <QHash>

#include <vgExport.h>

#include <vgRange.h>

class vtkVgTimeStamp;

struct vvDescriptor;
struct vsTrackId;

class vsEvent;

typedef vgRange<vtkVgTimeStamp> vsTimeInterval;
typedef QHash<vsTrackId, QMap<vsTimeInterval, void*> > vsEventTrackInfo;

extern VSP_SOURCEUTIL_EXPORT void
vsAddTracks(vsEvent&, const vsEventTrackInfo&);

#endif
