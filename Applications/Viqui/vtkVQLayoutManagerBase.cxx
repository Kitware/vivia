// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVQLayoutManagerBase.h"

vtkVQLayoutManagerBase::vtkVQLayoutManagerBase() : vtkObject()
{
}

vtkVQLayoutManagerBase::~vtkVQLayoutManagerBase()
{
}

void vtkVQLayoutManagerBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

const vtkVgGroupNode* vtkVQLayoutManagerBase::GetScene() const
{
}

vtkVgGroupNode* vtkVQLayoutManagerBase::GetScene()
{
}

void vtkVQLayoutManagerBase::SetScene(vtkVgGroupNode* root)
{
}
