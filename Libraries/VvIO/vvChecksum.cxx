// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <QByteArray>
#include <QDataStream>

#include <qtGlobal.h>

#include "vvChecksum.h"

//-----------------------------------------------------------------------------
QDataStream& operator<<(QDataStream& ds, const vvDescriptor& d)
{
  ds << d.ModuleName.c_str() << d.DescriptorName.c_str()
     << d.InstanceId << d.Confidence;

  foreach_iter (vvDescriptorRegionMap::const_iterator, iter, d.Region)
    {
    ds << iter->TimeStamp.FrameNumber << iter->TimeStamp.Time
       << iter->ImageRegion.TopLeft.X << iter->ImageRegion.TopLeft.Y
       << iter->ImageRegion.BottomRight.X << iter->ImageRegion.BottomRight.Y;
    }

  size_t i, j;
  for (i = 0; i < d.Values.size(); ++i)
    for (j = 0; j < d.Values[i].size(); ++j)
      ds << d.Values[i][j];

  for (i = 0; i < d.TrackIds.size(); ++i)
    ds << d.TrackIds[i].Source << d.TrackIds[i].SerialNumber;

  return ds;
}

//-----------------------------------------------------------------------------
quint16 vvChecksum(const vvDescriptor& d)
{
  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);

  stream << d;

  return qChecksum(data.constData(), data.size());
}

//-----------------------------------------------------------------------------
quint16 vvChecksum(const QList<vvDescriptor>& list)
{
  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);

  foreach (const vvDescriptor& d, list)
    stream << d;

  return qChecksum(data.constData(), data.size());
}

//-----------------------------------------------------------------------------
quint16 vvChecksum(const std::vector<vvDescriptor>& list)
{
  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);

  for (size_t n = 0; n < list.size(); ++n)
    stream << list[n];

  return qChecksum(data.constData(), data.size());
}
