/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
