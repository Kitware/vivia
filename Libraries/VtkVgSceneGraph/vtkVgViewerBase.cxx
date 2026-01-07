// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgViewerBase.h"

// VTK includes.
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

//-----------------------------------------------------------------------------
vtkVgViewerBase::vtkVgViewerBase() : vtkVgViewBase()
{
  this->RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  this->Interactor   = vtkSmartPointer<vtkRenderWindowInteractor>::New();

  this->RenderWindow->AddRenderer(this->SceneRenderer);
  this->RenderWindow->SetInteractor(this->Interactor);

  // We are using orthographic camera.
  this->SceneRenderer->GetActiveCamera()->ParallelProjectionOn();
}

//-----------------------------------------------------------------------------
vtkVgViewerBase::~vtkVgViewerBase()
{
}

//-----------------------------------------------------------------------------
void vtkVgViewerBase::PrintSelf(ostream& vtkNotUsed(os),
                                vtkIndent vtkNotUsed(indent))
{
}

//-----------------------------------------------------------------------------
void vtkVgViewerBase::SetRenderWindow(vtkRenderWindow* renderWindow)
{
  if (renderWindow && (renderWindow != this->RenderWindow))
    {
    this->RenderWindow = renderWindow;
    this->RenderWindow->AddRenderer(this->SceneRenderer);
    this->RenderWindow->SetInteractor(this->Interactor);
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
vtkRenderWindow* vtkVgViewerBase::GetRenderWindow()
{
  return this->RenderWindow;
}

//-----------------------------------------------------------------------------
const vtkRenderWindow* vtkVgViewerBase::GetRenderWindow() const
{
  return this->RenderWindow;
}

//-----------------------------------------------------------------------------
void vtkVgViewerBase::SetRenderWindowInteractor(vtkRenderWindowInteractor* interactor)
{
  if (interactor && (this->Interactor != interactor))
    {
    this->Interactor = interactor;
    this->RenderWindow->SetInteractor(this->Interactor);
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
vtkRenderWindowInteractor* vtkVgViewerBase::GetRenderWindowInteractor()
{
  return this->Interactor;
}

//-----------------------------------------------------------------------------
const vtkRenderWindowInteractor* vtkVgViewerBase::GetRenderWindowInteractor() const
{
  return this->Interactor;
}

//-----------------------------------------------------------------------------
double vtkVgViewerBase::GetCurrentScale()
{
  // If not a valid renderer return 1.
  if (!this->SceneRenderer)
    {
    return 1.0;
    }

  int*   viewPortSize = this->SceneRenderer->GetSize();
  double cameraWidth  = this->SceneRenderer->GetActiveCamera()->GetParallelScale() *
                        ((double)viewPortSize[0] / viewPortSize[1]);

  return ((cameraWidth) / (double)viewPortSize[0]);
}

//-----------------------------------------------------------------------------
void vtkVgViewerBase::ForceRender(bool vtkNotUsed(resetCameraOnFirstFrame))
{
  this->RenderWindow->Render();
}

//-----------------------------------------------------------------------------
void vtkVgViewerBase::ResetView()
{
  this->SceneRenderer->ResetCamera();
}
