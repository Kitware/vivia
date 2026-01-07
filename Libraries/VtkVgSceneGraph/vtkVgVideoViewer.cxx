// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgVideoViewer.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkVgVideoViewer);

//-----------------------------------------------------------------------------
vtkVgVideoViewer::vtkVgVideoViewer() : vtkVgViewer()
{
}

//-----------------------------------------------------------------------------
vtkVgVideoViewer::~vtkVgVideoViewer()
{
}

//-----------------------------------------------------------------------------
void vtkVgVideoViewer::Realize()
{
  // Do nothing other than setting the state to true.
  this->Realized = true;
}

//-----------------------------------------------------------------------------
void vtkVgVideoViewer::Run()
{
  if (!this->IsRealized())
    {
    this->Realize();
    }
}
