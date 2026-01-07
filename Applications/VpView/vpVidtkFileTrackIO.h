// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
