/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackId_h
#define __vsTrackId_h

#include <vvTrack.h>

#include <QUuid>

//-----------------------------------------------------------------------------
struct vsTrackId : vvTrackId
{
  vsTrackId() : vvTrackId(), UniqueId() {}
  vsTrackId(const vvTrackId& other) : vvTrackId(other), UniqueId() {}
  vsTrackId(int source, long long serialNumber, const QUuid& uniqueId) :
    vvTrackId(source, serialNumber), UniqueId(uniqueId) {}

  QUuid UniqueId;

  inline bool operator==(const vsTrackId& other) const;
};

//-----------------------------------------------------------------------------
bool vsTrackId::operator==(const vsTrackId& other) const
{
  return (this->vvTrackId::operator==(other) &&
          this->UniqueId == other.UniqueId);
}

#endif
