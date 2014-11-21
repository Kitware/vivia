/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
