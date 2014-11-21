/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVidtkEventIO_h
#define __vpVidtkEventIO_h

#include "vpEventIO.h"

#include <vtkType.h>

#include <event_detectors/event.h>
#include <tracking_data/track.h>

class vpVidtkReader;
class vtkVgEvent;
class vtkVgEventBase;
class vtkVgTrack;

class vpVidtkEventIO : public vpEventIO
{
public:
  vpVidtkEventIO(vpVidtkReader& reader,
                 vcl_map<vtkVgEvent*, vidtk::event_sptr>& eventMap,
                 vcl_map<unsigned int, vtkIdType>& sourceEventIdToModelIdMap,
                 const vcl_map<vtkVgTrack*, vidtk::track_sptr>& trackMap,
                 const vcl_map<unsigned int, vtkIdType>& sourceTrackIdToModelIdMap,
                 vtkVgEventModel* eventModel,
                 vtkVgEventTypeRegistry* eventTypes = 0);

  ~vpVidtkEventIO();

  virtual bool ReadEvents();

  virtual bool ImportEvents(vtkIdType idsOffset, float offsetX, float offsetY);

  virtual bool WriteEvents(const char* filename) const;

protected:
  const vpVidtkReader& GetReader() const { return this->Reader; }

private:
  bool SetupEvent(const vidtk::event_sptr vidtkEvent,
                  vtkVgEvent* vgEvent);

  bool SetupNodeEvent(const vidtk::event_sptr vidtkEvent,
                      vtkVgEventBase* vgEvent, float offsetX, float offsetY);

  bool ReadEvents(vtkIdType idsOffset, float offsetX, float offsetY);

private:
  vpVidtkReader& Reader;
  vcl_vector<vidtk::event_sptr> Events;
  vcl_map<vtkVgEvent*, vidtk::event_sptr>& EventMap;
  vcl_map<unsigned int, vtkIdType>& SourceEventIdToModelIdMap;
  const vcl_map<vtkVgTrack*, vidtk::track_sptr>& TrackMap;
  const vcl_map<unsigned int, vtkIdType>& SourceTrackIdToModelIdMap;
};

#endif // __vpVidtkEventIO_h
