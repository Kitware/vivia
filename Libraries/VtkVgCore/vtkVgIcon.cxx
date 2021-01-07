// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgIcon.h"
#include "vtkVgEvent.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkVgIcon);

//-----------------------------------------------------------------------------
vtkVgIcon::vtkVgIcon()
{
  this->Category = vgEventTypeDefinitions::CategoryUnknown;
  this->Type = vgSpecialIconTypeDefinitions::Unknown;
  this->Visibility = 0;
  this->Position[0] = this->Position[1] = 0;
  this->Scale = 1.0;
}

//-----------------------------------------------------------------------------
void vtkVgIcon::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
