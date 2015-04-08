/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfTrackId.h"

#include <QHash>

//-----------------------------------------------------------------------------
vdfTrackId::vdfTrackId() : Source(0), Provider(-1), SerialNumber(-1)
{
}

//-----------------------------------------------------------------------------
vdfTrackId::vdfTrackId(vdfDataSource* source, long long int serialNumber,
                       const QUuid& uniqueId) :
    Source(source),
    UniqueId(uniqueId),
    Provider(-1),
    SerialNumber(serialNumber)
{
}

//-----------------------------------------------------------------------------
vdfTrackId::vdfTrackId(vdfDataSource* source, int provider,
                       long long int serialNumber, const QUuid& uniqueId) :
    Source(source),
    UniqueId(uniqueId),
    Provider(provider),
    SerialNumber(serialNumber)
{
}

//-----------------------------------------------------------------------------
bool vdfTrackId::operator==(const vdfTrackId& other) const
{
  return this->SerialNumber == other.SerialNumber &&
         this->Provider == other.Provider &&
         this->Source == other.Source &&
         this->UniqueId == other.UniqueId;
}

//-----------------------------------------------------------------------------
uint qHash(const vdfTrackId& tid)
{
  return qHash(tid.Source) ^ qHash(tid.Provider) ^ qHash(tid.SerialNumber);
}
