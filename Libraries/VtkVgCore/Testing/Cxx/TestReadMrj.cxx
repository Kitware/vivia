// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgMultiResJpgImageReader2.h"

#include <vtkImageActor.h>
#include <vtkNew.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTesting.h>

int main(int argc, char** argv)
{
  if (argc < 2)
    {
    std::cerr << "Requires second argument for data source" << std::endl;
    return 1;
    }

  vtkNew<vtkVgMultiResJpgImageReader2> mrjReader;
  mrjReader->SetFileName(argv[1]);
  mrjReader->SetLevel(5);
  mrjReader->Update();

  vtkNew<vtkImageActor> imageActor;
  imageActor->SetInputData(mrjReader->GetOutput());

  vtkNew<vtkRenderer> ren;
  ren->AddActor(imageActor.GetPointer());

  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(ren.GetPointer());

  vtkNew<vtkRenderWindowInteractor> viewer;
  viewer->SetRenderWindow(renWin.GetPointer());

  renWin->Render();

  int retVal = vtkRegressionTestImageThreshold(renWin.GetPointer(), 25);
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    viewer->Start();
    retVal = vtkRegressionTester::PASSED;
    }

  return (!retVal);

  return 0;
}
