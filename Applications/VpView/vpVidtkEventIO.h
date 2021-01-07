// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
                 std::map<vtkVgEvent*, vidtk::event_sptr>& eventMap,
                 std::map<unsigned int, vtkIdType>& sourceEventIdToModelIdMap,
                 const std::map<vtkVgTrack*, vidtk::track_sptr>& trackMap,
                 const std::map<unsigned int, vtkIdType>& sourceTrackIdToModelIdMap,
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
  std::vector<vidtk::event_sptr> Events;
  std::map<vtkVgEvent*, vidtk::event_sptr>& EventMap;
  std::map<unsigned int, vtkIdType>& SourceEventIdToModelIdMap;
  const std::map<vtkVgTrack*, vidtk::track_sptr>& TrackMap;
  const std::map<unsigned int, vtkIdType>& SourceTrackIdToModelIdMap;
};

#endif // __vpVidtkEventIO_h
