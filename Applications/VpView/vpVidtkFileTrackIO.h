/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVidtkFileTrackIO_h
#define __vpVidtkFileTrackIO_h

#include "vpVidtkTrackIO.h"

#include "vpFileTrackReader.h"

class vpVidtkFileReader;

class vpVidtkFileTrackIO : public vpVidtkTrackIO
{
public:
  vpVidtkFileTrackIO(vpVidtkFileReader& reader,
                     std::map<vtkVgTrack*, vidtk::track_sptr>& trackMap,
                     std::map<unsigned int, vtkIdType>& sourceIdToModelIdMap,
                     vtkVpTrackModel* trackModel,
                     TrackStorageMode storageMode,
                     bool interpolateToGround,
                     TrackTimeStampMode timeStampMode,
                     vtkVgTrackTypeRegistry* trackTypes = nullptr,
                     vgAttributeSet* trackAttributes = nullptr,
                     vtkMatrix4x4* geoTransform = nullptr,
                     vpFileDataSource* imageDataSource = nullptr,
                     vpFrameMap* frameMap = nullptr);

  virtual bool ReadTracks(int frameOffset);

  virtual bool ImportTracks(int frameOffset, vtkIdType idsOffset,
                            float offsetX, float offsetY);

  bool ReadTrackTraits();
  bool ReadTrackClassifiers();

protected:
  vpFileTrackReader FileReader;
};

#endif // __vpVidtkFileTrackIO_h
