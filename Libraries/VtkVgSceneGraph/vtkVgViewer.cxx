// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgViewer.h"

// VTK includes.
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(vtkVgViewer);

//-----------------------------------------------------------------------------
vtkVgViewer::vtkVgViewer() : vtkVgViewerBase(),
  FirstFrame(true),
  Realized(false)
{
}

//-----------------------------------------------------------------------------
vtkVgViewer::~vtkVgViewer()
{
}

//-----------------------------------------------------------------------------
void vtkVgViewer::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "Realized: " << (this->Realized ? "TRUE" : "FALSE") << "\n";

  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
bool vtkVgViewer::IsFirstFrame()
{
  return this->FirstFrame;
}

//-----------------------------------------------------------------------------
bool vtkVgViewer::IsRealized()
{
  return this->Realized;
}

//-----------------------------------------------------------------------------
void vtkVgViewer::Realize()
{
  this->Interactor->Initialize();

  this->Realized = true;
}

//-----------------------------------------------------------------------------
void vtkVgViewer::Frame(const vtkVgTimeStamp& timestamp)
{
  if (!this->IsRealized())
    {
    this->Realize();
    }

  this->Update(timestamp);
}

//-----------------------------------------------------------------------------
void vtkVgViewer::ForceRender(bool resetCameraOnFirstFrame/*=true*/)
{
  if (this->IsFirstFrame())
    {
    this->FirstFrame = false;
    if (resetCameraOnFirstFrame)
      {
      this->SceneRenderer->ResetCamera();
      }
    }
  Superclass::ForceRender();
}

//-----------------------------------------------------------------------------
void vtkVgViewer::Run()
{
  if (!this->IsRealized())
    {
    this->Realize();
    }

  this->Interactor->CreateRepeatingTimer(100);

  this->Interactor->Start();
}

//-----------------------------------------------------------------------------
void vtkVgViewer::GetCurrentViewExtents(double* viewExtents)
{
  vtkCamera* camera = this->SceneRenderer->GetActiveCamera();

  double cameraCurrentPosition[3];
  camera->GetPosition(cameraCurrentPosition);
  double* rendererAspect = this->SceneRenderer->GetAspect();

  // xmin, xmax, ymin, ymax
  double renderedBounds[4] =
    {
    VTK_DOUBLE_MAX, VTK_DOUBLE_MIN,
    VTK_DOUBLE_MAX, VTK_DOUBLE_MIN
    };

  // might be rotated, so need to do all four corners to make sure we
  // got axis-aligned bounds

  for (int i = -1; i < 2; i += 2)
    {
    for (int j = -1; j < 2; j += 2)
      {
      double worldPt[4];
      this->SceneRenderer->SetViewPoint(i, j, 1);
      this->SceneRenderer->ViewToWorld();
      this->SceneRenderer->GetWorldPoint(worldPt);
      if (renderedBounds[0] > worldPt[0])
        {
        renderedBounds[0] = worldPt[0];
        }
      if (renderedBounds[1] < worldPt[0])
        {
        renderedBounds[1] = worldPt[0];
        }
      if (renderedBounds[2] > worldPt[1])
        {
        renderedBounds[2] = worldPt[1];
        }
      if (renderedBounds[3] < worldPt[1])
        {
        renderedBounds[3] = worldPt[1];
        }
      }
    }

  double extentHeight, extentWidth;
  extentWidth = renderedBounds[1] - renderedBounds[0];

  viewExtents[0] = cameraCurrentPosition[0] - extentWidth * 0.5;
  viewExtents[1] = cameraCurrentPosition[0] + extentWidth * 0.5;
  extentHeight = (viewExtents[1] - viewExtents[0]) * rendererAspect[1] / rendererAspect[0];
  viewExtents[2] = cameraCurrentPosition[1] - extentHeight * 0.5;
  viewExtents[3] = cameraCurrentPosition[1] + extentHeight * 0.5;
}

//-----------------------------------------------------------------------------
void vtkVgViewer::UpdateView(double bounds[6])
{
  vtkCamera* camera = this->SceneRenderer->GetActiveCamera();

  double center[2] = {(bounds[1] + bounds[0]) * 0.5,
                      (bounds[3] + bounds[2]) * 0.5
                     };

  camera->SetPosition(center[0], center[1], 1.0);
  camera->SetFocalPoint(center[0], center[1], 1.0);

  this->ForceRender();
}

//-----------------------------------------------------------------------------
int vtkVgViewer::UpdateViewTriggersEvent() const
{
  return vtkCommand::UpdateEvent;
}
