/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVidtkFileEventIO_h
#define __vpVidtkFileEventIO_h

#include "vpVidtkEventIO.h"

class vpVidtkFileReader;

class vpVidtkFileEventIO : public vpVidtkEventIO
{
public:
  vpVidtkFileEventIO(vpVidtkFileReader& reader,
                     vcl_map<vtkVgEvent*, vidtk::event_sptr>& eventMap,
                     vcl_map<unsigned int, vtkIdType>& sourceEventIdToModelIdMap,
                     const vcl_map<vtkVgTrack*, vidtk::track_sptr>& trackMap,
                     const vcl_map<unsigned int, vtkIdType>& sourceTrackIdToModelIdMap,
                     vtkVgEventModel* eventModel,
                     vtkVgEventTypeRegistry* eventTypes = 0);

  bool ReadEventLinks();
};

#endif // __vpVidtkFileEventIO_h
