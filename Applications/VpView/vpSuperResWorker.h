// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpSuperResWorker_h
#define __vpSuperResWorker_h

#include <super3d/depth/super_res.h>

#include <vgl/algo/vgl_h_matrix_2d.h>

#include <vtkSmartPointer.h>

#include <QScopedPointer>
#include <QThread>

#include <boost/shared_ptr.hpp>

#include <fstream>
#include <string>
#include <vector>

class vpSuperResWarper;

class vpSuperResWorker : public QObject
{
  Q_OBJECT
public:

  vpSuperResWorker(std::vector<std::string> imageNames,
                   int ref,
                   vpSuperResWarper* warper,
                   super3d::super_res_params* params,
                   double sensorBlur,
                   unsigned int maxIterations,
                   bool useGreyscale,
                   boost::shared_ptr<bool> interrupt);

  ~vpSuperResWorker();

  void receiveData(super3d::super_res_monitor::update_data data);

public slots:
  void process();

signals:
  void finished();
  void statusChanged(QString status);
  void progressUpdated(int progress);
  void imageUpdated(vil_image_view<double>);
  void imageCompleted(vil_image_view<vxl_byte>);
  void sendRefImage(vil_image_view<double>);

private:

  void normalizeImages(std::vector<vil_image_view<double> >& frames);

  std::vector<std::string> Filelist;
  std::vector<vil_image_view<double> > Frames;
  QScopedPointer<vpSuperResWarper> Warper;
  super3d::super_res_params* Srp;
  bool Greyscale;
  int RefFrame, Iterations;
  double SensorSigma;
  boost::shared_ptr<bool> Interrupted;
  super3d::super_res_monitor* Srm;
};

#endif
