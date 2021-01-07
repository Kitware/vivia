// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
