// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpGraphAnimateCallback_h
#define __vpGraphAnimateCallback_h

// VisGUI includes
#include "vpMultiGraphModel.h"
#include "vpMultiGraphRepresentation.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// C++ includes
#include <vector>

class vpGraphAnimateCallback : public vtkCommand
{
public:
  vpGraphAnimateCallback() : vtkCommand(),
    NoOfSteps(20),
    CurrentStep(1)
    {
    }

  static vpGraphAnimateCallback* New()
    {
    vpGraphAnimateCallback* vpcb = new vpGraphAnimateCallback();
    return vpcb;
    }

  void Reset(vpMultiGraphModel* graphModel, vpMultiGraphRepresentation* rep,
             vtkRenderer* ren,
             const std::vector<vtkIdType>& ids,
             const std::vector<double>& from,
             const std::vector<double>& to,
             vpMultiGraphModel::NodePositionType positionType)
    {
    this->CurrentStep = 1;
    this->GraphModel = graphModel;
    this->GraphRep = rep;
    this->Renderer = ren;
    this->RenderWindow = this->Renderer->GetRenderWindow();
    this->Ids = ids;
    this->ToXYZs = to;
    this->FromXYZs = from;
    this->PositionType = positionType;
    }

  virtual void Execute(vtkObject* vtkNotUsed(caller),
                       unsigned long vtkNotUsed(eventId),
                       void* vtkNotUsed(callData))
    {
    double transX, transY, transZ;
    size_t ix, iy, iz;
    int j = this->CurrentStep;
    for (size_t i = 0; i < Ids.size(); ++i)
      {
      ix = i * 3;
      iy = i * 3 + 1;
      iz = i * 3 + 2;

      transX = FromXYZs[ix] + j * (ToXYZs[ix] - FromXYZs[ix]) / NoOfSteps;
      transY = FromXYZs[iy] + j * (ToXYZs[iy] - FromXYZs[iy]) / NoOfSteps;
      transZ = FromXYZs[iz] + j * (ToXYZs[iz] - FromXYZs[iz]) / NoOfSteps;

      this->GraphModel->MoveNode(this->PositionType, Ids[i], transX, transY,
                                 transZ, true);
      }

    this->RenderWindow->Render();

    if (this->CurrentStep < this->NoOfSteps)
      {
      this->RenderWindow->GetInteractor()->CreateOneShotTimer(30);
      ++this->CurrentStep;
      }
    }

 private:
  int NoOfSteps;
  int CurrentStep;

  std::vector<vtkIdType> Ids;
  std::vector<double> FromXYZs;
  std::vector<double> ToXYZs;
  vpMultiGraphModel::NodePositionType PositionType;

  vpMultiGraphModel* GraphModel;
  vpMultiGraphRepresentation* GraphRep;
  vtkRenderer* Renderer;
  vtkRenderWindow* RenderWindow;
};

#endif // __vpGraphAnimateCallback_h
