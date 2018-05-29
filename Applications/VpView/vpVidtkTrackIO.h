/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVidtkTrackIO_h
#define __vpVidtkTrackIO_h

#include "vpTrackIO.h"

#include "vpFileTrackIOImpl.h"

#include <tracking_data/track.h>

class vpVidtkReader;
class vtkVgTrack;

class vpVidtkTrackIO : public vpTrackIO
{
public:
  vpVidtkTrackIO(vpVidtkReader& reader,
                 std::map<vtkVgTrack*, vidtk::track_sptr>& trackMap,
                 std::map<unsigned int, vtkIdType>& sourceIdToModelIdMap,
                 vtkVpTrackModel* trackModel,
                 vpTrackIO::TrackStorageMode storageMode,
                 vpTrackIO::TrackTimeStampMode timeStampMode,
                 vtkVgTrackTypeRegistry* trackTypes = 0,
                 vtkMatrix4x4* geoTransform = 0,
                 vpFrameMap* frameMap = 0);

  virtual ~vpVidtkTrackIO();

  virtual bool ReadTracks();

  virtual bool ImportTracks(vtkIdType idsOffset, float offsetX, float offsetY);

  virtual bool WriteTracks(const QString& filename,
                           bool writeSceneElements) const;

  virtual QStringList GetSupportedFormats() const;
  virtual QString GetDefaultFormat() const;

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
  bool ReadTracks(const vpFileTrackIOImpl::TrackRegionMap* trackRegionMap);

  bool ImportTracks(const vpFileTrackIOImpl::TrackRegionMap* trackRegionMap,
                    vtkIdType idsOffset, float offsetX, float offsetY);

  const vpVidtkReader& GetReader() const { return this->Reader; }

  virtual unsigned int GetImageHeight() const;

private:
  vtkIdType ComputeNumberOfPoints(
    const vpFileTrackIOImpl::TrackRegionMap* trackRegionMap);
  void ReadTrack(const vidtk::track_sptr vidtkTrack,
                 const vpFileTrackIOImpl::TrackRegionMap* trackRegionMap,
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
