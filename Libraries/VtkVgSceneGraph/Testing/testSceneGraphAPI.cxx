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

#include "vtkVgSceneManager.h"
#include "vtkVgGroupNode.h"
#include "vtkVgTransformNode.h"
#include "vtkVgVideoNode.h"

int main(int argc, char** argv)
{
  if (argc < 2)
    {
    std::cerr << "Requires second argument for data source" << std::endl;
    return 1;
    }

  vtkVgTimeStamp                         timeStamp;

  // Video item 1.
  vtkVgVideoModel0::SmartPtr              videoModel;
  vtkVgVideoRepresentationBase0::SmartPtr videoRep;

  // Video item 2.
  vtkVgVideoModel0::SmartPtr              videoModel2;
  vtkVgVideoRepresentationBase0::SmartPtr videoRep2;

  // Scene nodes and scene manager.
  vtkVgSceneManager::SmartPtr            sceneManager;

  vtkVgGroupNode::SmartPtr               root;
  vtkVgVideoNode::SmartPtr               videoNode;
  vtkVgVideoNode::SmartPtr               videoNode2;
  vtkVgTransformNode::SmartPtr           transformNode;

  // Render window.
  vtkSmartPointer<vtkRenderWindow>       renWin;

  vtkVgVideoProviderBase::SmartPtr videoSource = vtkVgFileVideoSource::SmartPtr::New();
  videoSource->SetDataSource(argv[1]);

  // Transform for videoModel2.
  vtkSmartPointer<vtkTransform> transform(vtkSmartPointer<vtkTransform>::New());
  transform->Translate(10000.0, 10000.0, 0.0);

  // Create video item 1.
  videoModel = vtkVgVideoModel0::SmartPtr::New();
  videoModel->SetVideoSource(videoSource);
  videoModel->SetLooping(1);
  videoRep  = vtkVgVideoRepresentation0::SmartPtr::New();
  videoRep->SetVideoModel(videoModel);

  // Create video item 2.
  videoModel2 = vtkVgVideoModel0::SmartPtr::New();
  videoModel2->SetVideoSource(videoSource);
  videoModel2->SetLooping(1);
  videoModel2->SetVideoMatrix(transform->GetMatrix());
  videoRep2  = vtkVgVideoRepresentation0::SmartPtr::New();
  videoRep2->SetVideoModel(videoModel2);

  // Create scene nodes and scene manager.
  sceneManager = vtkVgSceneManager::SmartPtr::New();

  root          = vtkVgGroupNode::SmartPtr::New();
  videoNode     = vtkVgVideoNode::SmartPtr::New();
  videoNode2    = vtkVgVideoNode::SmartPtr::New();
  transformNode = vtkVgTransformNode::SmartPtr::New();

  transformNode->AddChild(videoNode2);
  root->AddChild(videoNode);
  root->AddChild(transformNode);

  videoNode->SetVideoRepresentation(videoRep);
  videoNode2->SetVideoRepresentation(videoRep2);

  timeStamp.SetFrameNumber(1);

  sceneManager->SetSceneRoot(root);

  // Update it once.
  sceneManager->Update(timeStamp);

  // Set the VTK render pipeline.
  renWin = vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(sceneManager->GetSceneRenderer());

  vtkSmartPointer<vtkRenderWindowInteractor> renWinIntr
  (vtkSmartPointer<vtkRenderWindowInteractor>::New());
  renWinIntr->SetRenderWindow(renWin);

  // \NOTE: Don't render. This is a test for the API.
  return 0;
}
