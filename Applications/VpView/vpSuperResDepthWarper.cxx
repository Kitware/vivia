// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpSuperResDepthWarper.h"

#include "vpProjectParser.h"
#include "vpSuperResDepthViewer.h"

#include <super3d/depth/super_config.h>
#include <super3d/depth/flow_manip.h>
#include <super3d/depth/world_frustum.h>
#include <super3d/depth/cost_volume.h>
#include <super3d/depth/multiscale.h>
#include <super3d/depth/tv_refine_search.h>
#include <super3d/depth/weighted_dbw.h>

#include <tracking/refine_homography.h>
#include <video_transforms/super_res_utils.h>
#include <video_transforms/adjoint_image_op.h>
#include <video_transforms/warp_image.h>

#include <vil/vil_load.h>
#include <vil/vil_image_view.h>
#include <vil/vil_convert.h>
#include <vil/vil_crop.h>
#include <vil/vil_resample_bilin.h>
#include <vil/vil_save.h>
#include <vnl/vnl_inverse.h>

#include <vtkPoints.h>

#include <qtStlUtil.h>

#include <QDebug>

#include <boost/scoped_ptr.hpp>

//-----------------------------------------------------------------------------
vpSuperResDepthWarper::vpSuperResDepthWarper(
  const std::vector<vpgl_perspective_camera<double> >& cameras,
  boost::shared_ptr<super3d::config> cfg, std::string depthConfigFile,
  int cropi0, int cropj0, int cropni, int cropnj, int ref, double scaleFactor,
  double sensorSigma, double lambda, const std::string& cropId,
  vtkSmartPointer<vtkPolyData> polydata, boost::shared_ptr<bool> interrupt) :
    vpSuperResWarper(cropi0, cropj0, cropni, cropnj,
                     ref, scaleFactor, sensorSigma, interrupt)
{
  this->Cameras = cameras;
  this->Drm = 0;
  this->WorldSpace = 0;
  this->Config = cfg;
  this->DepthConfigFile = depthConfigFile;
  this->Iterations = cfg->get_value<unsigned int>("depth_iterations");
  this->PolyData = polydata;
  this->Lambda = lambda;
}

//-----------------------------------------------------------------------------
vpSuperResDepthWarper::~vpSuperResDepthWarper()
{
  if (this->Drm)
    {
    delete this->Drm;
    }

  if (this->WorldSpace)
    {
    delete this->WorldSpace;
    }
}

//-----------------------------------------------------------------------------
void vpSuperResDepthWarper::setPointsFromDepth(
  const vil_image_view<double>& depth)
{
  vtkPoints* pts = this->PolyData->GetPoints();
  vtkIdType index = 0;
  for (unsigned int j = 0; j < depth.nj(); ++j)
    {
    for (unsigned int i = 0; i < depth.ni(); ++i)
      {
      vnl_double_3 pt3d = this->WorldSpace->point_at_depth(i, j, depth(i, j));
      pts->SetPoint(index++, pt3d.data_block());
      }
    }
}

//-----------------------------------------------------------------------------
void vpSuperResDepthWarper::receiveData(
  super3d::depth_refinement_monitor::update_data d)
{
  emit progressUpdated(
    static_cast<int>((100.0 * d.num_iterations) / this->Iterations));
  this->setPointsFromDepth(d.current_result);
  emit depthRenderRequested();
}

//-----------------------------------------------------------------------------
bool vpSuperResDepthWarper::
process(std::vector<vil_image_view<double> >& frames,
        std::vector<vidtk::adjoint_image_ops_func<double> >& warps)
{
  try
    {
    // Normalize intensities between 0 and 1
    std::vector<vil_image_view<double> > greyFrames(frames.size());
    const double normalizer = 1.0 / 255.0;
    for (unsigned int i = 0; i < frames.size(); ++i)
      {
      vil_math_scale_values(frames[i], normalizer);
      if (frames[i].nplanes() == 1)
        {
        greyFrames[i].deep_copy(frames[i]);
        }
      else
        {
        vil_convert_planes_to_grey(frames[i], greyFrames[i]);
        }
      }

    std::vector<vpgl_perspective_camera<double> > scaledCameras;
    for (unsigned int i = 0; i < this->Cameras.size(); ++i)
      {
      scaledCameras.push_back(super3d::scale_camera(this->Cameras[i],
                                                    this->ScaleFactor));
      }

    vil_image_view<double> depth;
    if (!this->computeDepth(greyFrames, depth))
      {
      return false;
      }

    unsigned int dni =
      static_cast<unsigned int>(this->ScaleFactor * depth.ni());
    unsigned int dnj =
      static_cast<unsigned int>(this->ScaleFactor * depth.nj()) ;
    vil_image_view<double> temp;
    vil_resample_bilin(depth, temp, dni, dnj);
    depth = temp;

    int i0, ni, j0, nj;

    vpgl_perspective_camera<double> refCamera = scaledCameras[this->RefFrame];

    i0 = static_cast<int>(this->I0 * this->ScaleFactor);
    j0 = static_cast<int>(this->J0 * this->ScaleFactor);
    ni = static_cast<int>(this->Ni * this->ScaleFactor);
    nj = static_cast<int>(this->Nj * this->ScaleFactor);

    refCamera = super3d::crop_camera(refCamera, i0, j0);

    std::vector<vil_image_view<double> > weights(scaledCameras.size());

    for (unsigned int i = 0; i < scaledCameras.size(); ++i)
      {
      weights[i].set_size(dni, dnj);
      weights[i].fill(1.0);
      }

    std::vector<vil_image_view<double> > flows;
    super3d::compute_occluded_flows_from_depth(scaledCameras, refCamera, depth,
                                               flows);
    vidtk::crop_frames_and_flows(flows, frames, this->ScaleFactor, 3);
    this->createWarpsFromFlows(flows, frames, weights, warps);
    }
  catch (const super3d::config::cfg_exception& e)
    {
    emit statusChanged(e.what());
    emit progressUpdated(0);
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vpSuperResDepthWarper::computeDepth(
  std::vector<vil_image_view<double> >& frames, vil_image_view<double>& depth)
{
  try
    {
    emit statusChanged(tr("Estimating Depth Costs"));
    emit progressUpdated(0);

    frames[this->RefFrame] = vil_crop(frames[this->RefFrame],
                                      this->I0, this->Ni, this->J0, this->Nj);
    this->Cameras[this->RefFrame] =
      super3d::crop_camera(this->Cameras[this->RefFrame], this->I0, this->J0);

    double depthMin, depthMax;
    if (this->Config->is_set("depth_landmarks_path"))
      {
      std::string dir = this->DepthConfigFile.substr(0,
        this->DepthConfigFile.find_last_of("/\\"));
      dir = dir.append("/");
      std::string landmark_file =
        this->Config->get_value<std::string>("depth_landmarks_path");

      vpProjectParser::ConstructCompletePath(landmark_file, dir);
      qDebug() << "Using landmarks from" << qtString(landmark_file);
      if (!super3d::compute_depth_range(this->Cameras[this->RefFrame], 0,
                                        this->Ni, 0, this->Nj, landmark_file,
                                        depthMin, depthMax))
        {
        emit statusChanged("Not enough landmarks in selected region"
                           " to estimate depth range."
                           " Please consider a larger region.");
        emit progressUpdated(0);
        return false;
        }
      qDebug() << "Depth range:" << depthMin << depthMax;
      }
    else
      {
      depthMin = this->Config->get_value<double>("depth_min");
      depthMax = this->Config->get_value<double>("depth_max");
      }

    this->WorldSpace =
      new super3d::world_frustum(this->Cameras[this->RefFrame], depthMin,
                                 depthMax, this->Ni, this->Nj);

    unsigned int S = this->Config->get_value<unsigned int>("depth_num_slices");
    double theta0 = this->Config->get_value<double>("depth_theta_start");
    double theta_end = this->Config->get_value<double>("depth_theta_end");
    double beta = this->Config->get_value<double>("depth_beta");
    double gw_alpha = this->Config->get_value<double>("depth_gw_alpha");
    double epsilon = this->Config->get_value<double>("depth_epsilon");

    vil_image_view<double> g;
    vil_image_view<double> cost_volume;

    double iw = 0.0, gw = 0.0, cw = 0.0;
    gw = this->Config->get_value<double>("depth_gradient_weight");
    iw = this->Config->get_value<double>("depth_intensity_weight");

    super3d::compute_world_cost_volume(frames, this->Cameras, this->WorldSpace,
                                       this->RefFrame, S, cost_volume, iw, gw, cw);
    this->WorldSpace->compute_g(frames[this->RefFrame], g, gw_alpha, 1.0);
    emit progressUpdated(100);

    emit statusChanged(tr("Estimating Depth"));
    emit progressUpdated(0);

    depth.set_size(cost_volume.ni(), cost_volume.nj(), 1);

    boost::function<void (super3d::depth_refinement_monitor::update_data)> f;
    f = std::bind1st(std::mem_fun(&vpSuperResDepthWarper::receiveData), this);
    this->Drm = new super3d::depth_refinement_monitor(f, 20, this->Interrupted);

    super3d::refine_depth(cost_volume, g, depth, this->Iterations, theta0,
                          theta_end, this->Lambda, epsilon, this->Drm);
    if (*this->Interrupted)
      {
      return false;
      }

    this->setPointsFromDepth(depth);
    emit depthCompleted();
    double depthScale = depthMax - depthMin;
    vil_math_scale_and_offset_values(depth, depthScale, depthMin);
    }
  catch (const super3d::config::cfg_exception& e)
    {
    emit statusChanged(e.what());
    emit progressUpdated(0);
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
void vpSuperResDepthWarper::createWarpsFromFlows(
  const std::vector<vil_image_view<double> >& flows,
  const std::vector<vil_image_view<double> >& frames,
  const std::vector<vil_image_view<double> >& weights,
  std::vector<vidtk::adjoint_image_ops_func<double> >& warps)
{
  assert(flows.size() == frames.size());
  bool downSampleAveraging =
    this->Config->get_value<bool>("down_sample_averaging");
  bool bicubicWarping = this->Config->get_value<bool>("bicubic_warping");

  warps.clear();
  warps.reserve(flows.size());
  for (unsigned int i=0; i<flows.size(); ++i)
    {
    warps.push_back(super3d::create_dbw_from_flow(flows[i],
                                                  weights[i],
                                                  frames[i].ni(),
                                                  frames[i].nj(),
                                                  frames[i].nplanes(),
                                                  this->ScaleFactor,
                                                  this->SensorSigma,
                                                  downSampleAveraging,
                                                  bicubicWarping));
    }
}
