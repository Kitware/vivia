/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
