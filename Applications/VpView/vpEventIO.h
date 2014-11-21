/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpEventIO_h
#define __vpEventIO_h

#include <vtkType.h>

class vtkVgEventModel;
class vtkVgEventTypeRegistry;

class vpEventIO
{
public:
  vpEventIO(vtkVgEventModel* eventModel,
            vtkVgEventTypeRegistry* eventTypes);

  virtual ~vpEventIO() {}

  virtual bool ReadEvents() = 0;
  virtual bool ReadEventLinks();

  virtual bool ImportEvents(vtkIdType idsOffset, float offsetX, float offsetY);

  virtual bool WriteEvents(const char* filename) const = 0;

protected:
  friend class vpFileEventIOImpl;

  vtkVgEventModel* EventModel;
  vtkVgEventTypeRegistry* EventTypes;
};

#endif // __vpEventIO_h
