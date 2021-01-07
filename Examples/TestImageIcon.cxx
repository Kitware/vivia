// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// This example tests the vtkVgImageIcon

// First include the required header files for the VTK classes we are using.
#include "vtkSmartPointer.h"
#include "vtkVgImageIcon.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkJPEGReader.h"

int main(int argc, char* argv[])
{
  // Create the RenderWindow, Renderer and both Actors
  //
  vtkSmartPointer<vtkRenderer> ren1 = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(ren1);

  vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  // Create the widget
  vtkSmartPointer<vtkVgImageIcon> icon = vtkSmartPointer<vtkVgImageIcon>::New();
  icon->SetBalloonLayoutToImageRight();
  icon->SetFileName("C:/F/CV/Data/Test/Beach/beach1.jpg");
  icon->SetRenderer(ren1);
  icon->SetImageSize(32, 32);

  // Add the actors to the renderer, set the background and size
  //
  ren1->AddViewProp(icon);
  ren1->SetBackground(0.1, 0.2, 0.4);
  renWin->SetSize(300, 300);

  // render the image
  //
  iren->Initialize();
  renWin->Render();

  icon->SetPosition(50, 50);
  renWin->Render();
  icon->SetPosition(75, 75);
  renWin->Render();

  iren->Start();

  return 0;
}
