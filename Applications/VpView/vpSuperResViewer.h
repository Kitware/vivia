/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpSuperResViewer_h
#define __vpSuperResViewer_h

// super3d includes
#include <super3d/depth/super_res.h>

// vidtk includes
#include <video_transforms/super_res.h>

// vxl includes
#include <vil/vil_image_view.h>
#include <vgl/algo/vgl_h_matrix_2d.h>

// vtk includes
#include <vtkSmartPointer.h>

// Qt includes
#include <QDialog>
#include <QCloseEvent>

// boost includes
#include <boost/shared_ptr.hpp>

// Forward declarations.
class vtkImageActor;
class vtkImageData;
class vtkInteractorStyleImage;
class vtkRenderer;
class vtkRenderWindow;
class vtkRenderWindowInteractor;

class QVTKWidget;

class vpSuperResWorker;
class vpSuperResWidget;

namespace Ui
{
class vpSuperResViewer;
}

class vpSuperResViewer : public QDialog
{
  Q_OBJECT

public:
  vpSuperResViewer(QWidget* parent);

  virtual ~vpSuperResViewer();

  void start(const std::vector<std::string>& filelist,
             const std::vector<vgl_h_matrix_2d<double> >& homographies,
             super3d::super_res_params* srp,
             bool registerImages,
             bool greyscale,
             double sensorSigma,
             const std::string& cropId,
             int i0, int j0, int ni, int nj, int ref);

  void closeEvent(QCloseEvent* event);

signals:
  void homographiesUpdated(std::string, std::vector<vgl_h_matrix_2d<double> >);

public slots:
  void updateImage(vil_image_view<double>);
  void finalizeImage(vil_image_view<vxl_byte>);
  void stopHomographyCaching();
  void finalizeRegistration();
  void receiveRefImage(vil_image_view<double>);

private slots:
  void onSaveClicked();
  void onBicubicSelected();
  void onImageSelected();

private:
  Ui::vpSuperResViewer* Ui;

  void createImageData(unsigned int ni, unsigned int nj,
                       double scaleFactor, bool greyscale,
                       vtkSmartPointer<vtkImageData>& imgData,
                       vtkSmartPointer<vtkImageActor>& imgActor);

  vtkSmartPointer<vtkRenderWindow> RenderWindow;
  vtkSmartPointer<vtkRenderer> Renderer;
  vtkSmartPointer<vtkRenderWindowInteractor> Interactor;
  vtkSmartPointer<vtkInteractorStyleImage> ImageInteractorStyle;

  vtkSmartPointer<vtkImageActor> ImageActor, BicubicImageActor;
  vtkSmartPointer<vtkImageData> ImageData, BicubicImageData;
  boost::shared_ptr<bool> Interrupted;
  vil_image_view<vxl_byte> Result, Bicubic;

  bool finalized;
  double ScaleFactor;
};

#endif // __vpSuperResViewer_h
