/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgVideoFrameMetaData.h"

#include <vgUtil.h>

#include <vtkVgAdapt.h>

#include <vtkLine.h>
#include <vtkMath.h>

#include <vgl/vgl_homg_point_2d.h>
#include <vgl/algo/vgl_h_matrix_2d.h>
#include <vgl/algo/vgl_h_matrix_2d_compute_linear.h>

//-----------------------------------------------------------------------------
static void prepareLine(const vtkVgVideoFrameCorner& p1,
                        const vtkVgVideoFrameCorner& p2,
                        double (&line)[2][3], double& length)
{
  line[0][0] = p1.Longitude;
  line[0][1] = p1.Latitude;
  line[0][2] = 0.0;
  line[1][0] = p2.Longitude;
  line[1][1] = p2.Latitude;
  line[1][2] = 0.0;
  length = sqrt(vtkMath::Distance2BetweenPoints(line[0], line[1]));
}

//-----------------------------------------------------------------------------
static int checkIntersection(double (&line1)[2][3], double (&line2)[2][3])
{
  double u, v;
  return vtkLine::Intersection(line1[0], line1[1], line2[0], line2[1], u, v);
}

//-----------------------------------------------------------------------------
vtkVgVideoFrameMetaData::vtkVgVideoFrameMetaData() :
  Gsd(-1.0),
  HomographyReferenceFrame(-1),
  Width(-1.0),
  Height(-1.0)
{
}

//-----------------------------------------------------------------------------
bool vtkVgVideoFrameMetaData::AreCornerPointsValid() const
{
  if (this->Width < 1.0 || this->Height < 1.0)
    {
    vtkGenericWarningMacro(
      "Unable to validate corner points due to invalid width/height!");
    return false;
    }

  // Check for explicitly invalid corner points
  if (this->WorldLocation.GCS == -1 ||
      this->WorldLocation.UpperLeft.Latitude   > 400.0 ||
      this->WorldLocation.UpperLeft.Longitude  > 400.0 ||
      this->WorldLocation.LowerLeft.Latitude   > 400.0 ||
      this->WorldLocation.LowerLeft.Longitude  > 400.0 ||
      this->WorldLocation.UpperRight.Latitude  > 400.0 ||
      this->WorldLocation.UpperRight.Longitude > 400.0 ||
      this->WorldLocation.LowerRight.Latitude  > 400.0 ||
      this->WorldLocation.LowerRight.Longitude > 400.0)
    {
    return false;
    }

  // Prepare to check if defined polygon is reasonable
  double line1[2][3];
  double line2[2][3];
  double edgeLength[4];

  // Check if top / bottom edges intersect
  prepareLine(this->WorldLocation.UpperLeft,
              this->WorldLocation.UpperRight, line1, edgeLength[0]);
  prepareLine(this->WorldLocation.LowerLeft,
              this->WorldLocation.LowerRight, line2, edgeLength[2]);
  if (checkIntersection(line1, line2) == 2)
    {
    return false;
    }

  // Check if left / right edges intersect
  prepareLine(this->WorldLocation.UpperLeft,
              this->WorldLocation.LowerLeft, line1, edgeLength[1]);
  prepareLine(this->WorldLocation.UpperRight,
              this->WorldLocation.LowerRight, line2, edgeLength[3]);
  if (checkIntersection(line1, line2) == 2)
    {
    return false;
    }

  // Normalize edge lengths relative to image aspect ratio; this will give
  // lengths that approach equal as the projection approaches orthogonal
  const double scale = static_cast<double>(this->Height) /
                       static_cast<double>(this->Width);
  edgeLength[0] *= scale;
  edgeLength[2] *= scale;

  // Compare shortest and longest edge lengths to determine divergence from
  // orthogonality... some divergence is expected, but if it exceeds a given
  // threshold, something is weird with the corners
  const double edgeLengthFactor = 6;
  double minLength = edgeLength[0], maxLength = edgeLength[0];
  for (int i = 1; i < 4; i++)
    {
    vgExpandBoundaries(minLength, maxLength, edgeLength[i]);
    }
  return (maxLength <= minLength * edgeLengthFactor);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMatrix4x4>
vtkVgVideoFrameMetaData::MakeImageToLatLonMatrix() const
{
  if (this->Width < 1.0 || this->Height < 1.0)
    {
    vtkGenericWarningMacro(
      "Unable to make image to lat/lon matrix due to invalid width/height!");
    return 0;
    }

  std::vector<vgl_homg_point_2d<double> > toPoints;
  toPoints.push_back(
    vgl_homg_point_2d<double>(this->WorldLocation.UpperLeft.Longitude,
                              this->WorldLocation.UpperLeft.Latitude, 1));
  toPoints.push_back(
    vgl_homg_point_2d<double>(this->WorldLocation.UpperRight.Longitude,
                              this->WorldLocation.UpperRight.Latitude, 1));
  toPoints.push_back(
    vgl_homg_point_2d<double>(this->WorldLocation.LowerRight.Longitude,
                              this->WorldLocation.LowerRight.Latitude, 1));
  toPoints.push_back(
    vgl_homg_point_2d<double>(this->WorldLocation.LowerLeft.Longitude,
                              this->WorldLocation.LowerLeft.Latitude, 1));

  double llx = 0;
  double lly = 0;
  double urx = this->Width - 1.0;
  double ury = this->Height - 1.0;

  std::vector<vgl_homg_point_2d<double> > fromPoints;
  fromPoints.push_back(vgl_homg_point_2d<double>(llx, ury, 1.0));
  fromPoints.push_back(vgl_homg_point_2d<double>(urx, ury, 1.0));
  fromPoints.push_back(vgl_homg_point_2d<double>(urx, lly, 1.0));
  fromPoints.push_back(vgl_homg_point_2d<double>(llx, lly, 1.0));

  vgl_h_matrix_2d_compute_linear algo;
  vgl_h_matrix_2d<double> homography;

  if (algo.compute(fromPoints, toPoints, homography))
    {
    return vtkVgAdapt(homography.get_matrix());
    }

  return 0;
}
