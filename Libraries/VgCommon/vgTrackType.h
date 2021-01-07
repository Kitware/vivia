// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
