/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
