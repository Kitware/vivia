// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpEventTypeRegistry.h"

#ifdef VISGUI_USE_VIDTK
#include <event_detectors/event_types.h>
#endif

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vpEventTypeRegistry);

//-----------------------------------------------------------------------------
void vpEventTypeRegistry::WarnTypeNotFound(int id)
{
#ifdef VISGUI_USE_VIDTK
  if (id < static_cast<int>(vidtk::event_types::events_size))
    {
    vtkErrorMacro("Event type id is not registered: " <<
                  vidtk::event_types::event_names[id]);
    }
  else
#endif
    {
    this->Superclass::WarnTypeNotFound(id);
    }
}
