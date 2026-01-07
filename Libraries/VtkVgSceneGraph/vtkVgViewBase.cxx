// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgViewBase.h"

// VTK includes.
#include <vtkCamera.h>
#include <vtkObjectFactory.h>

// VG includes.
#include "vtkVgGroupNode.h"
#include "vtkVgSceneManager.h"

vtkStandardNewMacro(vtkVgViewBase);

//-----------------------------------------------------------------------------
vtkVgViewBase::vtkVgViewBase() : vtkVgSceneManager()
{
}

//-----------------------------------------------------------------------------
vtkVgViewBase::~vtkVgViewBase()
{
}

//-----------------------------------------------------------------------------
void vtkVgViewBase::PrintSelf(ostream& vtkNotUsed(os),
                              vtkIndent vtkNotUsed(indent))
{
}

//-----------------------------------------------------------------------------
void vtkVgViewBase::Update(const vtkVgTimeStamp& timestamp)
{
  this->Superclass::Update(timestamp);
}
