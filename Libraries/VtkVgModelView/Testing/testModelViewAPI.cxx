// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <cstdio>

// VTK includes.
#include <vtkCommand.h>
#include <vtkProp.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

// VG includes.
#include "vtkVgVideoProviderBase.h"
#include "vtkVgFileVideoSource.h"
#include "vtkVgVideoRepresentation0.h"
#include "vtkVgVideoModel0.h"
#include "vtkVgTimeStamp.h"

int main(int argc, char** argv)
{
  if (argc < 2)
    {
    std::cerr << "Requires second argument for data source" << std::endl;
    return 1;
    }

  vtkVgTimeStamp timeStamp;

  // Create two \c video model and corresponding representations.
  vtkVgVideoModel0::SmartPtr videoModel  = vtkVgVideoModel0::SmartPtr::New();
  vtkVgVideoModel0::SmartPtr videoModel2 = vtkVgVideoModel0::SmartPtr::New();

  vtkVgVideoRepresentationBase0::SmartPtr videoRep
    = vtkVgVideoRepresentation0::SmartPtr::New();
  vtkVgVideoRepresentationBase0::SmartPtr videoRep2
    = vtkVgVideoRepresentation0::SmartPtr::New();

  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();

  vtkVgVideoProviderBase::SmartPtr videoSource = vtkVgFileVideoSource::SmartPtr::New();

  videoSource->SetDataSource(argv[1]);

  videoModel->SetVideoSource(videoSource);
  videoModel->SetLooping(1);
  videoModel2->SetVideoSource(videoSource);
  videoModel2->SetLooping(1);

  videoRep ->SetVideoModel(videoModel);
  videoRep2->SetVideoModel(videoModel2);

  vtkSmartPointer<vtkRenderer> ren(vtkSmartPointer<vtkRenderer>::New());

  renWin->AddRenderer(ren);

  vtkSmartPointer<vtkRenderWindowInteractor> renWinIntr(
    vtkSmartPointer<vtkRenderWindowInteractor>::New());
  renWinIntr->SetRenderWindow(renWin);

  timeStamp.SetFrameNumber(1);

  videoModel->Update(timeStamp);
  videoModel2->Update(timeStamp);

  // Add props from video items.
  vtkPropCollection* collection = videoRep->GetActiveRenderObjects();
  collection->InitTraversal();

  vtkProp* prop = collection->GetNextProp();
  while (prop)
    {
    ren->AddViewProp(prop);
    prop = collection->GetNextProp();
    }

  vtkPropCollection* collection2 = videoRep2->GetActiveRenderObjects();
  collection2->InitTraversal();

  prop = collection2->GetNextProp();
  while (prop)
    {
    ren->AddViewProp(prop);
    prop = collection->GetNextProp();
    }

  ren->SetBackground(0.5, 0.5, 0.5);

  ren->ResetCamera();

  // \NOTE: Don't render.

  return 0;
}
