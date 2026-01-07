// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// The size of this class is kept small (sizeof(void*)) so that it can be
// efficiently passed by value and inserted into "inline" data structures
// with minimal overhead.

#ifndef __vtkVgTrackInfo_h
#define __vtkVgTrackInfo_h

#include "vgPointerInt.h"

class vtkVgTrack;

class vtkVgTrackInfo
{
  enum TrackTraits
    {
    DisplayTrack   = 1 << 0,
    PassesFilters  = 1 << 1,
    HeadVisible    = 1 << 2
    };

public:
  explicit vtkVgTrackInfo(vtkVgTrack* track = 0)
    : TrackAndTraits(track, DisplayTrack | PassesFilters | HeadVisible)
    {}

  operator bool() const
    {
    return !!this->TrackAndTraits.GetPointer();
    }

  void SetTrack(vtkVgTrack* t)
    {
    this->TrackAndTraits.SetPointer(t);
    }

  vtkVgTrack* GetTrack() const
    {
    return this->TrackAndTraits.GetPointer();
    }

  void SetDisplayTrackOn()
    {
    this->TrackAndTraits.SetBits(DisplayTrack);
    }

  void SetDisplayTrackOff()
    {
    this->TrackAndTraits.ClearBits(DisplayTrack);
    }

  bool GetDisplayTrack() const
    {
    return this->TrackAndTraits.GetBitsAreSet(DisplayTrack);
    }

  void SetPassesFiltersOn()
    {
    this->TrackAndTraits.SetBits(PassesFilters);
    }

  void SetPassesFiltersOff()
    {
    this->TrackAndTraits.ClearBits(PassesFilters);
    }

  bool GetPassesFilters() const
    {
    return this->TrackAndTraits.GetBitsAreSet(PassesFilters);
    }

  void SetHeadVisibleOn()
    {
    this->TrackAndTraits.SetBits(HeadVisible);
    }

  void SetHeadVisibleOff()
    {
    this->TrackAndTraits.ClearBits(HeadVisible);
    }

  bool GetHeadVisible() const
    {
    return this->TrackAndTraits.GetBitsAreSet(HeadVisible);
    }

private:
  vgPointerInt<vtkVgTrack*> TrackAndTraits;
};

#endif // __vtkVgTrackInfo_h
