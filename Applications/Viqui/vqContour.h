/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqContour_h
#define __vqContour_h

#include <vtkSmartPointer.h>

class vtkContourWidget;
class vtkOrientedGlyphContourRepresentation;
class vtkPolyData;
class vtkRenderWindowInteractor;
class vtkVgGeode;
class vtkVgGroupNode;

class vqContour
{
public:
  vqContour(vtkRenderWindowInteractor* iren);
  ~vqContour();

  void Initialize();
  void Initialize(vtkPolyData* pd);

  void Begin();
  void End();

  void Finalize();
  void AddToScene(vtkVgGroupNode* parent);

  vtkPolyData* GetPolyData();

  void SetVisible(int visible);
  int  GetVisible() { return this->Visible; }

protected:
  vtkOrientedGlyphContourRepresentation* GetRepresentation();

  void DisableInteraction();

private:
  int Visible;

  vtkSmartPointer<vtkVgGeode> SceneNode;
  vtkSmartPointer<vtkVgGroupNode> SceneParent;
  vtkSmartPointer<vtkContourWidget> ContourWidget;
  vtkSmartPointer<vtkRenderWindowInteractor> Interactor;
};

#endif // __vqContour_h
