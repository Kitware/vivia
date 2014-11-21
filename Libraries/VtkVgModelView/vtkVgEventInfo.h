/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// The size of this class is kept small (sizeof(void*)) so that it can be
// efficiently passed by value and inserted into "inline" data structures
// with minimal overhead.

#ifndef __vtkVgEventInfo_h
#define __vtkVgEventInfo_h

#include "vgPointerInt.h"

class vtkVgEvent;

class vtkVgEventInfo
{
  enum EventTraits
    {
    DisplayEvent          = 1 << 0,
    PassesSpatialFilters  = 1 << 1,
    PassesTemporalFilters = 1 << 2,
    PassesFilters         = PassesSpatialFilters | PassesTemporalFilters
    };

public:
  explicit vtkVgEventInfo(vtkVgEvent* event = 0)
    : EventAndTraits(event, DisplayEvent | PassesFilters)
    {}

  void SetEvent(vtkVgEvent* e)
    {
    this->EventAndTraits.SetPointer(e);
    }

  vtkVgEvent* GetEvent() const
    {
    return this->EventAndTraits.GetPointer();
    }

  void SetDisplayEventOn()
    {
    this->EventAndTraits.SetBits(DisplayEvent);
    }

  void SetDisplayEventOff()
    {
    this->EventAndTraits.ClearBits(DisplayEvent);
    }

  bool GetDisplayEvent() const
    {
    return this->EventAndTraits.GetBitsAreSet(DisplayEvent);
    }

  void SetPassesSpatialFiltersOn()
    {
    this->EventAndTraits.SetBits(PassesSpatialFilters);
    }

  void SetPassesSpatialFiltersOff()
    {
    this->EventAndTraits.ClearBits(PassesSpatialFilters);
    }

  bool GetPassesSpatialFilters() const
    {
    return this->EventAndTraits.GetBitsAreSet(PassesSpatialFilters);
    }

  void SetPassesTemporalFiltersOn()
    {
    this->EventAndTraits.SetBits(PassesTemporalFilters);
    }

  void SetPassesTemporalFiltersOff()
    {
    this->EventAndTraits.ClearBits(PassesTemporalFilters);
    }

  bool GetPassesTemporalFilters() const
    {
    return this->EventAndTraits.GetBitsAreSet(PassesTemporalFilters);
    }

  void SetPassesFiltersOn()
    {
    this->EventAndTraits.SetBits(PassesFilters);
    }

  void SetPassesFiltersOff()
    {
    this->EventAndTraits.ClearBits(PassesFilters);
    }

  bool GetPassesFilters() const
    {
    return this->EventAndTraits.GetBitsAreSet(PassesFilters);
    }

private:
  vgPointerInt<vtkVgEvent*> EventAndTraits;
};

#endif // __vtkVgEventInfo_h
