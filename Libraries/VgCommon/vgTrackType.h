/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgTrackType_h
#define __vgTrackType_h

#include "vgEntityType.h"

class vgTrackType : public vgEntityType
{
public:
  // For track types, the id is the just the label string
  void SetId(const char* id) { this->SetName(id); }
  const char* GetId() const  { return this->GetName(); }
};

#endif // __vgTrackType_h
