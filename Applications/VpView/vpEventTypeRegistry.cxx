/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpEventTypeRegistry.h"

#if defined(VISGUI_USE_VIDTK) && 0
#include <event_detectors/event_types.h>
#endif

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vpEventTypeRegistry);

//-----------------------------------------------------------------------------
void vpEventTypeRegistry::WarnTypeNotFound(int id)
{
#if defined(VISGUI_USE_VIDTK) && 0
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
