/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvDescriptor_h
#define __vvDescriptor_h

#include <vector>

#include <vgExport.h>

#include <vgTimeStamp.h>

#include "vvTrack.h"

// Disable warning about the STL members of vvDescriptor not being exported.
// This is not 'fixable', since classes from MS STL can't be exported, with the
// possible exception of vector (see http://support.microsoft.com/kb/168958).
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif

//-----------------------------------------------------------------------------
struct vvDescriptorRegionEntry
{
  vgTimeStamp TimeStamp;
  vvImageBoundingBox ImageRegion;

  bool operator<(const vvDescriptorRegionEntry& other) const
    { return this->TimeStamp < other.TimeStamp; }
};

typedef std::set<vvDescriptorRegionEntry> vvDescriptorRegionMap;

//-----------------------------------------------------------------------------
struct vvDescriptor
{
  vvDescriptor() : InstanceId(-1), Confidence(-1.0) {}

  std::string DescriptorName;
  std::string ModuleName;
  long long InstanceId;
  double Confidence;
  std::vector<std::vector<float> > Values;
  vvDescriptorRegionMap Region;
  std::vector<vvTrackId> TrackIds;

  enum CompareMode
    {
    CompareExact,
    CompareDetection
    };

  // NOTE: implemented in vvIO
  VV_IO_EXPORT static bool compare(const vvDescriptor&, const vvDescriptor&,
                                   CompareMode = CompareExact);
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
