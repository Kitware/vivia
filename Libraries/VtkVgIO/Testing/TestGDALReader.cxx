// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <cstdio>

// VG includes.
#include "../vtkGDALReader.h"

// VTK includes.
#include <vtkCommand.h>
#include <vtkImageActor.h>
#include <vtkProp.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTestUtilities.h>
#include <vtkTransform.h>

void PrintCornerPoints(const char* name, const double* corners)
{
  std::cerr << name << " " << corners[0] << " " << corners[1] << std::endl;
}

int main(int argc, char** argv)
{
  char* fileName = vtkTestUtilities::ExpandDataFileName(
                     argc, argv, "Data/20091021202634-01000197-VIS.ntf.r0");

  // Create and use GDAL reader
  vtkSmartPointer<vtkGDALReader> reader(vtkSmartPointer<vtkGDALReader>::New());
  reader->SetFileName(fileName);
  reader->Update();

  const char* projectionString = reader->GetProjectionString();
  if (projectionString != NULL)
    {
    std::cerr << "Projection string: " << projectionString << std::endl;
    }

  const double* geocornerPoints = reader->GetGeoCornerPoints();
  PrintCornerPoints("UpperLeft", &geocornerPoints[0]);
  PrintCornerPoints("LowerLeft", &geocornerPoints[2]);
  PrintCornerPoints("LowerRight", &geocornerPoints[4]);
  PrintCornerPoints("UpperRight", &geocornerPoints[6]);

  // Create actor
  vtkSmartPointer<vtkImageActor> actor(vtkSmartPointer<vtkImageActor>::New());
  actor->SetInputData(reader->GetOutput());

  // Create renderer and attach our actor to it
  vtkSmartPointer<vtkRenderer> ren(vtkSmartPointer<vtkRenderer>::New());
  ren->AddViewProp(actor);

  // Create render window and attach renderer to it
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(ren);

  vtkSmartPointer<vtkRenderWindowInteractor> interactor(
    vtkSmartPointer<vtkRenderWindowInteractor>::New());
  interactor->SetRenderWindow(renWin);

  renWin->SetSize(400, 400);
  renWin->Render();
  interactor->Initialize();
  renWin->Render();

  int retVal = vtkRegressionTestImage(renWin);
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    interactor->Start();
    }

  return !retVal;
}
