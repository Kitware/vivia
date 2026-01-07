// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpVidtkFileEventIO_h
#define __vpVidtkFileEventIO_h

#include "vpVidtkEventIO.h"

#include "vpFileEventReader.h"

class vpVidtkFileReader;

class vpVidtkFileEventIO : public vpVidtkEventIO
{
public:
  vpVidtkFileEventIO(vpVidtkFileReader& reader,
                     std::map<vtkVgEvent*, vidtk::event_sptr>& eventMap,
                     std::map<unsigned int, vtkIdType>& sourceEventIdToModelIdMap,
                     const std::map<vtkVgTrack*, vidtk::track_sptr>& trackMap,
                     const std::map<unsigned int, vtkIdType>& sourceTrackIdToModelIdMap,
                     vtkVgEventModel* eventModel,
                     vtkVgEventTypeRegistry* eventTypes = 0);

  bool ReadEventLinks();

protected:
  vpFileEventReader FileReader;
};

#endif // __vpVidtkFileEventIO_h
