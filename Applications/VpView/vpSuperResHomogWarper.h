/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpSuperResHomogWarper_h
#define __vpSuperResHomogWarper_h

#include "vpSuperResWarper.h"

#include <video_transforms/super_res.h>

#include <vgl/algo/vgl_h_matrix_2d.h>

#include <vtkSmartPointer.h>

#include <qtGlobal.h>

#include <boost/shared_ptr.hpp>

#include <fstream>
#include <string>
#include <vector>

class vpSuperResHomogWarper : public vpSuperResWarper
{
  Q_OBJECT
public:

  vpSuperResHomogWarper(
    const std::vector<vgl_h_matrix_2d<double> >& homographies,
    bool fineRegistration,
    int cropi0, int cropj0, int cropni, int cropnj,
    int ref, double scaleFactor, double sensorSigma,
    const std::string& cropId,
    boost::shared_ptr<bool> interrupt);

  ~vpSuperResHomogWarper();

  virtual void computeImageBounds(
    std::vector<vgl_box_2d<int> >& imgBounds) QTE_OVERRIDE;
  virtual bool process(
    std::vector<vil_image_view<double> >& frames,
    std::vector<vidtk::adjoint_image_ops_func<double> >& warps) QTE_OVERRIDE;

signals:
  void imageUpdated(vil_image_view<double>);
  void homographiesUpdated(std::string crop,
                           std::vector<vgl_h_matrix_2d<double> >);
  void registrationComplete();

private:
  bool refineHomographies(const std::vector<vil_image_view<double> >& frames);

  std::vector<vgl_h_matrix_2d<double> > Homographies;
  bool RegisterImages;
  std::string CropId;
};

#endif
