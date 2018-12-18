/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpSuperResHomogWarper.h"

#include "vpSuperResViewer.h"

#include <tracking/refine_homography.h>
#include <video_transforms/super_res_utils.h>
#include <video_transforms/adjoint_image_op.h>
#include <video_transforms/warp_image.h>
#include <vil_plugins/vil_plugin_loader.h>

#include <vil/vil_load.h>
#include <vil/vil_image_view.h>
#include <vil/vil_convert.h>
#include <vil/vil_crop.h>
#include <vnl/vnl_inverse.h>
#include <vil/vil_save.h>

#include <algorithm>
#include <functional>
#include <sstream>

//-----------------------------------------------------------------------------
vpSuperResHomogWarper::
vpSuperResHomogWarper(const std::vector<vgl_h_matrix_2d<double> >& homogs,
                      bool fineRegistration, int cropi0, int cropj0,
                      int cropni, int cropnj, int ref, double scaleFactor,
                      double sensorSigma, const std::string& cropId,
                      boost::shared_ptr<bool> interrupt) :
  vpSuperResWarper(cropi0, cropj0, cropni, cropnj,
                   ref, scaleFactor, sensorSigma, interrupt)
{
  this->Homographies = homogs;
  this->RegisterImages = fineRegistration;
  this->CropId = cropId;
}

//-----------------------------------------------------------------------------
vpSuperResHomogWarper::~vpSuperResHomogWarper()
{

}

//-----------------------------------------------------------------------------
void vpSuperResHomogWarper::computeImageBounds(
  std::vector<vgl_box_2d<int> >& imgBounds)
{
  // Compute flow from initial homographies to determine cropped view to load.
  // Pad cropped region by 100 pixels to allow for homography refinement.
  std::vector<vil_image_view<double> > flows;
  vidtk::homogs_to_flows(this->Homographies, this->RefFrame,
                         this->I0, this->J0, this->Ni, this->Nj,
                         this->ScaleFactor, flows);

  //crop the flows
  vidtk::crop_boxes_and_flows(flows, imgBounds,
                              this->ScaleFactor, 200);
  const vgl_box_2d<int>& refBox = imgBounds[this->RefFrame];
  this->I0 -= refBox.min_x();
  this->J0 -= refBox.min_y();

  //Update homographies
  for (unsigned int i = 0; i < imgBounds.size(); ++i)
    {
    const vgl_box_2d<int>& bbox = imgBounds[i];
    vnl_double_3x3 hMatrix = this->Homographies[i].get_matrix();
    vidtk::crop_homography_source(hMatrix, bbox.min_x(), bbox.min_y());
    this->Homographies[i].set(hMatrix);
    }
}

//-----------------------------------------------------------------------------
bool vpSuperResHomogWarper::process(
  std::vector<vil_image_view<double> >& frames,
  std::vector<vidtk::adjoint_image_ops_func<double> >& warps)
{
  double min, max;
  vil_math_value_range(frames[0], min, max);
  if (this->RegisterImages)
    {
    emit statusChanged(tr("Fine Image Registration"));
    emit progressUpdated(0);
    if (!refineHomographies(frames))
      {
      return false;
      }
    }

  std::vector<vil_image_view<double> > flows;
  vidtk::homogs_to_flows(this->Homographies, this->RefFrame,
                         this->I0, this->J0, this->Ni, this->Nj,
                         this->ScaleFactor, flows);

  vidtk::crop_frames_and_flows(flows, frames,
                               this->ScaleFactor, 3);

  const bool downScaling = false;
  const bool bicubicWarping = false;
  vidtk::create_warps_from_flows(flows, frames, warps,
                                 this->ScaleFactor,
                                 this->SensorSigma,
                                 downScaling,
                                 bicubicWarping);

  //Normalize intensities between 0 and 1, is done after fine registration
  //because feature computation assumes 0-255
  const double normalizer = 1.0 / 255.0;
  for (unsigned int i = 0; i < frames.size(); ++i)
    {
    vil_math_scale_values(frames[i], normalizer);
    }

  emit registrationComplete();

  return true;
}

//-----------------------------------------------------------------------------
//Would be nice to eventually do this using a thread pool
bool vpSuperResHomogWarper::refineHomographies(
  const std::vector<vil_image_view<double> >& frames)
{
  vil_image_view<double> refimg;

  if (frames[this->RefFrame].nplanes() > 1)
    {
    vil_convert_planes_to_grey(frames[this->RefFrame], refimg);
    }
  else
    {
    refimg = frames[this->RefFrame];
    }

  vnl_double_3x3 sMatrix;
  sMatrix.set_identity();
  sMatrix(0, 0) = 1.0 / this->ScaleFactor;
  sMatrix(1, 1) = 1.0 / this->ScaleFactor;

  vnl_double_3x3 refH = this->Homographies[this->RefFrame].get_matrix();
  vnl_double_3x3 refHInv =
    this->Homographies[this->RefFrame].get_inverse().get_matrix();
  for (unsigned int i = 0; i < this->Homographies.size(); i++)
    {
    if (*this->Interrupted.get())
      {
      return false;
      }

    vil_image_view<double> img;

    if (frames[i].nplanes() > 1)
      {
      vil_convert_planes_to_grey(frames[i], img);
      }
    else
      {
      img = frames[i];
      }

    vnl_double_3x3 hMatrix = refHInv * this->Homographies[i].get_matrix();
    vidtk::refine_homography(refimg, img, hMatrix, 4, 50, 5);

    vnl_double_3x3 mMatrix =  vnl_inverse(hMatrix);
    vidtk::crop_homography_source(mMatrix, this->I0, this->J0);
    mMatrix *= sMatrix;

    vil_image_view<double>
      result(static_cast<unsigned int>(this->Ni * this->ScaleFactor),
             static_cast<unsigned int>(this->Nj * this->ScaleFactor),
             frames[i].nplanes());
    vidtk::warp_image(frames[i], result, mMatrix);

    emit imageUpdated(result);

    this->Homographies[i].set(hMatrix);

    double ratio = (i + 1) / static_cast<double>(this->Homographies.size());
    emit progressUpdated(static_cast<int>(100.0 * ratio));
    }

  // Save the refined homographies so that we can re run this crop region
  // without registering the imagery again
  emit homographiesUpdated(this->CropId, this->Homographies);
  return true;
}
