/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVgApplySelectedColor.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/
#include "vtkVgApplySelectedColor.h"

#include "vtkCellData.h"
#include "vtkGraph.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkDataSet.h"
#include "vtkSmartPointer.h"
#include "vtkTable.h"
#include "vtkUnsignedCharArray.h"

vtkStandardNewMacro(vtkVgApplySelectedColor);

//----------------------------------------------------------------------------
vtkVgApplySelectedColor::vtkVgApplySelectedColor()
{
  this->DefaultPointColor[0] = 0.0;
  this->DefaultPointColor[1] = 0.0;
  this->DefaultPointColor[2] = 0.0;
  this->DefaultPointOpacity = 1.0;
  this->DefaultCellColor[0] = 0.0;
  this->DefaultCellColor[1] = 0.0;
  this->DefaultCellColor[2] = 0.0;
  this->DefaultCellOpacity = 1.0;
  this->SelectedPointColor[0] = 1.0;
  this->SelectedPointColor[1] = 0.0;
  this->SelectedPointColor[2] = 1.0;
  this->SelectedPointOpacity = 1.0;
  this->SelectedCellColor[0] = 1.0;
  this->SelectedCellColor[1] = 0.0;
  this->SelectedCellColor[2] = 1.0;
  this->SelectedCellOpacity = 1.0;
  this->SetNumberOfInputPorts(1);

  this->SetInputArrayToProcess(0, 0, 0,
                               vtkDataObject::FIELD_ASSOCIATION_VERTICES,
                               vtkDataSetAttributes::SCALARS);
  this->SetInputArrayToProcess(1, 0, 0,
                               vtkDataObject::FIELD_ASSOCIATION_VERTICES,
                               vtkDataSetAttributes::SCALARS);
  this->SetInputArrayToProcess(2, 0, 0,
                               vtkDataObject::FIELD_ASSOCIATION_EDGES,
                               vtkDataSetAttributes::SCALARS);
  this->SetInputArrayToProcess(3, 0, 0,
                               vtkDataObject::FIELD_ASSOCIATION_EDGES,
                               vtkDataSetAttributes::SCALARS);

  this->PointColorOutputArrayName = 0;
  this->CellColorOutputArrayName = 0;
  this->SetPointColorOutputArrayName("vtkVgApplySelectedColor color");
  this->SetCellColorOutputArrayName("vtkVgApplySelectedColor color");

  this->UsePointScalars = false;
  this->UsePointSelection = false;
  this->UseCellScalars = false;
  this->UseCellSelection = false;
}

//----------------------------------------------------------------------------
vtkVgApplySelectedColor::~vtkVgApplySelectedColor()
{
  this->SetPointColorOutputArrayName(0);
  this->SetCellColorOutputArrayName(0);
}

//----------------------------------------------------------------------------
int vtkVgApplySelectedColor::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
    {
    info->Remove(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE());
    info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkGraph");
    info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTable");
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkVgApplySelectedColor::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  if (!this->PointColorOutputArrayName || !this->CellColorOutputArrayName)
    {
    vtkErrorMacro("Point and cell array names must be valid");
    return 0;
    }

  // get the input and output
  vtkDataObject* input = inInfo->Get(vtkDataObject::DATA_OBJECT());
  vtkDataObject* output = outInfo->Get(vtkDataObject::DATA_OBJECT());

  output->ShallowCopy(input);

  vtkGraph* graph = vtkGraph::SafeDownCast(output);
  vtkDataSet* dataSet = vtkDataSet::SafeDownCast(output);
  vtkTable* table = vtkTable::SafeDownCast(output);

  // initialize color arrays
  vtkSmartPointer<vtkUnsignedCharArray> colorArr1 =
    vtkSmartPointer<vtkUnsignedCharArray>::New();

  colorArr1->SetName(this->PointColorOutputArrayName);
  colorArr1->SetNumberOfComponents(4);

  if (graph)
    {
    colorArr1->SetNumberOfTuples(graph->GetNumberOfVertices());
    graph->GetVertexData()->AddArray(colorArr1);
    }
  else if (dataSet)
    {
    colorArr1->SetNumberOfTuples(dataSet->GetNumberOfPoints());
    dataSet->GetPointData()->AddArray(colorArr1);
    }
  else
    {
    colorArr1->SetNumberOfTuples(table->GetNumberOfRows());
    table->AddColumn(colorArr1);
    }

  vtkSmartPointer<vtkUnsignedCharArray> colorArr2 =
    vtkSmartPointer<vtkUnsignedCharArray>::New();

  colorArr2->SetName(this->CellColorOutputArrayName);
  colorArr2->SetNumberOfComponents(4);

  if (graph)
    {
    colorArr2->SetNumberOfTuples(graph->GetNumberOfEdges());
    graph->GetEdgeData()->AddArray(colorArr2);
    }
  else if (dataSet)
    {
    colorArr2->SetNumberOfTuples(dataSet->GetNumberOfCells());
    dataSet->GetCellData()->AddArray(colorArr2);
    }

  unsigned char pointColor[4];
  pointColor[0] = static_cast<unsigned char>(255 * this->DefaultPointColor[0]);
  pointColor[1] = static_cast<unsigned char>(255 * this->DefaultPointColor[1]);
  pointColor[2] = static_cast<unsigned char>(255 * this->DefaultPointColor[2]);
  pointColor[3] = static_cast<unsigned char>(255 * this->DefaultPointOpacity);

  unsigned char sPointColor[4];
  sPointColor[0] = static_cast<unsigned char>(255 * this->SelectedPointColor[0]);
  sPointColor[1] = static_cast<unsigned char>(255 * this->SelectedPointColor[1]);
  sPointColor[2] = static_cast<unsigned char>(255 * this->SelectedPointColor[2]);
  sPointColor[3] = static_cast<unsigned char>(255 * this->SelectedPointOpacity);

  vtkAbstractArray* selected
    = this->UsePointSelection ?
      this->GetInputAbstractArrayToProcess(0, inputVector) : 0;

  if (selected &&
      selected->GetNumberOfTuples() != colorArr1->GetNumberOfTuples())
    {
    vtkErrorMacro("Selection array size doesn't match input size");
    return 0;
    }

  vtkUnsignedCharArray* inputColors
    = this->UsePointScalars ?
      vtkUnsignedCharArray::SafeDownCast(
        this->GetInputAbstractArrayToProcess(1, inputVector)) : 0;

  if (inputColors &&
      inputColors->GetNumberOfTuples() != colorArr1->GetNumberOfTuples())
    {
    vtkErrorMacro("Input color array size doesn't match input size");
    return 0;
    }

  this->ProcessColorArray(selected, inputColors, colorArr1,
                          sPointColor, pointColor);

  unsigned char cellColor[4];
  cellColor[0] = static_cast<unsigned char>(255 * this->DefaultCellColor[0]);
  cellColor[1] = static_cast<unsigned char>(255 * this->DefaultCellColor[1]);
  cellColor[2] = static_cast<unsigned char>(255 * this->DefaultCellColor[2]);
  cellColor[3] = static_cast<unsigned char>(255 * this->DefaultCellOpacity);

  unsigned char sCellColor[4];
  sCellColor[0] = static_cast<unsigned char>(255 * this->SelectedCellColor[0]);
  sCellColor[1] = static_cast<unsigned char>(255 * this->SelectedCellColor[1]);
  sCellColor[2] = static_cast<unsigned char>(255 * this->SelectedCellColor[2]);
  sCellColor[3] = static_cast<unsigned char>(255 * this->SelectedCellOpacity);

  selected = this->UseCellSelection ?
             this->GetInputAbstractArrayToProcess(2, inputVector) : 0;

  if (selected &&
      selected->GetNumberOfTuples() != colorArr2->GetNumberOfTuples())
    {
    vtkErrorMacro("Selection array size doesn't match input size");
    return 0;
    }

  inputColors = this->UseCellScalars ?
                vtkUnsignedCharArray::SafeDownCast(
                  this->GetInputAbstractArrayToProcess(3, inputVector)) : 0;

  if (inputColors &&
      inputColors->GetNumberOfTuples() != colorArr2->GetNumberOfTuples())
    {
    vtkErrorMacro("Input color array size doesn't match input size");
    return 0;
    }

  this->ProcessColorArray(selected, inputColors, colorArr2,
                          sCellColor, cellColor);

  return 1;
}

//----------------------------------------------------------------------------
void vtkVgApplySelectedColor::ProcessColorArray(
  vtkAbstractArray* selectedArr,
  vtkUnsignedCharArray* inputColorArr,
  vtkUnsignedCharArray* outputColorArr,
  unsigned char selectedColor[4],
  unsigned char defaultColor[4])
{
  for (vtkIdType i = 0; i < outputColorArr->GetNumberOfTuples(); ++i)
    {
    if (selectedArr && selectedArr->GetVariantValue(i).ToInt() != 0)
      {
      outputColorArr->SetTupleValue(i, selectedColor);
      continue;
      }

    if (inputColorArr)
      {
      unsigned char color[4] = { 0, 0, 0, 255 };
      inputColorArr->GetTupleValue(i, color);
      outputColorArr->SetTupleValue(i, color);
      }
    else
      {
      outputColorArr->SetTupleValue(i, defaultColor);
      }
    }
}

//----------------------------------------------------------------------------
void vtkVgApplySelectedColor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "DefaultPointColor: "
     << this->DefaultPointColor[0] << ","
     << this->DefaultPointColor[1] << ","
     << this->DefaultPointColor[2] << endl;
  os << indent << "DefaultPointOpacity: " << this->DefaultPointOpacity << endl;
  os << indent << "DefaultCellColor: "
     << this->DefaultCellColor[0] << ","
     << this->DefaultCellColor[1] << ","
     << this->DefaultCellColor[2] << endl;
  os << indent << "DefaultCellOpacity: " << this->DefaultCellOpacity << endl;
  os << indent << "SelectedPointColor: "
     << this->SelectedPointColor[0] << ","
     << this->SelectedPointColor[1] << ","
     << this->SelectedPointColor[2] << endl;
  os << indent << "SelectedPointOpacity: " << this->SelectedPointOpacity << endl;
  os << indent << "SelectedCellColor: "
     << this->SelectedCellColor[0] << ","
     << this->SelectedCellColor[1] << ","
     << this->SelectedCellColor[2] << endl;
  os << indent << "SelectedCellOpacity: " << this->SelectedCellOpacity << endl;
  os << indent << "PointColorOutputArrayName: "
     << (this->PointColorOutputArrayName ? this->PointColorOutputArrayName : "(none)") << endl;
  os << indent << "CellColorOutputArrayName: "
     << (this->CellColorOutputArrayName ? this->CellColorOutputArrayName : "(none)") << endl;
}
