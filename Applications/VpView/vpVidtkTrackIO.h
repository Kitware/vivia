/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVidtkTrackIO_h
#define __vpVidtkTrackIO_h

#include "vpTrackIO.h"

#include "vpFileTrackReader.h"

#include <tracking_data/track.h>

class vpVidtkReader;
class vtkVgTrack;
class vgAttributeSet;

class vpVidtkTrackIO : public vpTrackIO
{
public:
  vpVidtkTrackIO(vpVidtkReader& reader,
                 std::map<vtkVgTrack*, vidtk::track_sptr>& trackMap,
                 std::map<unsigned int, vtkIdType>& sourceIdToModelIdMap,
                 vtkVpTrackModel* trackModel,
                 vpTrackIO::TrackStorageMode storageMode,
                 bool interpolateToGround,
                 vpTrackIO::TrackTimeStampMode timeStampMode,
                 vtkVgTrackTypeRegistry* trackTypes = 0,
                 vgAttributeSet* trackAttributes = 0,
                 vtkMatrix4x4* geoTransform = 0,
                 vpFrameMap* frameMap = 0);

  virtual ~vpVidtkTrackIO();

  virtual bool ReadTracks(int frameOffset);

  virtual bool ImportTracks(int frameOffset, vtkIdType idsOffset,
                            float offsetX, float offsetY);

  virtual bool WriteTracks(const char* filename, int frameOffset,
                           QPointF aoiOffset, bool writeSceneElements) const;

  void UpdateTracks(const std::vector<vidtk::track_sptr>& tracks,
                    unsigned int updateStartFrame, unsigned int updateEndFrame);

  virtual bool GetNextValidTrackFrame(vtkVgTrack* track,
                                      unsigned int startFrame,
                                      vtkVgTimeStamp& timeStamp) const;

  virtual bool GetPrevValidTrackFrame(vtkVgTrack* track,
                                      unsigned int startFrame,
                                      vtkVgTimeStamp& timeStamp) const;

  virtual vtkIdType GetModelTrackId(unsigned int sourceId) const;

protected:
  bool ReadTracks(int frameOffset,
                  const vpFileTrackReader::TrackRegionMap* trackRegionMap);

  bool ImportTracks(const vpFileTrackReader::TrackRegionMap* trackRegionMap,
                    int frameOffset, vtkIdType idsOffset,
                    float offsetX, float offsetY);

  const vpVidtkReader& GetReader() const { return this->Reader; }

  virtual unsigned int GetImageHeight() const;

  vgAttributeSet* TrackAttributes;

private:
  vtkIdType ComputeNumberOfPoints(
    const vpFileTrackReader::TrackRegionMap* trackRegionMap);

  void ReadTrack(const vidtk::track_sptr vidtkTrack, int frameOffset,
                 const vpFileTrackReader::TrackRegionMap* trackRegionMap,
                 float offsetX = 0.0f, float offsetY = 0.0f,
                 bool update = false,
                 unsigned int updateStartFrame = 0,
                 unsigned int updateEndFrame = 0,
                 vtkIdType desiredId = -1);

private:
  vpVidtkReader& Reader;
  std::vector<vidtk::track_sptr> Tracks;
  std::map<vtkVgTrack*, vidtk::track_sptr>& TrackMap;
  std::map<unsigned int, vtkIdType>& SourceIdToModelIdMap;
};

#endif // __vpVidtkTrackIO_h
