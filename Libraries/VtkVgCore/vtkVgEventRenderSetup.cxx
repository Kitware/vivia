// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
