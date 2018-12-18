/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpSuperResWorker.h"
#include "vpSuperResWarper.h"

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

#include <vil_plugins/vil_plugin_loader.h>

#include <QDebug>

#include <algorithm>
#include <functional>
#include <sstream>

vpSuperResWorker::vpSuperResWorker(std::vector<std::string> imageNames,
                                   int ref,
                                   vpSuperResWarper* warper,
                                   super3d::super_res_params* params,
                                   double sensorBlur,
                                   unsigned int maxIterations,
                                   bool useGreyscale,
                                   boost::shared_ptr<bool> interrupt)
{
  this->Filelist = imageNames;
  this->RefFrame = ref;
  this->Warper.reset(warper);
  this->Srp = params;
  this->SensorSigma = sensorBlur;
  this->Iterations = maxIterations;
  this->Srm = 0;
  this->Greyscale = useGreyscale;
  this->Interrupted = interrupt;

  vidtk::load_vil_plugins();
}

//-----------------------------------------------------------------------------
vpSuperResWorker::~vpSuperResWorker()
{
  delete this->Srp;
  if (this->Srm)
    {
    delete this->Srm;
    }
}

//-----------------------------------------------------------------------------
void vpSuperResWorker::receiveData(super3d::super_res_monitor::update_data d)
{
  emit progressUpdated(
    static_cast<int>((100.0 * d.num_iterations) / this->Iterations));
  vil_image_view<double> dest;
  vil_convert_stretch_range_limited(d.current_result, dest, 0.0, 1.0, 0, 255);
  emit imageUpdated(dest);
}

//-----------------------------------------------------------------------------
void vpSuperResWorker::process()
{
  if (this->Frames.empty())
    {
    emit statusChanged(tr("Loading Image Files"));
    emit progressUpdated(0);

    std::vector<vil_image_resource_sptr> resources;
    std::vector<vgl_box_2d<int> > imgBounds;
    for (unsigned int i = 0; i < this->Filelist.size(); i++)
      {
      vil_image_resource_sptr img = vil_load_image_resource(Filelist[i].c_str());
      resources.push_back(img);
      vgl_box_2d<int> bBox(0, img->ni() - 1,
                           0, img->nj() - 1);
      imgBounds.push_back(bBox);
      double ratio = (i + 1) / static_cast<double>(this->Filelist.size());
      emit progressUpdated(static_cast<int>(100.0 * ratio));
      }

    emit statusChanged(tr("Decoding Data"));
    emit progressUpdated(0);

    this->Warper->computeImageBounds(imgBounds);

    for (unsigned int i = 0; i < resources.size(); ++i)
      {
      const vgl_box_2d<int>& bBox = imgBounds[i];
      vil_image_view<double> img =
        vil_convert_cast(double(),
                         resources[i]->get_copy_view(bBox.min_x(), bBox.width(),
                                                     bBox.min_y(), bBox.height()));
      if (this->Greyscale && img.nplanes() == 3)
        {
        vil_image_view<double> copy(img);
        vil_convert_planes_to_grey(copy, img);
        }

      imgBounds[i] = vgl_box_2d<int>(0, bBox.width(), 0, bBox.height());

      this->Frames.push_back(img);
      double ratio = (i + 1) / static_cast<double>(this->Filelist.size());
      emit progressUpdated(static_cast<int>(100.0 * ratio));
      }
    }

  this->normalizeImages(this->Frames);

  // Compute bicubic upsample of reference frame for comparison
  vil_image_view<double> ref;
  ref.deep_copy(vil_crop(this->Frames[this->RefFrame],
                         this->Warper->i0(), this->Warper->ni(),
                         this->Warper->j0(), this->Warper->nj()));
  emit sendRefImage(ref);

  std::vector<vidtk::adjoint_image_ops_func<double> > warps;
  if (!this->Warper->process(this->Frames, warps))
    {
    emit finished();
    return;
    }

  this->Srp->s_ni = warps[this->RefFrame].src_ni();
  this->Srp->s_nj = warps[this->RefFrame].src_nj();
  this->Srp->l_ni = warps[this->RefFrame].dst_ni();
  this->Srp->l_nj = warps[this->RefFrame].dst_nj();

  emit statusChanged(tr("Super Resolving"));
  emit progressUpdated(0);

  boost::function<void (super3d::super_res_monitor::update_data)> f;
  f = std::bind1st(std::mem_fun(&vpSuperResWorker::receiveData), this);
  this->Srm = new super3d::super_res_monitor(f, 10, this->Interrupted);

  vil_image_view<double> super_u(this->Srp->s_ni, this->Srp->s_nj,
                                 this->Frames[this->RefFrame].nplanes());
  super_u.fill(0.0);
  emit imageUpdated(super_u);

  std::vector<vil_image_view<double> > a;
  super3d::super_resolve_robust(this->Frames, warps, super_u, *this->Srp,
                                this->Iterations, a, "", this->Srm);

  vil_image_view<vxl_byte> result;
  vil_convert_stretch_range_limited(super_u, result, 0.0, 1.0, 0, 255);
  emit imageCompleted(result);
  emit statusChanged(tr("Finished"));

  emit finished();
}

//-----------------------------------------------------------------------------
void vpSuperResWorker::normalizeImages(
  std::vector<vil_image_view<double> >& frames)
{
  for (size_t i = 0; i < frames.size(); ++i)
    {
    double minV, maxV;
    vil_math_value_range(frames[i], minV, maxV);

    // detect the bit-depth of the source image and scale to [0, 255]
    long int iScale = 256;
    const long int maxScale = 1 << 24;
    while (iScale <= maxScale && maxV > static_cast<double>(iScale - 1))
      {
      iScale <<= 1;
      }

    double intensityScale = 255.0 / (iScale - 1);
    vil_math_scale_values(frames[i], intensityScale);
    }
}
