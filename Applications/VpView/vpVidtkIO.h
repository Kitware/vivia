/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVidtkIO_h
#define __vpVidtkIO_h

#include "vpModelIO.h"

#include <event_detectors/event.h>
#include <tracking_data/track.h>

class vpVidtkReader;

class vtkVgTrack;
class vtkVgEvent;

class vpVidtkIO : public vpModelIO
{
public:
  virtual ~vpVidtkIO();

  virtual void SetTrackModel(vtkVgTrackModel* trackModel,
                             vpTrackIO::TrackStorageMode storageMode,
                             vpTrackIO::TrackTimeStampMode timeStampMode,
                             vtkVgTrackTypeRegistry* trackTypes,
                             vtkMatrix4x4* geoTransform,
                             vpFrameMap* frameMap);

  virtual void SetEventModel(vtkVgEventModel* eventModel,
                             vtkVgEventTypeRegistry* eventTypes);

  virtual void SetActivityModel(vtkVgActivityManager* activityManager,
                                vpActivityConfig* activityConfig);

  virtual void SetImageHeight(unsigned int imageHeight);
  virtual unsigned int GetImageHeight() const;

  void UpdateTracks(const vcl_vector<vidtk::track_sptr>& tracks,
                    unsigned int updateStartFrame, unsigned int updateEndFrame);

private:
  virtual vpVidtkReader& GetReader() = 0;
  virtual const vpVidtkReader& GetReader() const = 0;

protected:
  vcl_map<vtkVgTrack*, vidtk::track_sptr> TrackMap;
  vcl_map<vtkVgEvent*, vidtk::event_sptr> EventMap;
  vcl_map<unsigned int, vtkIdType> SourceTrackIdToModelIdMap;
  vcl_map<unsigned int, vtkIdType> SourceEventIdToModelIdMap;
};

#endif // __vpVidtkIO_h
