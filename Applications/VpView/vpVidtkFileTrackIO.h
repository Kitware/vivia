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
                     vtkVgTrackTypeRegistry* trackTypes = 0,
                     vgAttributeSet* trackAttributes = 0,
                     vtkMatrix4x4* geoTransform = 0,
                     vpFrameMap* frameMap = 0);

  virtual bool ReadTracks(int frameOffset);

  virtual bool ImportTracks(int frameOffset, vtkIdType idsOffset,
                            float offsetX, float offsetY);

  bool ReadTrackTraits();
  bool ReadTrackClassifiers();

protected:
  vpFileTrackReader FileReader;
};

#endif // __vpVidtkFileTrackIO_h
