/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsAdapt.h"
#include "vsAdaptTracks.h"

#include <QMap>

#include <vgImage.h>

#include <vvQueryResult.h>

#include <vvUtil.h>

#include <vtkVgAdapt.h>

#include <vsEvent.h>
#include <vsTrackInfo.h>
#include <vtkVsTrackInfo.h>

namespace // anonymous
{

//-----------------------------------------------------------------------------
void addTracks(vsEventTrackInfo& eti, const vvDescriptor& d)
{
  if (!d.Region.empty())
    {
    const vsTimeInterval tti(d.Region.begin()->TimeStamp,
                             d.Region.rbegin()->TimeStamp);
    for (size_t n = 0, k = d.TrackIds.size(); n < k; ++n)
      {
      eti[d.TrackIds[n]].insert(tti, 0);
      }
    }
}

//-----------------------------------------------------------------------------
void addRegions(
  const vvDescriptor& d, QMap<vtkVgTimeStamp, vvImageBoundingBox>& regions)
{
  foreach_iter (vvDescriptorRegionMap::const_iterator, iter, d.Region)
    {
    if (regions.contains(iter->TimeStamp))
      {
      vvImageBoundingBox& dst = regions[iter->TimeStamp];
      const vvImageBoundingBox& src = iter->ImageRegion;

      // Calculate union of existing box with current iterator box
      dst.TopLeft.X = qMin(dst.TopLeft.X, src.TopLeft.X);
      dst.TopLeft.Y = qMin(dst.TopLeft.Y, src.TopLeft.Y);
      dst.BottomRight.X = qMin(dst.BottomRight.X, src.BottomRight.X);
      dst.BottomRight.Y = qMin(dst.BottomRight.Y, src.BottomRight.Y);
      }
    else
      {
      // No box exists yet at this time, so just add the one on the iterator
      regions.insert(iter->TimeStamp, iter->ImageRegion);
      }
    }
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vsEvent vsAdapt(const vvQueryResult& result)
{
  vsEvent eventBase = vsEvent(QUuid() /* FIXME? */);

  // Add descriptor track intervals
  vsEventTrackInfo eti;
  for (size_t n = 0, k = result.Descriptors.size(); n < k; ++n)
    {
    addTracks(eti, result.Descriptors[n]);
    }
  vsAddTracks(eventBase, eti);

  if (!result.Descriptors.empty())
    {
    // Generate merged regions, using metadata descriptor if present
    QMap<vtkVgTimeStamp, vvImageBoundingBox> mergedRegions;
    const size_t k = result.Descriptors.size();
    const size_t mdi = vvUtil::findMergedRegions(result.Descriptors);
    if (mdi < k)
      {
      addRegions(result.Descriptors[mdi], mergedRegions);
      }
    else
      {
      for (size_t n = 0; n < k; ++n)
        {
        addRegions(result.Descriptors[n], mergedRegions);
        }
      }

    // Set start and end times
    if (!mergedRegions.isEmpty())
      {
      eventBase->SetStartFrame(mergedRegions.begin().key());
      eventBase->SetEndFrame((mergedRegions.end() - 1).key());
      }

    // Add merged regions
    typedef QMap<vtkVgTimeStamp, vvImageBoundingBox>::const_iterator Iterator;
    foreach_iter (Iterator, iter, mergedRegions)
      {
      const vvImageBoundingBox& box = iter.value();

      double points[8];
      points[0] = box.TopLeft.X;     points[1] = box.TopLeft.Y;
      points[2] = box.BottomRight.X; points[3] = box.TopLeft.Y;
      points[4] = box.BottomRight.X; points[5] = box.BottomRight.Y;
      points[6] = box.TopLeft.X;     points[7] = box.BottomRight.Y;
      eventBase->AddRegion(iter.key(), 4, points);
      }
    }

  if (result.UserData.Flags.testFlag(vvUserData::Starred))
    {
    eventBase->SetFlags(vtkVgEventBase::EF_Starred);
    }

  eventBase->SetName(qPrintable(QString::number(result.InstanceId)));

  if (!result.UserData.Notes.empty())
    {
    eventBase->SetNote(result.UserData.Notes.c_str());
    }

  return eventBase;
}

//-----------------------------------------------------------------------------
vvTrackId vsAdaptTrackId(unsigned int vidtkId)
{
  vvTrackId vvId(0, vidtkId);
  if (vvId.SerialNumber >= 1000000)
    {
    vvId.Source = vsTrackInfo::GroundTruthSource;
    vvId.SerialNumber -= 1000000;
    }
  return vvId;
}

//-----------------------------------------------------------------------------
bool vsExtractClassifier(const vvDescriptor& sd, QList<vsEvent>& eventList)
{
  // Check if this descriptor is a classifier
  // \TODO should be using descriptor_oracle (if available) for this check
  if (sd.DescriptorName != "classifier")
    return false;

  vsEvent event = vsEvent(QUuid() /* FIXME? */);

  // Check format of classifier vector
  if (sd.Values.size() == 1)
    {
    // Convert flat map
    for (size_t i = 0; i < sd.Values[0].size(); ++i)
      {
      double p = sd.Values[0][i];
      if (p > 0.0)
        event->AddClassifier(static_cast<int>(i), p, 0.0);
      }
    }

  // Add regions from descriptor
  foreach_iter (vvDescriptorRegionMap::const_iterator, rIter, sd.Region)
    {
    const vvImageBoundingBox r = rIter->ImageRegion;
    double points[8];
    points[0] = r.TopLeft.X;     points[1] = r.TopLeft.Y;
    points[2] = r.BottomRight.X; points[3] = r.TopLeft.Y;
    points[4] = r.BottomRight.X; points[5] = r.BottomRight.Y;
    points[6] = r.TopLeft.X;     points[7] = r.BottomRight.Y;
    event->AddRegion(rIter->TimeStamp, 4, points);
    }

  // Get start and end frame
  vsEventTrackInfo eti;
  if (sd.Region.size())
    {
    vsTimeInterval tti(sd.Region.begin()->TimeStamp,
                       sd.Region.rbegin()->TimeStamp);

    // Add track(s)
    for (size_t n = 0, k = sd.TrackIds.size(); n < k; ++n)
      eti[sd.TrackIds[n]].insert(tti, 0);
    }

  // Add track interval(s) from descriptor to event
  vsAddTracks(event, eti);

  // Done
  eventList.append(event);
  return true;
}

//-----------------------------------------------------------------------------
void vsAddTracks(vsEvent& event, const vsEventTrackInfo& eti)
{
  foreach_iter (vsEventTrackInfo::const_iterator, iter, eti)
    {
    QMap<vsTimeInterval, void*> tti = iter.value();
    while (tti.count())
      {
      // Take first interval
      vsTimeInterval i = tti.begin().key();
      tti.erase(tti.begin());

      // Test for overlapping intervals and grow interval accordingly
      while (tti.count())
        {
        if (i.upper < tti.begin().key().lower)
          break;
        i.upper = tti.begin().key().upper;
        tti.erase(tti.begin());
        }

      // When no more overlapping intervals, add interval to event
      vtkVsTrackInfo* ti = new vtkVsTrackInfo(iter.key(), i.lower, i.upper);
      event->AddTrack(ti);
      }
    }
}
