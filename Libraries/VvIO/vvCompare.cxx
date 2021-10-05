/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <qglobal.h>

#include <vvDescriptor.h>

//-----------------------------------------------------------------------------
bool operator==(const vvDescriptorRegionEntry& a,
                const vvDescriptorRegionEntry& b)
{
  return a.ImageRegion.TopLeft.X == b.ImageRegion.TopLeft.X &&
         a.ImageRegion.TopLeft.Y == b.ImageRegion.TopLeft.Y &&
         a.ImageRegion.BottomRight.X == b.ImageRegion.BottomRight.X &&
         a.ImageRegion.BottomRight.Y == b.ImageRegion.BottomRight.Y &&
         qFuzzyCompare(a.TimeStamp.Time, b.TimeStamp.Time) &&
         (!(a.TimeStamp.HasFrameNumber() && b.TimeStamp.HasFrameNumber())
          || (a.TimeStamp.FrameNumber == b.TimeStamp.FrameNumber));
}

//-----------------------------------------------------------------------------
bool vvDescriptor::compare(const vvDescriptor& a, const vvDescriptor& b,
                           vvDescriptor::CompareMode mode)
{
  if (mode == vvDescriptor::CompareDetection)
    {
    // Compare detections, rather than exact match... i.e. the track ID's and
    // instance ID are allowed to be different, but the name, module, region
    // and values must match
    return a.DescriptorName == b.DescriptorName &&
           a.ModuleName == b.ModuleName &&
           a.Confidence == b.Confidence &&
           a.Region == b.Region &&
           a.Values == b.Values &&
           a.TrackIds.size() == b.TrackIds.size();
    }
  else
    {
    // Compare exactly
    return a.InstanceId == b.InstanceId &&
           a.DescriptorName == b.DescriptorName &&
           a.ModuleName == b.ModuleName &&
           a.Confidence == b.Confidence &&
           a.Region == b.Region &&
           a.Values == b.Values &&
           a.TrackIds == b.TrackIds;
    }
}
