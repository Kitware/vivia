/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVidtkFileTrackIO_h
#define __vpVidtkFileTrackIO_h

#include "vpVidtkTrackIO.h"

class vpVidtkFileReader;

class vpVidtkFileTrackIO : public vpVidtkTrackIO
{
public:
  vpVidtkFileTrackIO(vpVidtkFileReader& reader,
                     vcl_map<vtkVgTrack*, vidtk::track_sptr>& trackMap,
                     vcl_map<unsigned int, vtkIdType>& sourceIdToModelIdMap,
                     vtkVpTrackModel* trackModel,
                     TrackStorageMode storageMode,
                     TrackTimeStampMode timeStampMode,
                     vtkVgTrackTypeRegistry* trackTypes = 0,
                     vtkMatrix4x4* geoTransform = 0,
                     vpFrameMap* frameMap = 0);

  virtual bool ReadTracks();

  virtual bool ImportTracks(vtkIdType idsOffset, float offsetX, float offsetY);

  bool ReadTrackTraits();
};

#endif // __vpVidtkFileTrackIO_h
