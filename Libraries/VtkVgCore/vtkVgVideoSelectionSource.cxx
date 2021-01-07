// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgVideoSelectionSource.h"

#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkVgVideoSelectionSource);

//----------------------------------------------------------------------------
vtkVgVideoSelectionSource::vtkVgVideoSelectionSource()
{
  for (int i = 0; i < 3; i++)
    {
    this->Bounds[2 * i] = -1.0;
    this->Bounds[2 * i + 1] = 1.0;
    }

  this->RelativeTriangleSize = 0.05;

  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
int vtkVgVideoSelectionSource::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
{
  // get the info object
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData* output = vtkPolyData::SafeDownCast(
                          outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // construct 4 equilateral triangles, the side of which are
  // specifed % of sum of height and width of bounds

  if (this->Bounds[0] > this->Bounds[1] ||
      this->Bounds[2] > this->Bounds[3] ||
      this->Bounds[4] > this->Bounds[5])
    {
    vtkErrorMacro("Invalid bounds!");
    return VTK_ERROR;
    }

  double width = this->Bounds[1] - this->Bounds[0];
  double height = this->Bounds[3] - this->Bounds[2];
  double triangleSize = (width + height) * this->RelativeTriangleSize;

  vtkPoints* outputPts = vtkPoints::New();
  outputPts->Allocate(12);
  output->SetPoints(outputPts);
  outputPts->FastDelete();

  vtkCellArray* polys = vtkCellArray::New();
  polys->Allocate(16);
  output->SetPolys(polys);
  polys->FastDelete();

  double center[3] = { (this->Bounds[1] + this->Bounds[0]) / 2.0,
                       (this->Bounds[3] + this->Bounds[2]) / 2.0,
                       (this->Bounds[5] + this->Bounds[4]) / 2.0
                     };

  double newPt[3];
  vtkIdType pts[3];

  if (0) // equilateral
    {
    // top
    newPt[0] = center[0];
    newPt[1] = this->Bounds[3];
    newPt[2] = center[2];
    pts[0] = outputPts->InsertNextPoint(newPt);

    newPt[0] += 0.5 * triangleSize; // 0.5 = cos(vtkMath::Pi() / 3.0)
    newPt[1] += sin(vtkMath::Pi() / 3.0) * triangleSize;
    pts[1] = outputPts->InsertNextPoint(newPt);

    newPt[0] -= triangleSize;
    pts[2] = outputPts->InsertNextPoint(newPt);
    polys->InsertNextCell(3, pts);

    // bottom
    newPt[0] = center[0];
    newPt[1] = this->Bounds[2];
    pts[0] = outputPts->InsertNextPoint(newPt);

    newPt[0] -= 0.5 * triangleSize; // 0.5 = cos(vtkMath::Pi() / 3.0)
    newPt[1] -= sin(vtkMath::Pi() / 3.0) * triangleSize;
    pts[1] = outputPts->InsertNextPoint(newPt);

    newPt[0] += triangleSize;
    pts[2] = outputPts->InsertNextPoint(newPt);
    polys->InsertNextCell(3, pts);

    // left
    newPt[0] = this->Bounds[0];
    newPt[1] = center[1];
    pts[0] = outputPts->InsertNextPoint(newPt);

    newPt[0] -= sin(vtkMath::Pi() / 3.0) * triangleSize;
    newPt[1] += 0.5 * triangleSize; // 0.5 = cos(vtkMath::Pi() / 3.0)
    pts[1] = outputPts->InsertNextPoint(newPt);

    newPt[1] -= triangleSize;
    pts[2] = outputPts->InsertNextPoint(newPt);
    polys->InsertNextCell(3, pts);

    // right
    newPt[0] = this->Bounds[1];
    newPt[1] = center[1];
    pts[0] = outputPts->InsertNextPoint(newPt);

    newPt[0] += sin(vtkMath::Pi() / 3.0) * triangleSize;
    newPt[1] -= 0.5 * triangleSize; // 0.5 = cos(vtkMath::Pi() / 3.0)
    pts[1] = outputPts->InsertNextPoint(newPt);

    newPt[1] += triangleSize;
    pts[2] = outputPts->InsertNextPoint(newPt);
    polys->InsertNextCell(3, pts);
    }
  else
    {
    // top
    newPt[0] = center[0];
    newPt[1] = this->Bounds[3];
    newPt[2] = center[2];
    pts[0] = outputPts->InsertNextPoint(newPt);

    newPt[0] += width / 2.0;
    newPt[1] += triangleSize;
    pts[1] = outputPts->InsertNextPoint(newPt);

    newPt[0] -= width;
    pts[2] = outputPts->InsertNextPoint(newPt);
    polys->InsertNextCell(3, pts);

    // bottom
    newPt[0] = center[0];
    newPt[1] = this->Bounds[2];
    pts[0] = outputPts->InsertNextPoint(newPt);

    newPt[0] -= width / 2.0;
    newPt[1] -= triangleSize;
    pts[1] = outputPts->InsertNextPoint(newPt);

    newPt[0] += width;
    pts[2] = outputPts->InsertNextPoint(newPt);
    polys->InsertNextCell(3, pts);

    // left
    newPt[0] = this->Bounds[0];
    newPt[1] = center[1];
    pts[0] = outputPts->InsertNextPoint(newPt);

    newPt[0] -= triangleSize;
    newPt[1] += height / 2.0;
    pts[1] = outputPts->InsertNextPoint(newPt);

    newPt[1] -= height;
    pts[2] = outputPts->InsertNextPoint(newPt);
    polys->InsertNextCell(3, pts);

    // right
    newPt[0] = this->Bounds[1];
    newPt[1] = center[1];
    pts[0] = outputPts->InsertNextPoint(newPt);

    newPt[0] += triangleSize;
    newPt[1] -= height / 2.0;
    pts[1] = outputPts->InsertNextPoint(newPt);

    newPt[1] += height;
    pts[2] = outputPts->InsertNextPoint(newPt);
    polys->InsertNextCell(3, pts);
    }

  return 1;
}

//----------------------------------------------------------------------------
void vtkVgVideoSelectionSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
