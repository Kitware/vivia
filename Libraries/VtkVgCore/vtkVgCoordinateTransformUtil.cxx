
#include "vtkVgCoordinateTransformUtil.h"

#include "vtkVgAdapt.h"

#include <vtkMatrix4x4.h>

#include <vgl/vgl_homg_point_2d.h>
#include <vgl/algo/vgl_h_matrix_2d.h>
#include <vgl/algo/vgl_h_matrix_2d_compute_linear.h>


vtkSmartPointer<vtkMatrix4x4>
  vtkVgCoordinateTransformUtil::ComputeImageToLatLonMatrix(
  double llx, double lly,
  double urx, double ury,
  double geollx, double geolly,
  double geolrx, double geolry,
  double geourx, double geoury,
  double geoulx, double geouly)
{
  vcl_vector<vgl_homg_point_2d<double> > toPoints;
  toPoints.push_back(vgl_homg_point_2d<double>(geoulx, geouly, 1));
  toPoints.push_back(vgl_homg_point_2d<double>(geourx, geoury, 1));
  toPoints.push_back(vgl_homg_point_2d<double>(geolrx, geolry, 1));
  toPoints.push_back(vgl_homg_point_2d<double>(geollx, geolly, 1));

  vcl_vector<vgl_homg_point_2d<double> > fromPoints;
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

  vtkGenericWarningMacro("Failed to compute Image to Lat/Lon matrix!");
  return 0;
}
