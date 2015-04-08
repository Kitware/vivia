/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
