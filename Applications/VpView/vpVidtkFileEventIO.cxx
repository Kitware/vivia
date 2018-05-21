/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVidtkFileEventIO.h"

#include "vpFileEventIOImpl.h"
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
  vpVidtkEventIO(reader, eventMap, sourceEventIdToModelIdMap,
                 trackMap, sourceTrackIdToModelIdMap,
                 eventModel, eventTypes)
{}

//-----------------------------------------------------------------------------
bool vpVidtkFileEventIO::ReadEventLinks()
{
  return vpFileEventIOImpl::ReadEventLinks(
           this, static_cast<const vpVidtkFileReader&>(
                   this->GetReader()).GetEventLinksFileName());
}
