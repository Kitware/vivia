/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
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
                vtkVgTrackModel* trackModel,
                TrackStorageMode storageMode,
                TrackTimeStampMode timeStampMode,
                vtkVgTrackTypeRegistry* trackTypes = 0,
                vtkMatrix4x4* geoTransform = 0,
                vpFrameMap* frameMap = 0);

  bool ReadTrackTraits();

protected:
  vpFileReader& Reader;
};

#endif // __vpFileTrackIO_h
