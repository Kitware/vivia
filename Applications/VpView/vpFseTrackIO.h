/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFseTrackIO_h
#define __vpFseTrackIO_h

#include "vpTrackIO.h"

#include <string>

class vtkVgTrack;

class vpFseTrackIO : public vpTrackIO
{
public:
  vpFseTrackIO(vtkVgTrackModel* trackModel,
               vpTrackIO::TrackStorageMode storageMode,
               vpTrackIO::TrackTimeStampMode timeStampMode,
               vtkVgTrackTypeRegistry* trackTypes = 0,
               vtkMatrix4x4* geoTransform = 0,
               vpFrameMap* frameMap = 0);

  virtual ~vpFseTrackIO();

  void SetTracksFileName(const char* tracksFileName)
    { this->TracksFileName = tracksFileName; }

  void SetImageHeight(unsigned int imageHeight)
    {
    this->ImageHeight = imageHeight;
    }

  unsigned int GetImageHeight() const
    {
    return this->ImageHeight;
    }

  virtual bool ReadTracks();
  virtual bool ImportTracks(vtkIdType idsOffset, float offsetX, float offsetY);

  virtual bool WriteTracks(const char* filename, bool writeSceneElements) const;

private:
  std::string TracksFileName;
  unsigned int ImageHeight;
};

#endif // __vpFseTrackIO_h
