// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpBox_h
#define __vpBox_h

#include "vpBoundingRegion.h"

#include <vtkSmartPointer.h>

class vtkVgBorderRepresentation;

class vtkAbstractWidget;
class vtkBorderWidget;
class vtkPolyData;
class vtkRenderWindowInteractor;

class vpBox : public vpBoundingRegion
{
public:
  explicit vpBox(vtkRenderWindowInteractor* iren);
  virtual ~vpBox();

  virtual void Initialize(vtkPolyData* pd);

  virtual vtkPolyData* GetPolyData();
  virtual vtkAbstractWidget* GetWidget();

  virtual void SetVisible(int visible);
  virtual int  GetVisible() { return this->Visible; }

  virtual void SetLineColor(double r, double g, double b);

  virtual bool CanInteract(int X, int Y);

private:
  int Visible;

  vtkSmartPointer<vtkBorderWidget> BorderWidget;
  vtkSmartPointer<vtkVgBorderRepresentation> BorderRepresentation;
  vtkSmartPointer<vtkRenderWindowInteractor> Interactor;
  vtkSmartPointer<vtkPolyData> PolyData;
};

#endif // __vpBox_h
