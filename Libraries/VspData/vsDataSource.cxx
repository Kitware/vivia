/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsDataSource.h"

//-----------------------------------------------------------------------------
vsDataSource::vsDataSource()
{
}

//-----------------------------------------------------------------------------
vsDataSource::~vsDataSource()
{
}

//-----------------------------------------------------------------------------
void vsDataSource::start()
{
  // NOTE: The implementation should accept ::start() being called more than
  //       once!
}

//-----------------------------------------------------------------------------
vsDataSource::Status vsDataSource::status() const
{
  return NoSource;
}

//-----------------------------------------------------------------------------
QString vsDataSource::text() const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString vsDataSource::toolTip() const
{
  return QString();
}
