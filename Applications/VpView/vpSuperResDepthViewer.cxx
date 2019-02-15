/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpSuperResDepthViewer.h"

#include "ui_vpSuperResDepthViewer.h"

#include "vpSuperResWorker.h"
#include "vpSuperResDepthWarper.h"

#include <vgFileDialog.h>

// vxl includes
#include <vil/vil_image_view.h>
#include <vil/vil_load.h>
#include <vil/vil_save.h>
#include <vil/vil_convert.h>
#include <vgl/algo/vgl_h_matrix_2d.h>
#include <vnl/vnl_double_3.h>

// VTK includes
#include <QVTKWidget.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageActor.h>
#include <vtkImageProperty.h>
#include <vtkInteractorStyleImage.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataWriter.h>

// Qt includes
#include <QDebug>
#include <QFileDialog>
#include <vtkVgAdapt.h>

// boost includes
#include <boost/make_shared.hpp>

#include <super3d/depth/multiscale.h>

//-----------------------------------------------------------------------------
class ResetWindowAndLevel : public vtkCommand
{
public:
  static ResetWindowAndLevel* New()
  { return new ResetWindowAndLevel; }

  void AddProperty(vtkImageProperty* imageProperty)
    {
    this->Properties.push_back(imageProperty);
    }

  void SetRenderWindow(vtkRenderWindow* renderWindow)
    {
    this->RenderWindow = renderWindow;
    }

  virtual void Execute(vtkObject*, unsigned long, void*)
    {
    for (unsigned int i = 0; i < this->Properties.size(); i++)
      {
      this->Properties[i]->SetColorWindow(255.0);
      this->Properties[i]->SetColorLevel(127.5);
      }
    this->RenderWindow->Render();
    }

private:
  ResetWindowAndLevel() {}
  ~ResetWindowAndLevel() {}

  vtkRenderWindow* RenderWindow;
  std::vector<vtkImageProperty*> Properties;
};

//-----------------------------------------------------------------------------
vpSuperResDepthViewer::vpSuperResDepthViewer(QWidget* parent) : QDialog(parent)
{
  this->Ui = new Ui::vpSuperResDepthViewer;
  this->Ui->setupUi(this);
  this->finalized = false;

  connect(this->Ui->saveStopButton, SIGNAL(clicked()),
          this, SLOT(onSaveClicked()));
  connect(this->Ui->imageRadio, SIGNAL(clicked()),
          this, SLOT(onImageSelected()));
  connect(this->Ui->depthRadio, SIGNAL(clicked()),
          this, SLOT(onDepthSelected()));
  connect(this->Ui->bicubicRadio, SIGNAL(clicked()),
          this, SLOT(onBicubicSelected()));
}

//-----------------------------------------------------------------------------
vpSuperResDepthViewer::~vpSuperResDepthViewer()
{
  delete this->Ui;
}

//-----------------------------------------------------------------------------
void vpSuperResDepthViewer::start(
  const std::vector<std::string>& filelist,
  const std::vector<vpgl_perspective_camera<double> >& cameras,
  boost::shared_ptr<super3d::config> cfg, std::string depthConfigFile,
  super3d::super_res_params* srp,
  bool greyscale, double sensorSigma, double lambda,
  const std::string& cropId, int i0, int j0, int ni, int nj, int ref)
{
  this->Interrupted = boost::make_shared<bool>();
  *this->Interrupted = false;

  unsigned int maxIterations = 2000;
  this->ScaleFactor = srp->scale_factor;

  this->RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  this->DepthRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->ImageRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->Ui->renderImage->SetRenderWindow(this->RenderWindow);

  this->Interactor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  this->ImageInteractorStyle =
    vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->DepthInteractorStyle =
    vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();

  this->RenderWindow->SetInteractor(this->Interactor);

  this->createImageData(ni, nj, srp->scale_factor, greyscale,
                        this->ImageData, this->ImageActor);
  this->createImageData(ni, nj, srp->scale_factor, greyscale,
                        this->BicubicImageData, this->BicubicImageActor);

  this->createDepthPolyData(ni, nj);

  ResetWindowAndLevel* resetCallback = ResetWindowAndLevel::New();
  resetCallback->AddProperty(this->ImageActor->GetProperty());
  resetCallback->AddProperty(this->BicubicImageActor->GetProperty());
  resetCallback->SetRenderWindow(this->RenderWindow);
  this->ImageInteractorStyle->AddObserver(vtkCommand::ResetWindowLevelEvent,
                                          resetCallback);
  resetCallback->Delete();

  // Show depth first
  this->RenderWindow->AddRenderer(this->DepthRenderer);
  this->Interactor->SetInteractorStyle(this->DepthInteractorStyle);

  this->FirstDepthRender = true;

  vpSuperResDepthWarper* warper =
    new vpSuperResDepthWarper(cameras, cfg, depthConfigFile,
                              i0, j0, ni, nj, ref,
                              srp->scale_factor, sensorSigma, lambda, cropId,
                              this->PolyData, this->Interrupted);

  connect(warper, SIGNAL(statusChanged(QString)),
          this->Ui->label, SLOT(setText(QString)));
  connect(warper, SIGNAL(progressUpdated(int)),
          this->Ui->progressBar, SLOT(setValue(int)));
  connect(warper, SIGNAL(depthRenderRequested()), this, SLOT(updateDepth()));
  connect(warper, SIGNAL(depthCompleted()), this, SLOT(finalizeDepth()));

  // Worker now owns srp and warper
  vpSuperResWorker* worker = new vpSuperResWorker(filelist,
                                                  ref,
                                                  warper,
                                                  srp,
                                                  sensorSigma,
                                                  maxIterations,
                                                  greyscale,
                                                  this->Interrupted);
  QThread* thread = new QThread;
  worker->moveToThread(thread);
  connect(thread, SIGNAL(started()), worker, SLOT(process()));
  //TODO: figure out why these do not get called--causing a memory leak
  connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
  connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  connect(worker, SIGNAL(statusChanged(QString)),
          this->Ui->label, SLOT(setText(QString)));
  connect(worker, SIGNAL(progressUpdated(int)),
          this->Ui->progressBar, SLOT(setValue(int)));
  connect(worker, SIGNAL(imageUpdated(vil_image_view<double>)),
          this, SLOT(updateImage(vil_image_view<double>)));
  connect(worker, SIGNAL(imageCompleted(vil_image_view<vxl_byte>)),
          this, SLOT(finalizeImage(vil_image_view<vxl_byte>)));
  connect(worker, SIGNAL(sendRefImage(vil_image_view<double>)),
          this, SLOT(receiveRefImage(vil_image_view<double>)));
  thread->start();
}

//-----------------------------------------------------------------------------
void vpSuperResDepthViewer::createDepthPolyData(unsigned int width,
                                                unsigned int height)
{
  vtkNew<vtkPoints> pts;
  vtkNew<vtkCellArray> cells;
  vtkNew<vtkUnsignedCharArray> colors;

  colors->SetName("colors");
  colors->SetNumberOfComponents(3);

  vtkIdType tri[3];

  for (unsigned int j = 0; j < height; j++)
    {
    for (unsigned int i = 0; i < width; i++)
      {
      vnl_double_3 pt3d(0.0, 0.0, 0.0);
      pts->InsertNextPoint(pt3d.data_block());
      colors->InsertNextTuple3(255, 0, 255);

      if (i != 0 && j != 0)
        {
        tri[0] = j*width + i;
        tri[1] = (j - 1)*width + (i - 1);
        tri[2] = (j - 1)*width + i;
        cells->InsertNextCell(3, tri);

        tri[1] = j*width + (i - 1);
        tri[2] = (j - 1)*width + (i - 1);
        cells->InsertNextCell(3, tri);
        }
      }
    }

  this->PolyData = vtkSmartPointer<vtkPolyData>::New();
  this->PolyData->SetPoints(pts.GetPointer());
  this->PolyData->SetPolys(cells.GetPointer());
  this->PolyData->GetPointData()->SetScalars(colors.GetPointer());

  this->PolyMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->PolyMapper->SetInputData(this->PolyData);
  this->PolyActor = vtkSmartPointer<vtkActor>::New();
  this->PolyActor->SetMapper(this->PolyMapper);
  this->PolyActor->GetProperty()->SetColor(1.0f, 1.0f, 1.0f);

  this->DepthRenderer->AddActor(this->PolyActor);
}

//-----------------------------------------------------------------------------
void vpSuperResDepthViewer::createImageData(
  unsigned int ni, unsigned int nj, double scaleFactor, bool greyscale,
  vtkSmartPointer<vtkImageData> &imgData,
  vtkSmartPointer<vtkImageActor> &imgActor)
{
  // Create image data and zero it out
  imgData = vtkSmartPointer<vtkImageData>::New();
  imgData->SetDimensions(static_cast<double>(scaleFactor * ni),
                         static_cast<double>(scaleFactor * nj), 1);
  unsigned int nComponents = greyscale ? 1 : 3;
  imgData->AllocateScalars(VTK_DOUBLE, nComponents);
  double* vtkptr = static_cast<double*>(imgData->GetScalarPointer(0, 0, 0));
  std::fill_n(vtkptr,
              imgData->GetPointData()->GetScalars()->GetNumberOfTuples(),
              0.0);

  imgActor = vtkSmartPointer<vtkImageActor>::New();
  imgActor->SetInputData(imgData);
}

//-----------------------------------------------------------------------------
void vpSuperResDepthViewer::updateImage(vil_image_view<double> img)
{
  vtkVgAdapt(img, ImageData);
  this->ImageActor->GetInput()->Modified();
  this->RenderWindow->Render();
}

//-----------------------------------------------------------------------------
void vpSuperResDepthViewer::closeEvent(QCloseEvent* event)
{
  *this->Interrupted = true;
  event->accept();
}

//-----------------------------------------------------------------------------
void vpSuperResDepthViewer::finalizeImage(vil_image_view<vxl_byte> img)
{
  this->Result = img;
  this->finalized = true;
  this->Ui->saveStopButton->setEnabled(true);
  this->Ui->saveStopButton->setText(QString("Save As..."));
  this->Ui->progressBar->setValue(100);
}

//-----------------------------------------------------------------------------
void vpSuperResDepthViewer::updateDepth()
{
  this->PolyData->Modified();
  this->RenderWindow->Render();
  if (this->FirstDepthRender)
    {
    this->FirstDepthRender = false;
    this->DepthRenderer->ResetCamera();
    vtkCamera* camera = this->DepthRenderer->GetActiveCamera();
    camera->Zoom(2.6);
    camera->SetViewUp(0, -1, 0);
    this->RenderWindow->Render();
    }
}

//-----------------------------------------------------------------------------
void vpSuperResDepthViewer::finalizeDepth()
{
  this->PolyData->Modified();
  this->RenderWindow->Render();
  this->Ui->saveStopButton->setText(QString("Save As..."));
  this->Ui->depthRadio->setEnabled(true);
  this->Ui->imageRadio->setEnabled(true);
  this->Ui->bicubicRadio->setEnabled(true);
  this->Ui->saveStopButton->setEnabled(true);
}

//-----------------------------------------------------------------------------
void vpSuperResDepthViewer::receiveRefImage(vil_image_view<double> img)
{
  vil_image_view<double> bicubic;
  super3d::upsample(img, bicubic, this->ScaleFactor,
                    vidtk::warp_image_parameters::CUBIC);
  vtkVgAdapt(bicubic, this->BicubicImageData);
  this->BicubicImageActor->GetInput()->Modified();
  vil_convert_cast(bicubic, Bicubic);

  vil_image_view<vxl_byte> ref;
  vil_convert_cast(img, ref);
  vtkIdType index = 0;
  vtkUnsignedCharArray* colors =
    static_cast<vtkUnsignedCharArray*>(
      this->PolyData->GetPointData()->GetScalars());
  for (unsigned int j = 0; j < ref.nj(); j++)
    {
    for (unsigned int i = 0; i < ref.ni(); i++)
      {
      if (ref.nplanes() == 1)
        {
        unsigned char v = ref(i,j);
        colors->SetTuple3(index++, v, v, v);
        }
      else
        {
        colors->SetTuple3(index++, ref(i,j,0), ref(i,j,1), ref(i,j,2));
        }
      }
    }
  this->PolyData->Modified();
}

//-----------------------------------------------------------------------------
void vpSuperResDepthViewer::onSaveClicked()
{
  if (this->Ui->bicubicRadio->isChecked())
    {
    QString filename =
      vgFileDialog::getSaveFileName(this, "Save bicubic image",
                                    QDir::currentPath(),
                                    "PNG Images (*.png)");
    if (!filename.isEmpty())
      {
      vil_save(Bicubic, qPrintable(filename));
      }
    }
  else if (this->Ui->imageRadio->isChecked())
    {
    if (!this->finalized)
      {
      *this->Interrupted = true;
      this->Ui->saveStopButton->setEnabled(false);
      }
    else
      {
      // Save is disabled until we have an image
      QString filename =
        vgFileDialog::getSaveFileName(this, "Save super res image",
                                      QDir::currentPath(),
                                      "PNG Images (*.png)");
      if (!filename.isEmpty())
        {
        vil_save(this->Result, qPrintable(filename));
        }
      }
    }
  else
    {
    QString filename =
      vgFileDialog::getSaveFileName(this, "Save depth",
                                    QDir::currentPath(),
                                    "VTK Polygonal Data (*.vtp)");

    if (!filename.isEmpty())
      {
      vtkSmartPointer<vtkXMLPolyDataWriter> writer =
        vtkSmartPointer<vtkXMLPolyDataWriter>::New();
      writer->SetInputData(this->PolyData);
      writer->SetFileName(qPrintable(filename));
      writer->Update();
      }
    }
}

//-----------------------------------------------------------------------------
void vpSuperResDepthViewer::onDepthSelected()
{
  this->Ui->saveStopButton->setText(QString("Save As..."));
  this->Interactor->SetInteractorStyle(this->DepthInteractorStyle);
  this->RenderWindow->RemoveRenderer(this->ImageRenderer);
  this->RenderWindow->AddRenderer(this->DepthRenderer);
  this->RenderWindow->Render();
}

//-----------------------------------------------------------------------------
void vpSuperResDepthViewer::onImageSelected()
{
  if (this->finalized)
    {
    this->Ui->saveStopButton->setText(QString("Save As..."));
    }
  else
    {
    this->Ui->saveStopButton->setText(QString("Stop"));
    }

  this->Interactor->SetInteractorStyle(this->ImageInteractorStyle);

  // Copy over window / level settings
  this->ImageActor->GetProperty()->SetColorWindow(
    this->BicubicImageActor->GetProperty()->GetColorWindow());
  this->ImageActor->GetProperty()->SetColorLevel(
    this->BicubicImageActor->GetProperty()->GetColorLevel());

  this->RenderWindow->RemoveRenderer(this->DepthRenderer);
  this->RenderWindow->AddRenderer(this->ImageRenderer);
  this->ImageRenderer->RemoveViewProp(this->BicubicImageActor);
  this->ImageRenderer->AddViewProp(this->ImageActor);
  this->RenderWindow->Render();
}

//-----------------------------------------------------------------------------
void vpSuperResDepthViewer::onBicubicSelected()
{
  this->Ui->saveStopButton->setText(QString("Save As..."));
  this->Interactor->SetInteractorStyle(this->ImageInteractorStyle);

  // Copy over window / level settings
  this->BicubicImageActor->GetProperty()->SetColorWindow(
    this->ImageActor->GetProperty()->GetColorWindow());
  this->BicubicImageActor->GetProperty()->SetColorLevel(
    this->ImageActor->GetProperty()->GetColorLevel());

  this->RenderWindow->RemoveRenderer(this->DepthRenderer);
  this->RenderWindow->AddRenderer(this->ImageRenderer);
  this->ImageRenderer->RemoveViewProp(this->ImageActor);
  this->ImageRenderer->AddViewProp(this->BicubicImageActor);
  this->RenderWindow->Render();
}
