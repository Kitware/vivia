/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpSuperResDepthWarper_h
#define __vpSuperResDepthWarper_h

#include "vpSuperResWarper.h"

#include <super3d/depth/super_config.h>
#include <super3d/depth/tv_refine_search.h>
#include <super3d/depth/world_space.h>

#include <video_transforms/super_res.h>

#include <vgl/algo/vgl_h_matrix_2d.h>
#include <vpgl/vpgl_perspective_camera.h>

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include <qtGlobal.h>

#include <QThread>

#include <boost/shared_ptr.hpp>

#include <fstream>
#include <string>
#include <vector>

class vpSuperResDepthWarper : public vpSuperResWarper
{
  Q_OBJECT
public:

  vpSuperResDepthWarper(
    const std::vector<vpgl_perspective_camera<double> >& cameras,
    boost::shared_ptr<super3d::config> cfg,
    std::string depthConfigFile,
    int cropi0, int cropj0, int cropni, int cropnj,
    int ref, double scaleFactor, double sensorSigma, double lambda,
    const std::string& cropId, vtkSmartPointer<vtkPolyData> polydata,
    boost::shared_ptr<bool> interrupt);

  ~vpSuperResDepthWarper();

  virtual bool process(
    std::vector<vil_image_view<double> >& frames,
    std::vector<vidtk::adjoint_image_ops_func<double> >& warps) QTE_OVERRIDE;

  void receiveData(super3d::depth_refinement_monitor::update_data d);

signals:
   void depthRenderRequested();
   void depthCompleted();

private:
  bool computeDepth(std::vector<vil_image_view<double> >& frames,
                    vil_image_view<double>& depth);

  void createWarpsFromFlows(
    const std::vector<vil_image_view<double> >& flows,
    const std::vector<vil_image_view<double> >& frames,
    const std::vector<vil_image_view<double> >& weights,
    std::vector<vidtk::adjoint_image_ops_func<double> >& warps);

  void setPointsFromDepth(const vil_image_view<double>& depth);

  std::vector<vpgl_perspective_camera<double> > Cameras;
  unsigned int Iterations;
  boost::shared_ptr<super3d::config> Config;

  std::string CropId;
  vtkSmartPointer<vtkPolyData> PolyData;
  std::string DepthConfigFile;

  double Lambda;

  super3d::world_space* WorldSpace;

  super3d::depth_refinement_monitor* Drm;
};

#endif
