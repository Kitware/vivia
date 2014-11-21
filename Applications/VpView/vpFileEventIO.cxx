/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFileEventIO.h"

#include "vpFileEventIOImpl.h"
#include "vpFileReader.h"

//-----------------------------------------------------------------------------
vpFileEventIO::vpFileEventIO(vpFileReader& reader,
                             vtkVgEventModel* eventModel,
                             vtkVgEventTypeRegistry* eventTypes) :
  vpEventIO(eventModel, eventTypes), Reader(reader)
{}

//-----------------------------------------------------------------------------
bool vpFileEventIO::ReadEventLinks()
{
  return vpFileEventIOImpl::ReadEventLinks(this,
                                           this->Reader.GetEventLinksFileName());
}
