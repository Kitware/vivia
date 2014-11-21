/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVgAssignCoordinatesLayoutStrategy.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkVgAssignCoordinatesLayoutStrategy.h"

#include "vtkVgAssignCoordinates.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkTree.h"

vtkStandardNewMacro(vtkVgAssignCoordinatesLayoutStrategy);

vtkVgAssignCoordinatesLayoutStrategy::vtkVgAssignCoordinatesLayoutStrategy()
{
  this->AssignCoordinates = vtkSmartPointer<vtkVgAssignCoordinates>::New();
}

vtkVgAssignCoordinatesLayoutStrategy::~vtkVgAssignCoordinatesLayoutStrategy()
{
}

void vtkVgAssignCoordinatesLayoutStrategy::SetXYZCoordArrayName(const char* name)
{
  this->AssignCoordinates->SetXYZCoordArrayName(name);
}

const char* vtkVgAssignCoordinatesLayoutStrategy::GetXYZCoordArrayName()
{
  return this->AssignCoordinates->GetXYZCoordArrayName();
}

void vtkVgAssignCoordinatesLayoutStrategy::Layout()
{
  this->AssignCoordinates->SetInputData(this->Graph);
  this->AssignCoordinates->Update();
  this->Graph->ShallowCopy(this->AssignCoordinates->GetOutput());
}

void vtkVgAssignCoordinatesLayoutStrategy::SetJitter(bool enable)
{
  this->AssignCoordinates->SetJitter(enable);
}

void vtkVgAssignCoordinatesLayoutStrategy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
