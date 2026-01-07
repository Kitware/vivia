// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpSuperResViewer.h"

#include "ui_vpSuperResViewer.h"

#include "vpSuperResHomogWarper.h"
#include "vpSuperResWorker.h"

// VisGUI includes
#include <vtkVgAdapt.h>

#include <vgFileDialog.h>

// vxl includes
#include <vil/vil_image_view.h>
#include <vil/vil_load.h>
#include <vil/vil_save.h>
#include <vil/vil_convert.h>
#include <vgl/algo/vgl_h_matrix_2d.h>

// VTK includes
#include <QVTKWidget.h>
#include <vtkDataArray.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkPointData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyleImage.h>

// Qt includes
#include <QDebug>
#include <QFileDialog>

// boost includes
#include <boost/make_shared.hpp>

#include <super3d/depth/multiscale.h>

//-----------------------------------------------------------------------------
class ResetWindowAndLevel: public vtkCommand
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
vpSuperResViewer::vpSuperResViewer(QWidget* parent) : QDialog(parent)
{
  this->Ui = new Ui::vpSuperResViewer;
  this->Ui->setupUi(this);
  this->finalized = false;

  connect(this->Ui->saveStopButton, SIGNAL(clicked()),
          this, SLOT(onSaveClicked()));
  connect(this->Ui->imageRadio, SIGNAL(clicked()),
          this, SLOT(onImageSelected()));
  connect(this->Ui->bicubicRadio, SIGNAL(clicked()),
          this, SLOT(onBicubicSelected()));
}

//-----------------------------------------------------------------------------
vpSuperResViewer::~vpSuperResViewer()
{
  delete this->Ui;
}

//-----------------------------------------------------------------------------
void vpSuperResViewer::start(const std::vector<std::string>& filelist,
                             const std::vector<vgl_h_matrix_2d<double> >& hmgs,
                             super3d::super_res_params* srp,
                             bool registerImages,
                             bool greyscale,
                             double sensorSigma,
                             const std::string& cropId,
                             int i0, int j0, int ni, int nj, int ref)
{
  this->Interrupted = boost::make_shared<bool>();
  *this->Interrupted = false;

  const unsigned int maxIterations = 2000;
  this->ScaleFactor = srp->scale_factor;

  this->RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->Ui->renderImage->SetRenderWindow(this->RenderWindow);

  this->Interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  this->ImageInteractorStyle = vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->Interactor->SetInteractorStyle(this->ImageInteractorStyle);
  this->RenderWindow->SetInteractor(this->Interactor);

  this->RenderWindow->AddRenderer(this->Renderer);

  this->createImageData(ni, nj, srp->scale_factor, greyscale,
                        this->ImageData, this->ImageActor);
  this->createImageData(ni, nj, srp->scale_factor, greyscale,
                        this->BicubicImageData, this->BicubicImageActor);

  ResetWindowAndLevel* resetCallback = ResetWindowAndLevel::New();
  resetCallback->AddProperty(this->ImageActor->GetProperty());
  resetCallback->AddProperty(this->BicubicImageActor->GetProperty());
  resetCallback->SetRenderWindow(this->RenderWindow);
  this->ImageInteractorStyle->AddObserver(vtkCommand::ResetWindowLevelEvent, resetCallback);
  resetCallback->Delete();

  this->Renderer->AddViewProp(this->ImageActor);

  vpSuperResHomogWarper *warper =
    new vpSuperResHomogWarper(hmgs, registerImages, i0, j0,
                              ni, nj, ref, srp->scale_factor,
                              sensorSigma, cropId,
                              this->Interrupted);

  connect(warper, SIGNAL(statusChanged(QString)),
          this->Ui->label, SLOT(setText(QString)));
  connect(warper, SIGNAL(progressUpdated(int)),
          this->Ui->progressBar, SLOT(setValue(int)));
  connect(warper, SIGNAL(imageUpdated(vil_image_view<double>)),
          this, SLOT(updateImage(vil_image_view<double>)));
  connect(warper, SIGNAL(homographiesUpdated(std::string,
                           std::vector<vgl_h_matrix_2d<double> >)),
          this, SIGNAL(homographiesUpdated(std::string,
                         std::vector<vgl_h_matrix_2d<double> >)));
  connect(warper, SIGNAL(registrationComplete()),
          this, SLOT(finalizeRegistration()));

  // Worker now owns srp and warper
  vpSuperResWorker* worker =
    new vpSuperResWorker(filelist, ref,
                         static_cast<vpSuperResWarper *>(warper),
                         srp, sensorSigma,
                         maxIterations,
                         greyscale,
                         this->Interrupted);

  QThread* thread = new QThread;
  worker->moveToThread(thread);
  connect(thread, SIGNAL(started()), worker, SLOT(process()));
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
void vpSuperResViewer::createImageData(
  unsigned int ni, unsigned int nj, double scaleFactor, bool greyscale,
  vtkSmartPointer<vtkImageData>& imgData,
  vtkSmartPointer<vtkImageActor>& imgActor)
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
void vpSuperResViewer::updateImage(vil_image_view<double> img)
{
  vtkVgAdapt(img, this->ImageData);
  this->ImageActor->GetInput()->Modified();
  this->RenderWindow->Render();
}

//-----------------------------------------------------------------------------
void vpSuperResViewer::closeEvent(QCloseEvent* event)
{
  *this->Interrupted = true;
  event->accept();
}

//-----------------------------------------------------------------------------
void vpSuperResViewer::finalizeImage(vil_image_view<vxl_byte> img)
{
  this->Result = img;
  this->finalized = true;
  this->Ui->saveStopButton->setEnabled(true);
  this->Ui->saveStopButton->setText(QString("Save As..."));
  this->Ui->progressBar->setValue(100);
}

//-----------------------------------------------------------------------------
void vpSuperResViewer::receiveRefImage(vil_image_view<double> img)
{
  vil_image_view<double> bicubic;
  super3d::upsample(img, bicubic, this->ScaleFactor,
                    vidtk::warp_image_parameters::CUBIC);
  vtkVgAdapt(bicubic, this->BicubicImageData);
  this->BicubicImageActor->GetInput()->Modified();
  vil_convert_cast(bicubic, Bicubic);
}

//-----------------------------------------------------------------------------
void vpSuperResViewer::onSaveClicked()
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
  else
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
}

//-----------------------------------------------------------------------------
void vpSuperResViewer::stopHomographyCaching()
{
  disconnect(this,
             SIGNAL(homographiesUpdated(std::string,
                                        std::vector<vgl_h_matrix_2d<double> >)),
             this->parent(),
             SLOT(cacheHomographies(std::string,
                                    std::vector<vgl_h_matrix_2d<double> >)));
}

//-----------------------------------------------------------------------------
void vpSuperResViewer::finalizeRegistration()
{
  // Allow stopping
  this->Ui->saveStopButton->setEnabled(true);
  this->Ui->imageRadio->setEnabled(true);
  this->Ui->bicubicRadio->setEnabled(true);
}

//-----------------------------------------------------------------------------
void vpSuperResViewer::onImageSelected()
{
  if (this->finalized)
    {
    this->Ui->saveStopButton->setText(QString("Save As..."));
    }
  else
    {
    this->Ui->saveStopButton->setText(QString("Stop"));
    }

  // Copy over window / level settings
  this->ImageActor->GetProperty()->SetColorWindow(
    this->BicubicImageActor->GetProperty()->GetColorWindow());
  this->ImageActor->GetProperty()->SetColorLevel(
    this->BicubicImageActor->GetProperty()->GetColorLevel());

  this->Renderer->RemoveViewProp(this->BicubicImageActor);
  this->Renderer->AddViewProp(this->ImageActor);
  this->RenderWindow->Render();
}

//-----------------------------------------------------------------------------
void vpSuperResViewer::onBicubicSelected()
{
  this->Ui->saveStopButton->setText(QString("Save As..."));

  // Copy over window / level settings
  this->BicubicImageActor->GetProperty()->SetColorWindow(
    this->ImageActor->GetProperty()->GetColorWindow());
  this->BicubicImageActor->GetProperty()->SetColorLevel(
    this->ImageActor->GetProperty()->GetColorLevel());

  this->Renderer->RemoveViewProp(this->ImageActor);
  this->Renderer->AddViewProp(this->BicubicImageActor);
  this->RenderWindow->Render();
}
