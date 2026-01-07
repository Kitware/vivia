// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
