/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVQCoordinateTransform.h"

// VTK includes.
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>

// VXL includes.
#include <vgl/vgl_homg_point_2d.h>
#include <vgl/algo/vgl_h_matrix_2d.h>
#include <vgl/algo/vgl_h_matrix_2d_compute_linear.h>

#include "vtkVgAdapt.h"

vtkStandardNewMacro(vtkVQCoordinateTransform);

//-----------------------------------------------------------------------------
vtkVQCoordinateTransform::vtkVQCoordinateTransform()
{
}

//-----------------------------------------------------------------------------
vtkVQCoordinateTransform::~vtkVQCoordinateTransform()
{
}

//-----------------------------------------------------------------------------
void vtkVQCoordinateTransform::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVQCoordinateTransform::SetFromPoints(double pt1[], double pt2[],
                                             double pt3[], double pt4[])
{
  this->FromPoint[0][0] = pt1[0];
  this->FromPoint[0][1] = pt1[1];
  this->FromPoint[1][0] = pt2[0];
  this->FromPoint[1][1] = pt2[1];
  this->FromPoint[2][0] = pt3[0];
  this->FromPoint[2][1] = pt3[1];
  this->FromPoint[3][0] = pt4[0];
  this->FromPoint[3][1] = pt4[1];
}

//-----------------------------------------------------------------------------
void vtkVQCoordinateTransform::SetFromPoints(double x1, double y1,
                                             double x2, double y2,
                                             double x3, double y3,
                                             double x4, double y4)
{
  this->FromPoint[0][0] = x1;
  this->FromPoint[0][1] = y1;
  this->FromPoint[1][0] = x2;
  this->FromPoint[1][1] = y2;
  this->FromPoint[2][0] = x3;
  this->FromPoint[2][1] = y3;
  this->FromPoint[3][0] = x4;
  this->FromPoint[3][1] = y4;
}

//-----------------------------------------------------------------------------
void vtkVQCoordinateTransform::SetToPoints(double pt1[], double pt2[],
                                           double pt3[], double pt4[])
{
  this->ToPoint[0][0] = pt1[0];
  this->ToPoint[0][1] = pt1[1];
  this->ToPoint[1][0] = pt2[0];
  this->ToPoint[1][1] = pt2[1];
  this->ToPoint[2][0] = pt3[0];
  this->ToPoint[2][1] = pt3[1];
  this->ToPoint[3][0] = pt4[0];
  this->ToPoint[3][1] = pt4[1];
}

//-----------------------------------------------------------------------------
void vtkVQCoordinateTransform::SetToPoints(double x1, double y1,
                                           double x2, double y2,
                                           double x3, double y3,
                                           double x4, double y4)
{
  this->ToPoint[0][0] = x1;
  this->ToPoint[0][1] = y1;
  this->ToPoint[1][0] = x2;
  this->ToPoint[1][1] = y2;
  this->ToPoint[2][0] = x3;
  this->ToPoint[2][1] = y3;
  this->ToPoint[3][0] = x4;
  this->ToPoint[3][1] = y4;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMatrix4x4> vtkVQCoordinateTransform::GetHomographyMatrix()
{
  vcl_vector< vgl_homg_point_2d<double> > fromPoints;
  vcl_vector< vgl_homg_point_2d<double> > toPoints;

  // Add world coordinates to worldPoints
  for (int i = 0; i < 4; ++i)
    {
    vgl_homg_point_2d<double> vglPt2d(this->FromPoint[i][0], this->FromPoint[i][1], 1);
    fromPoints.push_back(vglPt2d);
    }

  // Add image coordinates to imagePoints
  for (int i = 0; i < 4; ++i)
    {
    vgl_homg_point_2d<double> vglPt2d(this->ToPoint[i][0], this->ToPoint[i][1], 1);
    toPoints.push_back(vglPt2d);
    }

  vgl_h_matrix_2d_compute_linear algo;
  vgl_h_matrix_2d<double> homography;
  bool success = algo.compute(fromPoints, toPoints, homography);

  if (success)
    {
    return vtkVgAdapt(homography.get_matrix());
    }
  else
    {
    return 0;
    }
}
