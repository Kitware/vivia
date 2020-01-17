/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpSuperResDepthViewer_h
#define __vpSuperResDepthViewer_h

// super3d includes
#include <super3d/depth/super_config.h>
#include <super3d/depth/super_res.h>

// vxl includes
#include <vil/vil_image_view.h>
#include <vgl/algo/vgl_h_matrix_2d.h>
#include <vpgl/vpgl_perspective_camera.h>

// vtk includes
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

#include <qtGlobal.h>

// Qt includes
#include <QDialog>
#include <QCloseEvent>

// boost includes
#include <boost/shared_ptr.hpp>

// Forward declarations.
class QVTKWidget;
class vtkRenderWindow;
class vtkRenderer;
class vtkImageActor;
class vtkImageData;
class vtkActor;
class vtkPolyDataMapper;
class vpSuperResWorker;
class vpSuperResWidget;
class vtkInteractorStyleTrackballCamera;
class vtkInteractorStyleImage;
class vtkRenderWindowInteractor;
class vtkCamera;

namespace Ui
{
class vpSuperResDepthViewer;
}

class vpSuperResDepthViewer : public QDialog
{
  Q_OBJECT

public:
  vpSuperResDepthViewer(QWidget* parent);

  virtual ~vpSuperResDepthViewer();

  void start(const std::vector<std::string>& filelist,
             const std::vector<vpgl_perspective_camera<double> >& cameras,
             boost::shared_ptr<super3d::config> cfg,
             std::string depthConfigFile,
             super3d::super_res_params* srp,
             bool greyscale,
             double sensorSigma,
             double lambda,
             const std::string& cropId,
             int i0, int j0, int ni, int nj, int ref);

  virtual void closeEvent(QCloseEvent* event) QTE_OVERRIDE;

public slots:
  void updateImage(vil_image_view<double>);
  void updateDepth();
  void finalizeDepth();
  void finalizeImage(vil_image_view<vxl_byte>);
  void receiveRefImage(vil_image_view<double>);

private slots:
  void onSaveClicked();
  void onDepthSelected();
  void onBicubicSelected();
  void onImageSelected();

private:
  void createDepthPolyData(unsigned int width, unsigned int height);
  void createImageData(unsigned int ni, unsigned int nj,
                       double scaleFactor, bool greyscale,
                       vtkSmartPointer<vtkImageData>& imgData,
                       vtkSmartPointer<vtkImageActor>& imgActor);

  Ui::vpSuperResDepthViewer* Ui;

  vtkSmartPointer<vtkRenderWindow> RenderWindow;
  vtkSmartPointer<vtkRenderer> ImageRenderer;
  vtkSmartPointer<vtkRenderer> DepthRenderer;
  vtkSmartPointer<vtkRenderWindowInteractor> Interactor;
  vtkSmartPointer<vtkInteractorStyleTrackballCamera> DepthInteractorStyle;
  vtkSmartPointer<vtkInteractorStyleImage> ImageInteractorStyle;

  bool FirstDepthRender;

  vtkSmartPointer<vtkPolyData> PolyData;
  vtkSmartPointer<vtkPolyDataMapper> PolyMapper;
  vtkSmartPointer<vtkActor> PolyActor;

  vtkSmartPointer<vtkImageActor> ImageActor, BicubicImageActor;
  vtkSmartPointer<vtkImageData> ImageData, BicubicImageData;
  boost::shared_ptr<bool> Interrupted;
  vil_image_view<vxl_byte> Result, Bicubic;

  bool finalized;
  double ScaleFactor;
};

#endif // __vpSuperResViewer_h
