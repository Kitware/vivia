/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgRendererUtils.h"

#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkPlane.h>
#include <vtkRenderer.h>

//-----------------------------------------------------------------------------
void vtkVgRendererUtils::GetBounds(vtkRenderer* renderer,
                                   double bounds[4])
{
  // do we actually want to offset by half-pixel????
  double worldPt[4];
  renderer->SetViewPoint(-1, -1, 1);
  renderer->ViewToWorld();
  renderer->GetWorldPoint(worldPt);

  bounds[0] = worldPt[0];
  bounds[2] = worldPt[1];

  renderer->SetViewPoint(1, 1, 1);
  renderer->ViewToWorld();
  renderer->GetWorldPoint(worldPt);

  bounds[1] = worldPt[0];
  bounds[3] = worldPt[1];
}

//-----------------------------------------------------------------------------
void vtkVgRendererUtils::ZoomToImageExtents2D(
  vtkRenderer* ren, vtkImageData* image, bool parallelProjection/*=true*/)
{
  if (image)
    {
    int iextents[6];
    double spacing[3];
    image->GetExtent(iextents);
    image->GetSpacing(spacing);
    double extents[4] =
      {
      static_cast<double>(iextents[0]) * spacing[0],
      static_cast<double>(iextents[1]) * spacing[0],
      static_cast<double>(iextents[2]) * spacing[1],
      static_cast<double>(iextents[3]) * spacing[1],
      };
    ZoomToExtents2D(ren, extents, parallelProjection);
    }
}

//-----------------------------------------------------------------------------
void vtkVgRendererUtils::ZoomToExtents2D(vtkRenderer* ren,
                                         double extents[4],
                                         bool parallelProjection/*=true*/)
{
  vtkCamera* cam = ren->GetActiveCamera();

  double pos[3], fp[3];
  cam->GetPosition(pos);
  cam->GetFocalPoint(fp);

  pos[0] = 0.5 * (extents[0] + extents[1]);
  pos[1] = 0.5 * (extents[2] + extents[3]);

  fp[0] = pos[0];
  fp[1] = pos[1];

  cam->SetPosition(pos);
  cam->SetFocalPoint(fp);

  double extsize[] = { extents[1] - extents[0], extents[3] - extents[2] };
  if (extsize[1] == 0.0)
    {
    return;
    }

  double extAspect = extsize[0] / extsize[1];
  ren->ComputeAspect();

  double renAspect =  ren->GetAspect()[0];
  double scale = 0.5 * extsize[1];
  if (extAspect > renAspect)
    scale *= extAspect / renAspect;

  if (parallelProjection)
    {
    cam->SetParallelScale(scale);
    }
  else
    {
    double angle = 2 * atan((extsize[1] / 2) / cam->GetDistance());
    cam->SetViewAngle(vtkMath::DegreesFromRadians(angle));
    }
}

//-----------------------------------------------------------------------------
bool vtkVgRendererUtils::GetLinePlaneIntersection(double point1[3],
  double point2[3], double normal[3], double planePoint[3],
  double intersectedPoint[3])
{
  double t;
  int intersected = vtkPlane::IntersectWithLine(point1, point2, normal,
                                                planePoint, t, intersectedPoint);
  return (intersected == 1 ? true : false);
}

//-----------------------------------------------------------------------------
void vtkVgRendererUtils::CalculateBounds(const std::vector<double>& points,
  double bounds[6])
{
  if (!(points.size() % 3 == 0))
    {
    std::cerr << "ERROR: Requires points with three components" << std::endl;
    return;
    }

  for (size_t i = 0; i < points.size(); i += 3)
    {
    double point[3] =
      {
      points[i],
      points[i + 1],
      points[i + 2]
      };

    if (point[0] < bounds[0])
      {
      bounds[0] = point[0];
      }
    if (point[0] > bounds[1])
      {
      bounds[1] = point[0];
      }
    if (point[1] < bounds[2])
      {
      bounds[2] = point[1];
      }
    if (point[1] > bounds[3])
      {
      bounds[3] = point[1];
      }
    if (point[2] < bounds[4])
      {
      bounds[4] = point[2];
      }
    if (point[2] > bounds[5])
      {
      bounds[5] = point[2];
      }
    }
}
