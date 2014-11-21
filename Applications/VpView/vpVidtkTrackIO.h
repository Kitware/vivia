/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVidtkTrackIO_h
#define __vpVidtkTrackIO_h

#include "vpTrackIO.h"

#include <tracking_data/track.h>

class vpVidtkReader;
class vtkVgTrack;

class vpVidtkTrackIO : public vpTrackIO
{
public:
  vpVidtkTrackIO(vpVidtkReader& reader,
                 vcl_map<vtkVgTrack*, vidtk::track_sptr>& trackMap,
                 vcl_map<unsigned int, vtkIdType>& sourceIdToModelIdMap,
                 vtkVgTrackModel* trackModel,
                 vpTrackIO::TrackStorageMode storageMode,
                 vpTrackIO::TrackTimeStampMode timeStampMode,
                 vtkVgTrackTypeRegistry* trackTypes = 0,
                 vtkMatrix4x4* geoTransform = 0,
                 vpFrameMap* frameMap = 0);

  virtual ~vpVidtkTrackIO();

  virtual bool ReadTracks();

  virtual bool ImportTracks(vtkIdType idsOffset, float offsetX, float offsetY);

  virtual bool WriteTracks(const char* filename, bool writeSceneElements) const;

  void UpdateTracks(const vcl_vector<vidtk::track_sptr>& tracks,
                    unsigned int updateStartFrame, unsigned int updateEndFrame);

  virtual bool GetNextValidTrackFrame(vtkVgTrack* track,
                                      unsigned int startFrame,
                                      vtkVgTimeStamp& timeStamp) const;

  virtual bool GetPrevValidTrackFrame(vtkVgTrack* track,
                                      unsigned int startFrame,
                                      vtkVgTimeStamp& timeStamp) const;

  virtual vtkIdType GetModelTrackId(unsigned int sourceId) const;

protected:
  const vpVidtkReader& GetReader() const { return this->Reader; }

  virtual unsigned int GetImageHeight() const;

private:
  void ReadTrack(const vidtk::track_sptr vidtkTrack,
                 float offsetX = 0.0f, float offsetY = 0.0f,
                 bool update = false,
                 unsigned int updateStartFrame = 0,
                 unsigned int updateEndFrame = 0,
                 vtkIdType desiredId = -1);

private:
  vpVidtkReader& Reader;
  vcl_vector<vidtk::track_sptr> Tracks;
  vcl_map<vtkVgTrack*, vidtk::track_sptr>& TrackMap;
  vcl_map<unsigned int, vtkIdType>& SourceIdToModelIdMap;
};

#endif // __vpVidtkTrackIO_h
