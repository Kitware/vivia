/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFileTrackIO.h"

#include "vpFileReader.h"
#include "vpFileTrackIOImpl.h"

//-----------------------------------------------------------------------------
vpFileTrackIO::vpFileTrackIO(vpFileReader& reader,
                             vtkVpTrackModel* trackModel,
                             TrackStorageMode storageMode,
                             bool interpolateToGround,
                             TrackTimeStampMode timeStampMode,
                             vtkVgTrackTypeRegistry* trackTypes,
                             vtkMatrix4x4* geoTransform,
                             vpFrameMap* frameMap) :
  vpTrackIO(trackModel, storageMode, interpolateToGround, timeStampMode,
            trackTypes, geoTransform, frameMap),
  Reader(reader)
{}

//-----------------------------------------------------------------------------
bool vpFileTrackIO::ReadTrackTraits()
{
  return vpFileTrackIOImpl::ReadTrackTraits(
    this, this->Reader.GetTrackTraitsFileName());
}
//-----------------------------------------------------------------------------
bool vpFileTrackIO::ReadTrackClassifiers()
{
  return vpFileTrackIOImpl::ReadTrackClassifiers(
    this, this->Reader.GetTrackClassifiersFileName());
}
