// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpSuperResWidget_h
#define __vpSuperResWidget_h

#include <super3d/depth/super_config.h>
#include <super3d/depth/super_res.h>

#include <vtkSmartPointer.h>
#include <vtkType.h>

#include <vgl/algo/vgl_h_matrix_2d.h>
#include <vpgl/vpgl_perspective_camera.h>

#include <QWidget>

#include <boost/shared_ptr.hpp>

#include <map>

class QListWidgetItem;

class vtkActor;
class vtkPoints;

class vpSuperResViewer;

namespace Ui
{
class vpSuperResWidget;
}

class vpViewCore;

class vpSuperResWidget : public QWidget
{
  Q_OBJECT

public:
  vpSuperResWidget(QWidget* parent = 0);
  virtual ~vpSuperResWidget();

  void initialize(vpViewCore* viewCore);

  void setBundleAjustmentConfigFile(const std::string& configFile)
    {
    this->BundleAdjustmentConfigFile = configFile;
    }

  void disableGreyscaleOption(bool disable);
  void enableOptions();

  void loadCameras(const std::string& cameraDirectory);
  void loadDepthConfigFile(const std::string& depthCfgFile);

signals:
  void invalidateHomographyCaching();

private slots:
  void updateXMin();
  void updateYMin();
  void updateBox();
  void bundleCropSelect();
  void cropSelect();
  void disable();
  void frameChanged();
  void runBundleAdjustment();
  void onBundleSelectionComplete();
  void onSelectionComplete();
  void onRunClicked();
  void updateNumImages();
  void cacheHomographies(std::string, std::vector<vgl_h_matrix_2d<double> >);
  void imageIncludeChanged(bool);
  void onImageListSelection();

private:
  void completeSelection(bool isBundleCrop);

  void readSuperResParams(super3d::super_res_params::TV_METHOD method,
                          super3d::super_res_params* srp,
                          double regularizationScale);
  void updateImageInclusion();
  QListWidgetItem* buildImageIncludeList(int currentFrame);

  void loadImagery(std::vector<std::string>& filenames);

  void updateHomographyReferenceFrameAndCamera(int newReferenceFrame);

  Ui::vpSuperResWidget* Ui;

  vpViewCore* ViewCoreInstance;
  vtkSmartPointer<vtkActor> CropRegionActor;
  vtkSmartPointer<vtkPoints> CropRegionPoints;

  std::string BundleAdjustmentConfigFile;

  int ImageWidth, ImageHeight;

  std::vector<vpgl_perspective_camera<double> > Cameras;
  std::vector<bool> Included;

  //depth config file
  boost::shared_ptr<super3d::config> Config;

  //map crop regions (as strings) to homographies
  typedef std::map<std::string, std::vector<vgl_h_matrix_2d<double> > > hCache;
  hCache CachedHomographies;

  double WorldUpperLeft[2];
  double WorldLowerRight[2];

  bool UsingBundleAdjustHomographies;
  bool SelectingSuperResCrop;
  int LastFrameChanged;
};

#endif
