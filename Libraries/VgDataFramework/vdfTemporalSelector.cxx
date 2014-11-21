/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfTemporalSelector.h"

#include <vgTimeStamp.h>

//-----------------------------------------------------------------------------
class vdfTemporalSelectorPrivate
{
public:
  vdfTemporalSelectorPrivate(const vgTimeStamp& ts, vg::SeekMode sm,
                             vdfTemporalSelector::TemporalSelectionFlags sf);
  vgTimeStamp TimeStamp;
  vg::SeekMode SeekMode;
  vdfTemporalSelector::TemporalSelectionFlags Options;
};

QTE_IMPLEMENT_D_FUNC(vdfTemporalSelector)

//-----------------------------------------------------------------------------
vdfTemporalSelectorPrivate::vdfTemporalSelectorPrivate(
  const vgTimeStamp& ts, vg::SeekMode sm,
  vdfTemporalSelector::TemporalSelectionFlags sf) :
  TimeStamp(ts), SeekMode(sm), Options(sf)
{
}

//-----------------------------------------------------------------------------
vdfTemporalSelector::vdfTemporalSelector(
  const vgTimeStamp& ts, vg::SeekMode sm, TemporalSelectionFlags sf) :
  d_ptr(new vdfTemporalSelectorPrivate(ts, sm, sf))
{
}

//-----------------------------------------------------------------------------
vdfTemporalSelector::vdfTemporalSelector(
  const vgTimeStamp& ts, TemporalSelectionFlags sf) :
  d_ptr(new vdfTemporalSelectorPrivate(ts, vg::SeekNearest, sf))
{
}

//-----------------------------------------------------------------------------
vdfTemporalSelector::~vdfTemporalSelector()
{
}

//-----------------------------------------------------------------------------
vdfSelector* vdfTemporalSelector::clone() const
{
  return new vdfTemporalSelector(this->timeStamp(), this->seekMode(),
                                 this->options());
}

//-----------------------------------------------------------------------------
vgTimeStamp vdfTemporalSelector::timeStamp() const
{
  QTE_D_CONST(vdfTemporalSelector);
  return d->TimeStamp;
}

//-----------------------------------------------------------------------------
vg::SeekMode vdfTemporalSelector::seekMode() const
{
  QTE_D_CONST(vdfTemporalSelector);
  return d->SeekMode;
}

//-----------------------------------------------------------------------------
vdfTemporalSelector::TemporalSelectionFlags vdfTemporalSelector::options() const
{
  QTE_D_CONST(vdfTemporalSelector);
  return d->Options;
}
