// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpVidtkFileEventIO.h"

#include "vpVidtkFileReader.h"

//-----------------------------------------------------------------------------
vpVidtkFileEventIO::vpVidtkFileEventIO(
  vpVidtkFileReader& reader,
  std::map<vtkVgEvent*, vidtk::event_sptr>& eventMap,
  std::map<unsigned int, vtkIdType>& sourceEventIdToModelIdMap,
  const std::map<vtkVgTrack*, vidtk::track_sptr>& trackMap,
  const std::map<unsigned int, vtkIdType>& sourceTrackIdToModelIdMap,
  vtkVgEventModel* eventModel,
  vtkVgEventTypeRegistry* eventTypes) :
  vpVidtkEventIO{reader, eventMap, sourceEventIdToModelIdMap,
                 trackMap, sourceTrackIdToModelIdMap,
                 eventModel, eventTypes},
  FileReader{this}
{}

//-----------------------------------------------------------------------------
bool vpVidtkFileEventIO::ReadEventLinks()
{
  const auto& reader =
    static_cast<const vpVidtkFileReader&>(this->GetReader());
  return this->FileReader.ReadEventLinks(reader.GetEventLinksFileName());
}
