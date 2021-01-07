// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpVidtkIO_h
#define __vpVidtkIO_h

#include "vpModelIO.h"

#include <event_detectors/event.h>
#include <tracking_data/track.h>

class vpVidtkReader;

class vtkVgTrack;
class vtkVgEvent;

class vgAttributeSet;

class vpVidtkIO : public vpModelIO
{
public:
  virtual ~vpVidtkIO();

  virtual void SetTrackModel(vtkVpTrackModel* trackModel,
                             vpTrackIO::TrackStorageMode storageMode,
                             bool interpolateToGround,
                             vpTrackIO::TrackTimeStampMode timeStampMode,
                             vtkVgTrackTypeRegistry* trackTypes,
                             vgAttributeSet* trackAttributes,
                             vtkMatrix4x4* geoTransform,
                             vpFrameMap* frameMap);

  virtual void SetEventModel(vtkVgEventModel* eventModel,
                             vtkVgEventTypeRegistry* eventTypes);

  virtual void SetActivityModel(vtkVgActivityManager* activityManager,
                                vpActivityConfig* activityConfig);

  virtual void SetImageHeight(unsigned int imageHeight);
  virtual unsigned int GetImageHeight() const;

  void UpdateTracks(const std::vector<vidtk::track_sptr>& tracks,
                    unsigned int updateStartFrame, unsigned int updateEndFrame);

private:
  virtual vpVidtkReader& GetReader() = 0;
  virtual const vpVidtkReader& GetReader() const = 0;

protected:
  std::map<vtkVgTrack*, vidtk::track_sptr> TrackMap;
  std::map<vtkVgEvent*, vidtk::event_sptr> EventMap;
  std::map<unsigned int, vtkIdType> SourceTrackIdToModelIdMap;
  std::map<unsigned int, vtkIdType> SourceEventIdToModelIdMap;
};

#endif // __vpVidtkIO_h
