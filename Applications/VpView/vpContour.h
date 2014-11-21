/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpContour_h
#define __vpContour_h

#include "vpBoundingRegion.h"

#include "vtkVgTimeStamp.h"

#include <vtkSmartPointer.h>

class vtkAbstractWidget;
class vtkContourWidget;
class vtkMatrix4x4;
class vtkPolyData;
class vtkRenderWindowInteractor;
class vtkVgContourWidget;
class vtkVgContourRepresentation;
class vtkVgGeode;
class vtkVgGroupNode;

class vpContour : public vpBoundingRegion
{
public:
  explicit vpContour(vtkRenderWindowInteractor* iren);
  virtual ~vpContour();

  void Initialize();
  virtual void Initialize(vtkPolyData* pd);

  void Begin();
  void End();

  void Finalize();

  virtual vtkPolyData* GetPolyData();
  virtual vtkAbstractWidget* GetWidget();

  virtual void SetVisible(int visible);
  virtual int  GetVisible() { return this->Visible; }

  virtual void SetLineColor(double r, double g, double b);
  void SetFinalLineColor(double r, double g, double b);

  virtual bool CanInteract(int X, int Y);

  void SetTimeStamp(vtkVgTimeStamp& timeStamp)
    {
    this->TimeStamp = timeStamp;
    }
  const vtkVgTimeStamp* GetTimeStampPtr()
    {
    return &this->TimeStamp;
    }

  void SetWorldToImageMatrix(vtkMatrix4x4* matrix);
  vtkMatrix4x4* GetWorldToImageMatrix()
    {
    return this->WorldToImageMatrix;
    }

protected:
  void DisableInteraction();

private:
  int Visible;
  bool InteractionDisabled;
  double FinalLineColor[3];

  vtkVgTimeStamp TimeStamp;
  vtkMatrix4x4* WorldToImageMatrix;

  vtkSmartPointer<vtkVgContourWidget> ContourWidget;
  vtkSmartPointer<vtkVgContourRepresentation> ContourRepresentation;
  vtkSmartPointer<vtkRenderWindowInteractor> Interactor;
};

#endif // __vpContour_h
