// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgQuad.h"

// VTK includes
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkActor> vtkVgQuad::CreateQuad(double ulx, double uly,
                                                double llx, double lly,
                                                double lrx, double lry,
                                                double urx, double ury)
{
  vtkSmartPointer<vtkActor> actor (vtkSmartPointer<vtkActor>::New());
  vtkSmartPointer<vtkPolyDataMapper> mapper
    (vtkSmartPointer<vtkPolyDataMapper>::New());

  // Points of the quad.
  vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
  pts->InsertNextPoint(ulx, uly, 0.0);
  pts->InsertNextPoint(llx, lly, 0.0);
  pts->InsertNextPoint(lrx, lry, 0.0);
  pts->InsertNextPoint(urx, ury, 0.0);

  // Formation.
  vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
  polys->InsertNextCell(4);
  polys->InsertCellPoint(0);
  polys->InsertCellPoint(1);
  polys->InsertCellPoint(2);
  polys->InsertCellPoint(3);

  // Texture coordinates.
  vtkSmartPointer<vtkDoubleArray> texCoords =
    vtkSmartPointer<vtkDoubleArray>::New();
  texCoords->SetNumberOfComponents(2);
  texCoords->SetNumberOfTuples(4);

  double texCoord[2];
  texCoord[0]=0.0;
  texCoord[1]=0.0;
  texCoords->SetTypedTuple(0,texCoord);
  texCoord[0]=1.0;
  texCoord[1]=0.0;
  texCoords->SetTypedTuple(1,texCoord);
  texCoord[0]=1.0;
  texCoord[1]=1.0;
  texCoords->SetTypedTuple(2,texCoord);
  texCoord[0]=0.0;
  texCoord[1]=1.0;
  texCoords->SetTypedTuple(3,texCoord);
  texCoords->Modified();

  vtkSmartPointer<vtkPolyData> quadGeom = vtkSmartPointer<vtkPolyData>::New();
  quadGeom->SetPoints(pts);
  quadGeom->SetPolys(polys);
  quadGeom->GetPointData()->SetTCoords(texCoords);

  mapper->SetInputData(quadGeom);
  actor->SetMapper(mapper);

  return actor;
}
