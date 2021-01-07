// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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

  inline operator bool() const;
  inline bool operator!() const;

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
vvTrackId::operator bool() const
{
  return (this->Source != -1 || this->SerialNumber != -1);
}

//-----------------------------------------------------------------------------
bool vvTrackId::operator!() const
{
  return (this->Source == -1 && this->SerialNumber == -1);
}

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

#endif
