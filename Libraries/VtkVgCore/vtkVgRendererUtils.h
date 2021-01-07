// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgRendererUtils_h
#define __vtkVgRendererUtils_h

// C++ includes
#include <vector>

#include <vgExport.h>

// Forward declarations
class vtkImageData;
class vtkRenderer;

// FIXME: use namespace when we can wrap those
struct VTKVG_CORE_EXPORT vtkVgRendererUtils
{
  // Description:
  // Get the world bounds of renderer viewport as {xmin, ymin, xmax, ymax}.
  static void GetBounds(vtkRenderer* renderer, double bounds[4]);

  // Description:
  // Set camera zoom to fill the viewport with the given image. Maintains
  // aspect ratio.
  static void ZoomToImageExtents2D(
    vtkRenderer* renderer, vtkImageData* image, bool parallelProjection = true);

  // Description:
  // Set camera zoom to fill the viewport with the given extents,
  // specified as {xmin, xmax, ymin, ymax}. Maintains aspect ratio.
  static void ZoomToExtents2D(
    vtkRenderer* renderer, double extents[4], bool parallelProjection = true);

  // Description:
  // Given two points on line, normal of a plane, a point on a plane return
  // intersected point.
  static bool GetLinePlaneIntersection(
    double point1[3], double point2[3], double normal[3],
    double planePoint[3], double intersectedPoint[3]);

  // Description:
  // Given points (each point has three components double[3]), return bounds of
  // the bounding box.
  static void CalculateBounds(const std::vector<double>& points,
                              double bounds[6]);
};

#endif // __vtkVgRendererUtils_h
