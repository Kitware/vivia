/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFileTrackIO_h
#define __vpFileTrackIO_h

#include "vpTrackIO.h"

class vpFileReader;

class vpFileTrackIO : public vpTrackIO
{
public:
  vpFileTrackIO(vpFileReader& reader,
                vtkVpTrackModel* trackModel,
                TrackStorageMode storageMode,
                TrackTimeStampMode timeStampMode,
                vtkVgTrackTypeRegistry* trackTypes = nullptr,
                vtkMatrix4x4* geoTransform = nullptr,
                vpFileDataSource* imageDataSource = nullptr,
                vpFrameMap* frameMap = nullptr);

  bool ReadTrackTraits();

protected:
  vpFileReader& Reader;
};

#endif // __vpFileTrackIO_h
