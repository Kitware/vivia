// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgImageIcon.h"
#include "vtkJPEGReader.h"
#include "vtkObjectFactory.h"
#include "vtkTexture.h"

vtkStandardNewMacro(vtkVgImageIcon);

//-----------------------------------------------------------------------------
vtkVgImageIcon::vtkVgImageIcon()
{
  this->Visibility = 1;
  this->Offset[0] = 0;
  this->Offset[1] = 0;

  this->JPEGReader = vtkSmartPointer<vtkJPEGReader>::New();
}

//-----------------------------------------------------------------------------
vtkVgImageIcon::~vtkVgImageIcon()
{
}

//-----------------------------------------------------------------------------
void vtkVgImageIcon::SetFileName(const char* fname)
{
  this->JPEGReader->SetFileName(fname);
  this->JPEGReader->Update();
  this->SetBalloonImage(this->JPEGReader->GetOutput());
  this->Modified();
}

//----------------------------------------------------------------------
void vtkVgImageIcon::ReleaseGraphicsResources(vtkWindow* w)
{
  this->Texture->ReleaseGraphicsResources(w);
  this->Superclass::ReleaseGraphicsResources(w);
}

//-----------------------------------------------------------------------------
void vtkVgImageIcon::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
