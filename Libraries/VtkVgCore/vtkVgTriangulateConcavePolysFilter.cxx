// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgTriangulateConcavePolysFilter.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkIdList.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"

#include <queue>

vtkStandardNewMacro(vtkVgTriangulateConcavePolysFilter);

//-----------------------------------------------------------------------------
int vtkVgTriangulateConcavePolysFilter::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the info object
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkPolyData* input = vtkPolyData::SafeDownCast(
                         inInfo->Get(vtkPolyData::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast(
                          outInfo->Get(vtkPolyData::DATA_OBJECT()));

  // setup output equal to (just passing though) input;  will modify Polys and
  // CellData if we find any caoncave polys in the dataset
  output->ShallowCopy(input);

  // 1st check to see if there is anything other than triangles present;  if
  // not, can't have concave polygons (and thus we're done)
  if (input->GetPolys()->GetMaxCellSize() < 4)
    {
    return VTK_OK;
    }

  // anything greater than 3 needs to be inspected... do so and get a list (if
  // any) of those that need to be split into triangles (so now concave)
  vtkCellArray* inputPolys = input->GetPolys();
  vtkPoints* inputPoints = input->GetPoints();

  vtkIdType npts, *pts;
  inputPolys->InitTraversal();
  int additionalTriangles = 0, cellIndex = 0;
  std::queue<int> concaveCells;
  for (inputPolys->InitTraversal(); inputPolys->GetNextCell(npts, pts); cellIndex++)
    {
    if (npts > 3)
      {
      if (this->IsPolygonConcave(inputPoints, npts, pts))
        {
        // it's concave, add it to the queue
        additionalTriangles += npts - 3;
        concaveCells.push(cellIndex);
        }
      }
    }

  // if we didn't find any concave polygons then just pass input through (we're done)
  if (additionalTriangles == 0)
    {
    return VTK_OK;
    }

  // allocate array for polys
  vtkCellArray* outputPolys = vtkCellArray::New();
  outputPolys->Allocate(
    inputPolys->GetNumberOfConnectivityEntries() + (additionalTriangles * 4));

  // setup cell data
  vtkCellData* outputCellData = output->GetCellData();
  outputCellData->Initialize();
  outputCellData->CopyAllOn();
  outputCellData->CopyAllocate(input->GetCellData(),
                               input->GetNumberOfCells() + additionalTriangles);

  // ids for copying CellData
  vtkIdType inputCellId = 0, outputCellId = 0;

  // Handle any Vertices
  if (input->GetVerts())
    {
    output->SetVerts(input->GetVerts());
    for (int i = 0; i < input->GetNumberOfVerts(); i++)
      {
      outputCellData->CopyData(input->GetCellData(), inputCellId++, outputCellId++);
      }
    }
  // Handle any Lines
  if (input->GetLines())
    {
    output->SetLines(input->GetLines());
    for (int i = 0; i < input->GetNumberOfLines(); i++)
      {
      outputCellData->CopyData(input->GetCellData(), inputCellId++, outputCellId++);
      }
    }

  // Now the main task... triangulate any concave polygons, copying all others
  vtkPolygon* polygon = vtkPolygon::New();
  vtkIdList* outTris = vtkIdList::New();

  cellIndex = 0;
  int concaveCellIndex = concaveCells.front();
  concaveCells.pop();
  for (inputPolys->InitTraversal(); inputPolys->GetNextCell(npts, pts); cellIndex++)
    {
    // is it one of the concave cells?  If so, triangulate and add each of the
    // resulting triangles to the output polys; otherwise, just add the poly to
    // the output polys
    if (cellIndex == concaveCellIndex)
      {
      polygon->Initialize(npts, pts, inputPoints);
      polygon->Triangulate(outTris);
      vtkIdType newPts[3];
      for (int i = 0; i < outTris->GetNumberOfIds();)
        {
        newPts[0] = pts[outTris->GetId(i++)];
        newPts[1] = pts[outTris->GetId(i++)];
        newPts[2] = pts[outTris->GetId(i++)];
        outputPolys->InsertNextCell(3, newPts);
        outputCellData->CopyData(input->GetCellData(), inputCellId, outputCellId++);
        }
      inputCellId++;

      // get next concave cell, unless we've already processed them all, in
      // which case set to -1 (know we won't get a match)
      if (concaveCells.size() > 0)
        {
        concaveCellIndex = concaveCells.front();
        concaveCells.pop();
        }
      else
        {
        concaveCellIndex = -1;
        }
      }
    else
      {
      // cell wasn't concave, just copy it to the output
      outputPolys->InsertNextCell(npts, pts);
      outputCellData->CopyData(input->GetCellData(), inputCellId++, outputCellId++);
      }
    }

  output->SetPolys(outputPolys);

  // cleanup
  outputPolys->Delete();
  polygon->Delete();
  outTris->Delete();

  return VTK_OK;
}

//-----------------------------------------------------------------------------
bool vtkVgTriangulateConcavePolysFilter::IsPolygonConcave(vtkPoints* points,
    vtkIdType npts,
    vtkIdType* pts)
{
  // look for flip of cross product (direction) between adjacent sets of three points
  double v1[3], v2[3], pt[3][3], cross[2][3];

  int basePt = 1, previousPt, nextPt;
  points->GetPoint(pts[npts - 1], pt[0]);
  points->GetPoint(pts[0], pt[1]);

  for (vtkIdType i = 0; i < npts; i++, basePt++)
    {
    basePt %= 3;
    nextPt = (basePt + 1) % 3;
    previousPt = (basePt + 2) % 3;

    points->GetPoint(pts[(i + 1) % npts], pt[nextPt]);

    for (int j = 0; j < 3; j++)
      {
      v1[j] = pt[previousPt][j] - pt[basePt][j];
      v2[j] = pt[nextPt][j] - pt[basePt][j];
      }

    vtkMath::Cross(v1, v2, cross[i % 2]);
    if (i > 0)
      {
      if (vtkMath::Dot(cross[0], cross[1]) < 0)
        {
        return true;
        }
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
void vtkVgTriangulateConcavePolysFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
