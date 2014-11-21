/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgDataSourceBase.h"

//-----------------------------------------------------------------------------
vtkVgDataSourceBase::vtkVgDataSourceBase() : vtkObject(),
  DataSource(NULL)
{
}

//-----------------------------------------------------------------------------
vtkVgDataSourceBase::~vtkVgDataSourceBase()
{
  this->SetDataSource(NULL);
}

//-----------------------------------------------------------------------------
void vtkVgDataSourceBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
const char* vtkVgDataSourceBase::GetDataSource() const
{
  return this->DataSource;
}

//-----------------------------------------------------------------------------
void vtkVgDataSourceBase::ShallowCopy(vtkVgDataSourceBase& other)
{
  this->SetDataSource(other.DataSource);
}

//-----------------------------------------------------------------------------
void vtkVgDataSourceBase::DeepCopy(vtkVgDataSourceBase& other)
{
  this->SetDataSource(other.DataSource);
}
