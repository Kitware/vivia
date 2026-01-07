// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
