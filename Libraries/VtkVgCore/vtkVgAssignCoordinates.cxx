/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVgAssignCoordinates.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVgAssignCoordinates.h"

#include "vtkGraph.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPoints.h"

vtkStandardNewMacro(vtkVgAssignCoordinates);

vtkVgAssignCoordinates::vtkVgAssignCoordinates()
{
  this->XYZCoordArrayName = 0;

  this->Jitter = false;
}

vtkVgAssignCoordinates::~vtkVgAssignCoordinates()
{
  if(this->XYZCoordArrayName!=0)
    {
    delete[] this->XYZCoordArrayName;
    }
}

int vtkVgAssignCoordinates::RequestData(vtkInformation *vtkNotUsed(request),
                            vtkInformationVector **inputVector,
                            vtkInformationVector *outputVector)
{

  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkDataObject *input = inInfo->Get(vtkDataObject::DATA_OBJECT());
  vtkDataObject *output = outInfo->Get(vtkDataObject::DATA_OBJECT());

  // Do a shallow copy of the input to the output
  output->ShallowCopy(input);

  // Create new points on the output
  vtkDataSetAttributes *data = 0;
  vtkPoints* pts = vtkPoints::New();
  if (vtkPointSet::SafeDownCast(input))
    {
    vtkPointSet *psInput = vtkPointSet::SafeDownCast(input);
    vtkPointSet *psOutput = vtkPointSet::SafeDownCast(output);
    pts->DeepCopy(psInput->GetPoints());
    psOutput->SetPoints(pts);
    pts->Delete();
    data = psOutput->GetPointData();
    }
  else if (vtkGraph::SafeDownCast(input))
    {
    vtkGraph *graphInput = vtkGraph::SafeDownCast(input);
    vtkGraph *graphOutput = vtkGraph::SafeDownCast(output);
    pts->DeepCopy(graphInput->GetPoints());
    graphOutput->SetPoints(pts);
    pts->Delete();
    data = graphOutput->GetVertexData();
    }
  else
    {
    vtkErrorMacro(<<"Input must be graph or point set.");
    return 0;
    }

  // I need at least one coordinate array
  if (!this->XYZCoordArrayName || strlen(XYZCoordArrayName) == 0)
    {
    return 0;
    }

  // Okay now check for coordinate arrays
  vtkDataArray* XYZArray = data->GetArray(this->XYZCoordArrayName);

  // Does the array exist at all?
  if (XYZArray == NULL)
    {
    vtkErrorMacro("Could not find array named " << this->XYZCoordArrayName);
    return 0;
    }

  // Generate the points
  int numPts = pts->GetNumberOfPoints();
  for (int i = 0; i < numPts; i++)
    {
    double rx,ry;
    if (Jitter)
      {
      rx = vtkMath::Random()-.5;
      ry = vtkMath::Random()-.5;
      rx *= 1e-4;
      ry *= 1e-4;
      }
    else
      {
      rx = ry = 0;
      }

    double* tuple = XYZArray->GetTuple3(i);
    double point[3] = { tuple[0], tuple[1], tuple[2] };
    point[0] += rx;
    point[1] += ry;
    pts->SetPoint(i, point);
    }

  return 1;
}

int vtkVgAssignCoordinates::FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  // This algorithm may accept a vtkPointSet or vtkGraph.
  info->Remove(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE());
  info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPointSet");
  info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkGraph");
  return 1;
}

void vtkVgAssignCoordinates::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "XYZCoordArrayName: "
     << (this->XYZCoordArrayName ? this->XYZCoordArrayName : "(none)") << endl;

  os << indent << "Jitter: "
     << (this->Jitter ? "True" : "False") << endl;
}
