/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgContourRepresentation_h
#define __vtkVgContourRepresentation_h

#include "vtkOrientedGlyphContourRepresentation.h"
#include "vtkTimeStamp.h"

#include <vgExport.h>

class vtkMatrix4x4;
class vtkPoints;

class VTKVG_CORE_EXPORT vtkVgContourRepresentation
  : public vtkOrientedGlyphContourRepresentation
{
public:
  vtkTypeMacro(vtkVgContourRepresentation, vtkOrientedGlyphContourRepresentation);

  static vtkVgContourRepresentation* New();

  void SetTransformMatrix(vtkMatrix4x4* mat);

  void SetShowHandles(bool enable);

  void Finalize();

  // Description:
  // Set / get the current operation. The widget is either
  // inactive, or it is being translated.
  virtual void SetCurrentOperation(int op);

  // Description:
  // Add a node at a specific display position. This will be
  // converted into a world position according to the current
  // constraints of the point placer. Return 0 if a point could
  // not be added, 1 otherwise.
  virtual int AddNodeAtDisplayPosition(double displayPos[2]);
  using vtkOrientedGlyphContourRepresentation::AddNodeAtDisplayPosition;

  // Description:
  // Given a specific X, Y pixel location, add a new node
  // on the contour at this location.
  virtual int AddNodeOnContour(int X, int Y);

  virtual void WidgetInteraction(double eventPos[2]);

  // Description:
  // Delete the nth node. Return 1 on success or 0 if n
  // is out of range.
  virtual int DeleteNthNode(int n);

  vtkPoints* GetPoints();

  // Description:
  // Find position on contour nearest to the given display point. Returns 0 if
  // no part of the contour comes near the display point.
  int FindClosestPoint(int X, int Y, double pos[3]);

protected:
  vtkVgContourRepresentation();
  ~vtkVgContourRepresentation();

  virtual void Initialize(vtkPolyData* pd);
  virtual void Initialize(vtkPolyData* pd, vtkIdList* list);

  void UpdateTransformedPoints();
  void ScaleContourFixed(double eventPos[2]);

private:
  vtkMatrix4x4* TransformMatrix;
  vtkPoints* TransformedPoints;
  vtkPoints* Points;
  vtkTimeStamp PointsBuildTime;
};

#endif
