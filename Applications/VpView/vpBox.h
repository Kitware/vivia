/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
