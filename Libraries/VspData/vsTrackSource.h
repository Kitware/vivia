/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackSource_h
#define __vsTrackSource_h

#include <vgExport.h>

#include <vvTrack.h>

#include "vsDataSource.h"
#include "vsTrackData.h"

class VSP_DATA_EXPORT vsTrackSource : public vsDataSource
{
  Q_OBJECT

public:
  virtual ~vsTrackSource() {}

signals:
  void trackUpdated(vvTrackId trackId, vvTrackState state);
  void trackUpdated(vvTrackId trackId, QList<vvTrackState> states);
  void trackDataUpdated(vvTrackId trackId, vsTrackData data);
  void trackClosed(vvTrackId trackId);

  void destroyed(vsTrackSource*);

protected:
  vsTrackSource() {}

  void suicide() { emit this->destroyed(this); }

private:
  QTE_DISABLE_COPY(vsTrackSource)
};

#endif
