// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
  friend class vpFileEventReader;

  vtkVgEventModel* EventModel;
  vtkVgEventTypeRegistry* EventTypes;
};

#endif // __vpEventIO_h
