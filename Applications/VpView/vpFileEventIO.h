/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFileEventIO_h
#define __vpFileEventIO_h

#include "vpEventIO.h"

class vpFileReader;

class vpFileEventIO : public vpEventIO
{
public:
  vpFileEventIO(vpFileReader& reader,
                vtkVgEventModel* eventModel,
                vtkVgEventTypeRegistry* eventTypes = 0);

  bool ReadEventLinks();

protected:
  vpFileReader& Reader;
};

#endif // __vpFileEventIO_h
