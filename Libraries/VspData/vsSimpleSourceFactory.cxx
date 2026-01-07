// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsSimpleSourceFactory.h"

// Need these so that QSharedPointer can find their dtors
#include "vsVideoSource.h"
#include "vsTrackSource.h"
#include "vsDescriptorSource.h"

//-----------------------------------------------------------------------------
vsSimpleSourceFactory::vsSimpleSourceFactory()
{
}

//-----------------------------------------------------------------------------
vsSimpleSourceFactory::~vsSimpleSourceFactory()
{
}

//-----------------------------------------------------------------------------
vsVideoSourcePtr vsSimpleSourceFactory::videoSource() const
{
  return vsVideoSourcePtr();
}

//-----------------------------------------------------------------------------
QList<vsTrackSourcePtr> vsSimpleSourceFactory::trackSources() const
{
  return QList<vsTrackSourcePtr>();
}

//-----------------------------------------------------------------------------
QList<vsDescriptorSourcePtr> vsSimpleSourceFactory::descriptorSources() const
{
  return QList<vsDescriptorSourcePtr>();
}
