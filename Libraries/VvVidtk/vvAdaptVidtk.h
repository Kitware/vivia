// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvAdaptVidtk_h
#define __vvAdaptVidtk_h

#include <vgExport.h>

namespace vidtk { struct track_state; }
template <typename T> class QList;
struct vvTrackState;

extern VV_VIDTK_EXPORT QList<vvTrackState>
vvAdapt(const vidtk::track_state&);

#endif
