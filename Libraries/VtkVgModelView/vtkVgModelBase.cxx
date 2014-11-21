/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgModelBase.h"

// VTK includes
#include <vtkMatrix4x4.h>

// VG includes
#include "vtkVgContourOperatorManager.h"
#include "vtkVgTemporalFilters.h"

vtkCxxSetObjectMacro(vtkVgModelBase, ContourOperatorManager,
                     vtkVgContourOperatorManager);

vtkCxxSetObjectMacro(vtkVgModelBase, TemporalFilters,
                     vtkVgTemporalFilters);

//-----------------------------------------------------------------------------
vtkVgModelBase::vtkVgModelBase() : vtkObject(),
  Id(-1),
  UseInternalTimeStamp(0),
  Initialized(0),
  ContourOperatorManager(0),
  TemporalFilters(0)
{
  this->ModelMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
}

//-----------------------------------------------------------------------------
vtkVgModelBase::~vtkVgModelBase()
{
  this->SetContourOperatorManager(0);
  this->SetTemporalFilters(0);
}

//-----------------------------------------------------------------------------
void vtkVgModelBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgModelBase::SetModelMatrix(vtkMatrix4x4* matrix)
{
  if (!matrix || (this->ModelMatrix == matrix))
    {
    return;
    }

  this->ModelMatrix = matrix;
  this->UpdateDataRequestTime.Modified();
}

//-----------------------------------------------------------------------------
const vtkMatrix4x4* vtkVgModelBase::GetModelMatrix() const
{
  return this->ModelMatrix;
}

//-----------------------------------------------------------------------------
vtkMatrix4x4* vtkVgModelBase::GetModelMatrix()
{
  return this->ModelMatrix;
}
