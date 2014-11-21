/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFileEventIOImpl.h"

#include "vpEventIO.h"

#include <vtkVgEventModel.h>

//-----------------------------------------------------------------------------
bool vpFileEventIOImpl::ReadEventLinks(vpEventIO* io,
                                       const std::string& eventLinksFileName)
{
  std::ifstream file(eventLinksFileName.c_str());
  if (!file)
    {
    return false;
    }

  EventLink link;
  while (file >> link.Source >> link.Destination >> link.Probability)
    {
    // convert from log probability
    link.Probability = exp(link.Probability);
    io->EventModel->AddEventLink(link);
    }

  return true;
}
