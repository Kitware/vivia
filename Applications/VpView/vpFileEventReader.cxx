// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
