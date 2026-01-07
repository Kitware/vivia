// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
