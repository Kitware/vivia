// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
