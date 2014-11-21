/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
