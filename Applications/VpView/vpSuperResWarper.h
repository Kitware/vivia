/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpSuperResWarper_h
#define __vpSuperResWarper_h

#include <video_transforms/super_res.h>

#include <vgl/algo/vgl_h_matrix_2d.h>

// Qt includes
#include <QThread>

#include <boost/shared_ptr.hpp>

#include <fstream>
#include <string>
#include <vector>

// An abstract class for techniques that computes a per pixel warping from many
// frames to a reference frame has emits signals for displaying progress
class vpSuperResWarper : public QObject
{
  Q_OBJECT
public:

  vpSuperResWarper(int cropi0, int cropj0,
                   int cropni, int cropnj,
                   int ref, double scaleFactor,
                   double sensorSigma,
                   boost::shared_ptr<bool> interrupt) :
    I0(cropi0), J0(cropj0), Ni(cropni), Nj(cropnj),
    RefFrame(ref), ScaleFactor(scaleFactor),
    SensorSigma(sensorSigma),
    Interrupted(interrupt)
    {}

  virtual ~vpSuperResWarper() {}

  virtual void computeImageBounds(std::vector<vgl_box_2d<int> >& imgBounds) {}

  // Also normalizes intensities between 0 and 1
  virtual bool process(
    std::vector<vil_image_view<double> >& frames,
    std::vector<vidtk::adjoint_image_ops_func<double> >& warps) = 0;

  int i0() const { return this->I0; }
  int j0() const { return this->J0; }
  int ni() const { return this->Ni; }
  int nj() const { return this->Nj; }

signals:
  void statusChanged(QString status);
  void progressUpdated(int progress);

protected:

  int I0, J0, Ni, Nj;
  int RefFrame;
  double ScaleFactor, SensorSigma;
  boost::shared_ptr<bool> Interrupted;
};

#endif
