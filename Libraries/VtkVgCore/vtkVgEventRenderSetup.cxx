/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgEventRenderSetup.h"

vtkStandardNewMacro(vtkVgEventRenderSetup);

//-----------------------------------------------------------------------------
class vtkVgEventRenderSetup::vtkInternal
{
  vtkInternal();
  ~vtkInternal();
};

//-----------------------------------------------------------------------------
vtkVgEventRenderSetup::vtkInternal::vtkInternal()
{
}

//-----------------------------------------------------------------------------
vtkVgEventRenderSetup::vtkInternal::~vtkInternal()
{
}

//-----------------------------------------------------------------------------
vtkVgEventRenderSetup::vtkVgEventRenderSetup() : vtkObject()
{
  this->Internal = new vtkInternal();
}

//-----------------------------------------------------------------------------
vtkVgEventRenderSetup::~vtkVgEventRenderSetup()
{
  if (this->Internal)
    {
    delete this->Internal;
    this->Internal = 0x0;
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventRenderSetup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgEventRenderSetup::SetColor(int type, const double& r, const double& g,
                                     const double& b)
{
  // @TODO: Implement this.
}

//-----------------------------------------------------------------------------
void vtkVgEventRenderSetup::GetColor(int type, double& r, double& g, double& b)
{
  // @TODO: Implement this.
}
