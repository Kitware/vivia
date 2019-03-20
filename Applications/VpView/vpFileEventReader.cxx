/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFileEventReader.h"

#include "vpEventIO.h"

#include <vtkVgEventModel.h>

#include <qtStlUtil.h>

//-----------------------------------------------------------------------------
vpFileEventReader::vpFileEventReader(vpEventIO* io) : IO{io}
{
}

//-----------------------------------------------------------------------------
bool vpFileEventReader::ReadEventLinks(
  const QString& eventLinksFileName) const
{
  std::ifstream file(stdString(eventLinksFileName));
  if (!file)
    {
    return false;
    }

  EventLink link;
  while (file >> link.Source >> link.Destination >> link.Probability)
    {
    // convert from log probability
    link.Probability = exp(link.Probability);
    this->IO->EventModel->AddEventLink(link);
    }

  return true;
}
