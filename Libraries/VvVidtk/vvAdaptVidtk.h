/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvAdaptVidtk_h
#define __vvAdaptVidtk_h

#include <vgExport.h>

namespace vidtk { struct track_state; }
template <typename T> class QList;
struct vvTrackState;

extern VV_VIDTK_EXPORT QList<vvTrackState>
vvAdapt(const vidtk::track_state&);

#endif
