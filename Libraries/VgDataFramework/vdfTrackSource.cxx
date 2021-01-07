// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vdfTrackSource.h"

//-----------------------------------------------------------------------------
vdfTrackSourceInterface::vdfTrackSourceInterface(QObject* parent) :
  QObject(parent)
{
}

//-----------------------------------------------------------------------------
vdfTrackSourceInterface::~vdfTrackSourceInterface()
{
}

//-----------------------------------------------------------------------------
vdfTrackSource::vdfTrackSource(QObject* parent) :
  vdfTrackSourceInterface(parent)
{
}

//-----------------------------------------------------------------------------
vdfTrackSource::~vdfTrackSource()
{
}
