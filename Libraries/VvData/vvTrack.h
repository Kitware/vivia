/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvTrack_h
#define __vvTrack_h

#include <map>
#include <set>
#include <string>

#include <vgGeoTypes.h>
#include <vgTimeStamp.h>

//-----------------------------------------------------------------------------
struct vvImagePoint
{
  vvImagePoint() : X(-1), Y(-1) {}
  vvImagePoint(int x, int y) : X(x), Y(y) {}
  int X, Y;
};

//-----------------------------------------------------------------------------
struct vvImagePointF
{
  vvImagePointF() : X(-1), Y(-1) {}
  vvImagePointF(double x, double y) : X(x), Y(y) {}

  double X, Y;
};
typedef std::vector<vvImagePointF> vvImagePolygonF;

//-----------------------------------------------------------------------------
struct vvImageBoundingBox
{
  vvImagePoint TopLeft, BottomRight;
};

//-----------------------------------------------------------------------------
struct vvTrackId
{
  int Source;
  long long SerialNumber;

  vvTrackId()
    : Source(-1), SerialNumber(-1) {}
  vvTrackId(int source, long long serialNumber)
    : Source(source), SerialNumber(serialNumber) {}

  inline bool operator==(const vvTrackId& other) const;
  inline bool operator<(const vvTrackId& other) const;
};

//-----------------------------------------------------------------------------
struct vvTrackState
{
  vgTimeStamp TimeStamp;
  vvImagePointF ImagePoint;
  vvImageBoundingBox ImageBox;
  vvImagePolygonF ImageObject;
  vgGeocodedCoordinate WorldLocation;

  inline bool operator<(const vvTrackState& other) const;
  inline bool operator<(const vgTimeStamp& timestamp) const;
  inline bool operator>(const vgTimeStamp& timestamp) const;
};

//-----------------------------------------------------------------------------
typedef std::map<std::string, double> vvTrackObjectClassification;
typedef std::set<vvTrackState> vvTrackTrajectory;
struct vvTrack
{
  vvTrackId Id;
  vvTrackObjectClassification Classification;
  vvTrackTrajectory Trajectory;
};

//-----------------------------------------------------------------------------
bool vvTrackId::operator==(const vvTrackId& other) const
{
  return (this->SerialNumber == other.SerialNumber &&
          this->Source == other.Source);
}
//-----------------------------------------------------------------------------
bool vvTrackId::operator<(const vvTrackId& other) const
{
  return (this->Source < other.Source) ||
         ((this->Source == other.Source) &&
          (this->SerialNumber < other.SerialNumber));
}

//-----------------------------------------------------------------------------
bool vvTrackState::operator<(const vvTrackState& other) const
{
  return this->TimeStamp < other.TimeStamp;
}

//-----------------------------------------------------------------------------
bool vvTrackState::operator<(const vgTimeStamp& timestamp) const
{
  return this->TimeStamp < timestamp;
}

//-----------------------------------------------------------------------------
bool vvTrackState::operator>(const vgTimeStamp& timestamp) const
{
  return timestamp < this->TimeStamp;
}

//-----------------------------------------------------------------------------
extern unsigned int qHash(long long);
inline unsigned int qHash(const vvTrackId& tid)
{
  static const int sourceRotateBits = 4;
  static const int sourceReverseRotateBits =
    ((8 * sizeof(unsigned int)) - sourceRotateBits);

  unsigned int source = static_cast<unsigned int>(tid.Source);
  unsigned int sourceHash = (source >> sourceRotateBits) & (~0U);
  sourceHash ^= (source << sourceReverseRotateBits & (~0U));
  return qHash(tid.SerialNumber) ^ sourceHash;
}

#endif
