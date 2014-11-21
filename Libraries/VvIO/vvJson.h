/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvJson_h
#define __vvJson_h

#include <qtJson.h>

#include <vgExport.h>

#include <set>

struct vgGeoRawCoordinate;
struct vgGeocodedCoordinate;
struct vgTimeStamp;

struct vvDescriptor;
struct vvDescriptorRegionEntry;
struct vvImageBoundingBox;
struct vvImagePointF;
struct vvQueryResult;
struct vvTrack;
struct vvTrackId;
struct vvTrackState;

namespace vvUserData { struct Data; }

namespace vvJson
{
  extern VV_IO_EXPORT qtJson::Value serialize(const vgTimeStamp&);
  extern VV_IO_EXPORT qtJson::Object serialize(const vgGeoRawCoordinate&);
  extern VV_IO_EXPORT qtJson::Value serialize(const vgGeocodedCoordinate&);

  extern VV_IO_EXPORT qtJson::Object serialize(const vvImagePointF&);
  extern VV_IO_EXPORT qtJson::Object serialize(const vvImageBoundingBox&);

  extern VV_IO_EXPORT qtJson::Object serialize(const vvTrackId&);
  extern VV_IO_EXPORT qtJson::Object serialize(const vvTrackState&);
  extern VV_IO_EXPORT qtJson::Object serialize(const vvTrack&);

  extern VV_IO_EXPORT qtJson::Object serialize(const vvDescriptorRegionEntry&);
  extern VV_IO_EXPORT qtJson::Object serialize(const vvDescriptor&);
  extern VV_IO_EXPORT qtJson::Object serialize(const vvUserData::Data&);

  extern VV_IO_EXPORT qtJson::Object serialize(const vvQueryResult&);

  template <typename T> qtJson::JsonData encode(const T&);
}

//-----------------------------------------------------------------------------
template <typename T>
qtJson::JsonData vvJson::encode(const T& object)
{
    return qtJson::encode(serialize(object));
}

#endif
