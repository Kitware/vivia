/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFileTrackIO.h"

#include "vpFileReader.h"
#include "vpFileTrackIOImpl.h"

//-----------------------------------------------------------------------------
vpFileTrackIO::vpFileTrackIO(vpFileReader& reader,
                             vtkVgTrackModel* trackModel,
                             TrackStorageMode storageMode,
                             TrackTimeStampMode timeStampMode,
                             vtkVgTrackTypeRegistry* trackTypes,
                             vtkMatrix4x4* geoTransform,
                             vpFrameMap* frameMap) :
  vpTrackIO(trackModel, storageMode, timeStampMode, trackTypes,
            geoTransform, frameMap),
  Reader(reader)
{}

//-----------------------------------------------------------------------------
bool vpFileTrackIO::ReadTrackTraits()
{
  return vpFileTrackIOImpl::ReadTrackTraits(this,
                                            this->Reader.GetTrackTraitsFileName());
}
//-----------------------------------------------------------------------------
bool vpFileTrackIO::ReadTrackPVOs()
{
  return vpFileTrackIOImpl::ReadTrackPVOs(this,
                                          this->Reader.GetTrackPVOsFileName());
}
