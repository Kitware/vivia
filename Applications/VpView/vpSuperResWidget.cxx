// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpSuperResWidget.h"

#include "ui_vpSuperResWidget.h"

#include "vpFileDataSource.h"
#include "vpSuperResDepthViewer.h"
#include "vpSuperResViewer.h"
#include "vpTrackIO.h"
#include "vpUtils.h"
#include "vpViewCore.h"

#include <vtkVgAdapt.h>
#include <vtkVgGDALReader.h>
#include <vtkVgGeoCoord.h>
#include <vtkVgInteractorStyleRubberBand2D.h>
#include <vtkVgQtUtil.h>
#include <vtkVgUtil.h>

// Super3d includes
#include <super3d/depth/file_io.h>
#include <super3d/depth/super_config.h>

#include <maptk/core/algo/compute_ref_homography.h>
#include <maptk/core/algo/convert_image.h>
#include <maptk/core/algo/detect_features.h>
#include <maptk/core/algo/extract_descriptors.h>
#include <maptk/core/algo/image_io.h>
#include <maptk/core/algo/match_features.h>
#include <maptk/core/algo/match_features_homography.h>
#include <maptk/core/algo/track_features.h>
#include <maptk/core/config_block_io.h>
#include <maptk/core/track_set_io.h>
#include <maptk/modules.h>
#include <maptk/utilities/nitf_roi_krtd_extraction_utility.h>
#include <maptk/vxl/image_container.h>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkUnsignedCharArray.h>

#include <vnl/vnl_double_3x3.h>

#include <qtScopedValueChange.h>
#include <qtStlUtil.h>
#include <qtUtil.h>

#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegExpValidator>
#include <QListWidgetItem>

#include <boost/make_shared.hpp>

#include <sstream>

//-----------------------------------------------------------------------------
vpSuperResWidget::vpSuperResWidget(QWidget* p)
  : QWidget(p)
{
  this->Ui = new Ui::vpSuperResWidget;
  this->Ui->setupUi(this);

  this->ImageHeight = 0;
  this->ImageWidth = 0;

  this->Ui->width->setMaximum(0);
  this->Ui->height->setMaximum(0);

  this->CropRegionActor = 0;

  this->UsingBundleAdjustHomographies = false;
  this->SelectingSuperResCrop = false;
  this->LastFrameChanged = -1;

  connect(this->Ui->xmin, SIGNAL(editingFinished()),
          this, SLOT(updateXMin()));
  connect(this->Ui->ymin, SIGNAL(editingFinished()),
          this, SLOT(updateYMin()));
  connect(this->Ui->xmin, SIGNAL(valueChanged(int)),
          this, SLOT(updateBox()));
  connect(this->Ui->ymin, SIGNAL(valueChanged(int)),
          this, SLOT(updateBox()));
  connect(this->Ui->width, SIGNAL(valueChanged(int)),
          this, SLOT(updateBox()));
  connect(this->Ui->height, SIGNAL(valueChanged(int)),
          this, SLOT(updateBox()));
  connect(this->Ui->runButton, SIGNAL(clicked()),
          this, SLOT(onRunClicked()));
  connect(this->Ui->cropSelect, SIGNAL(clicked()),
          this, SLOT(cropSelect()));
  connect(this->Ui->bundleCropSelect, SIGNAL(clicked()),
          this, SLOT(bundleCropSelect()));
  connect(this->Ui->runBundleAdjustmentButton, SIGNAL(clicked()),
          this, SLOT(runBundleAdjustment()));
  connect(this->Ui->disable, SIGNAL(clicked()),
          this, SLOT(disable()));
  connect(this->Ui->numImages, SIGNAL(valueChanged(int)),
          this, SLOT(updateNumImages()));
  connect(this->Ui->includeImage, SIGNAL(clicked(bool)),
          this, SLOT(imageIncludeChanged(bool)));
  connect(this->Ui->includeList, SIGNAL(itemSelectionChanged()),
          this, SLOT(onImageListSelection()));
}

//-----------------------------------------------------------------------------
vpSuperResWidget::~vpSuperResWidget()
{
  delete this->Ui;
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::initialize(vpViewCore* viewCore)
{
  this->ViewCoreInstance = viewCore;
  this->CropRegionActor = viewCore->getCropRegionActor();
  this->CropRegionPoints = viewCore->getCropRegionPoints();
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::enableOptions()
{
  if (this->Cameras.empty())
    {
    this->Ui->depthRadio->setEnabled(false);
    this->Ui->depthParamBox->setEnabled(false);
    this->Ui->registerImagesRadio->setChecked(true);
    }

  if (this->BundleAdjustmentConfigFile.empty())
    {
    this->Ui->bundleGroup->setEnabled(false);
    }

  // If homographies were loaded with the project, enable crop select
  vtkNew<vtkMatrix4x4> homography;
  if (this->ViewCoreInstance->getHomography(0, homography.GetPointer()))
    {
    this->Ui->regionBox->setEnabled(true);
    this->Ui->cropSelect->setEnabled(true);
    }
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::loadCameras(const std::string& cameraDirectory)
{
  // Load cameras here for now (eventually re write framemap class to handle
  // cameras)
  for (int i = 0;
       i < this->ViewCoreInstance->getImageDataSource()->getFileCount(); ++i)
    {
    std::string camname =
      this->ViewCoreInstance->getImageDataSource()->getDataFile(i);
    std::string::size_type found = camname.find_last_of("/\\");
    if (found != std::string::npos)
      {
      camname = camname.substr(found + 1, camname.size() - 4 - found - 1);
      }
    camname = cameraDirectory + "/" + camname + ".krtd";
    this->Cameras.push_back(super3d::load_cam(camname));
    }
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::loadDepthConfigFile(const std::string& depthCfgFile)
{
  try
    {
    this->Config = boost::make_shared<super3d::config>();
    this->Config->read_config(depthCfgFile.c_str());
    this->Ui->depthLambda->setValue(
      this->Config->get_value<double>("depth_grad_lambda"));
    }
  catch (const super3d::config::cfg_exception& e)
    {
    qDebug(e.what());
    }
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::disableGreyscaleOption(bool disable)
{
  this->Ui->greyscale->setDisabled(disable);
  // If the option is disabled, it means the images are greyscale already; the
  // state to checked so that the worker knows to process them as greyscale
  if (disable)
    {
    this->Ui->greyscale->setChecked(true);
    }
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::updateXMin()
{
  this->Ui->width->setMaximum(
    std::max(this->ImageWidth - this->Ui->xmin->value() - 1, 0));
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::updateYMin()
{
  this->Ui->height->setMaximum(
    std::max(this->ImageHeight - this->Ui->ymin->value() - 1, 0));
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::updateBox()
{
  if (this->SelectingSuperResCrop)
    {
    return;
    }

  double left = this->Ui->xmin->value();
  double right = left + this->Ui->width->value();
  double top = -this->Ui->ymin->value() + this->ImageHeight;
  double bot = top - this->Ui->height->value();

  left = std::max(left, 0.0);
  top = std::min(top, this->ImageHeight - 1.0);
  right = std::min(right, this->ImageWidth - 1.0);
  bot = std::max(bot, 0.0);

  this->CropRegionPoints->SetPoint(0, left, top, 0.0);
  this->CropRegionPoints->SetPoint(1, right, top, 0.0);
  this->CropRegionPoints->SetPoint(2, right, bot, 0.0);
  this->CropRegionPoints->SetPoint(3, left, bot, 0.0);

  vtkPolyDataMapper::SafeDownCast(
    this->CropRegionActor->GetMapper())->GetInput()->Modified();
  this->ViewCoreInstance->render();
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::bundleCropSelect()
{
  this->Ui->runButton->setEnabled(false);
  vtkVgInteractorStyleRubberBand2D* rbi =
    this->ViewCoreInstance->getInteractorStyle();
  rbi->SetRubberBandModeToSelection();

  // This may not be the case, but assume so for now (can only do bundle
  // adjustment, for now, if originally was using geocoordinates)
  this->ViewCoreInstance->setUseGeoCoordinates(true);
  this->ViewCoreInstance->setHomographyReferenceFrame(-1);
  this->UsingBundleAdjustHomographies = false;

  this->Ui->runBundleAdjustmentButton->setEnabled(false);
  this->Ui->cropSelect->setEnabled(false);
  this->Ui->regionBox->setEnabled(false);
  this->ViewCoreInstance->getBundleRegionActor()->VisibilityOff();
  this->ViewCoreInstance->getCropRegionActor()->VisibilityOff();
  this->ViewCoreInstance->render();

  // Listen for completion of selection
  vtkConnect(
    rbi, vtkVgInteractorStyleRubberBand2D::SelectionCompleteEvent,
    this, SLOT(onBundleSelectionComplete()));
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::cropSelect()
{
  this->SelectingSuperResCrop = true;

  this->Ui->xmin->setEnabled(false);
  this->Ui->width->setEnabled(false);
  this->Ui->ymin->setEnabled(false);
  this->Ui->height->setEnabled(false);
  this->Ui->numImages->setEnabled(false);
  this->Ui->runButton->setEnabled(false);
  this->Ui->includeImage->setEnabled(false);

  this->Ui->disable->setEnabled(true);

  this->Ui->xmin->setValue(0);
  this->Ui->width->setValue(0);
  this->Ui->ymin->setValue(0);
  this->Ui->height->setValue(0);

  vtkVgInteractorStyleRubberBand2D* rbi =
    this->ViewCoreInstance->getInteractorStyle();
  rbi->SetRubberBandModeToSelection();

  //listen for completion of selection
  vtkConnect(
    rbi, vtkVgInteractorStyleRubberBand2D::SelectionCompleteEvent,
    this, SLOT(onSelectionComplete()));

  if (this->UsingBundleAdjustHomographies)
    {
    this->updateHomographyReferenceFrameAndCamera(
      this->ViewCoreInstance->getCurrentFrameIndex());
    }
  else
    {
    this->ViewCoreInstance->setHomographyReferenceFrame(-1);
    }

  this->Included.clear();
  this->Ui->includeList->clear();
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::disable()
{
  // May not be true, but used to block signals in updateBox
  qtScopedValueChange<bool> blockBoxUpdates(this->SelectingSuperResCrop, true);

  this->Ui->xmin->setValue(0);
  this->Ui->width->setValue(0);
  this->Ui->ymin->setValue(0);
  this->Ui->height->setValue(0);

  this->Ui->xmin->setEnabled(false);
  this->Ui->width->setEnabled(false);
  this->Ui->ymin->setEnabled(false);
  this->Ui->height->setEnabled(false);
  this->Ui->numImages->setEnabled(false);
  this->Ui->disable->setEnabled(false);
  this->Ui->runButton->setEnabled(false);

  this->CropRegionActor->VisibilityOff();

  if (this->UsingBundleAdjustHomographies)
    {
    this->updateHomographyReferenceFrameAndCamera(0);
    }
  else
    {
    this->ViewCoreInstance->render();
    this->ViewCoreInstance->setHomographyReferenceFrame(-1);
    }
  this->Included.clear();

  this->Ui->includeList->clear();
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::frameChanged()
{
  if (this->ViewCoreInstance->getCurrentFrameIndex() == this->LastFrameChanged)
    {
    return;
    }

  this->LastFrameChanged = this->ViewCoreInstance->getCurrentFrameIndex();

  // If the run button is enabled, then we've already set the reference frame,
  // don't update image width / height values
  if (!this->Ui->runButton->isEnabled())
    {
    this->ImageWidth =
      static_cast<int>(this->ViewCoreInstance->getImageWidth());
    this->ImageHeight =
      static_cast<int>(this->ViewCoreInstance->getImageHeight());
    this->Ui->width->setMaximum(
      std::max(this->ImageWidth - this->Ui->xmin->value() - 1, 0));
    this->Ui->height->setMaximum(
      std::max(this->ImageHeight - this->Ui->ymin->value() - 1, 0));
    this->Ui->xmin->setMaximum(this->ImageWidth);
    this->Ui->ymin->setMaximum(this->ImageHeight);
    }

  this->Ui->numImages->setMaximum(this->ViewCoreInstance->getNumberOfFrames());

  this->CachedHomographies.clear();
  emit invalidateHomographyCaching();

  if (!this->Included.empty())
    {
    unsigned int currentFrameIndex =
      this->ViewCoreInstance->getCurrentFrameIndex();
    // Reference frame can not be excluded
    if (currentFrameIndex == ViewCoreInstance->getHomographyReferenceFrame())
      {
      this->Ui->includeImage->setEnabled(false);
      this->Ui->includeImage->setChecked(true);
      }
    else
      {
      // when Included vector is not empty, then we can change the inclusion
      // status
      this->Ui->includeImage->setEnabled(true);
      this->Ui->includeImage->setChecked(this->Included[currentFrameIndex]);
      }

    qtScopedBlockSignals bs(this->Ui->includeList);
    if (this->Ui->includeImage->isChecked())
      {
      for (int i = 0; i < this->Ui->includeList->count(); ++i)
        {
        QListWidgetItem* item = this->Ui->includeList->item(i);
        if (item->data(Qt::UserRole).toInt() == currentFrameIndex)
          {
          this->Ui->includeList->setCurrentItem(item);
          break;
          }
        }
      }
    else
      {
      this->Ui->includeList->clearSelection();
      }
    }
  else
    {
    this->Ui->includeImage->setEnabled(false);
    this->Ui->includeImage->setChecked(false);
    }

  if (this->UsingBundleAdjustHomographies && this->SelectingSuperResCrop)
    {
    this->updateHomographyReferenceFrameAndCamera(
      this->ViewCoreInstance->getCurrentFrameIndex());
    }
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::updateHomographyReferenceFrameAndCamera(
  int newReferenceFrame)
{
  double newCameraPosition[2], currentPos[3];
  vtkNew<vtkMatrix4x4> homography;
  vtkNew<vtkMatrix4x4> yFlip;
  yFlip->SetElement(1, 1, -1.0);  // y dim set later

  vtkCamera* camera =
    this->ViewCoreInstance->getSceneRenderer()->GetActiveCamera();
  camera->GetPosition(currentPos);

  vtkPoints* bundleRegionPts = this->ViewCoreInstance->getBundleRegionPoints();

  // NOTE: "homography reference" means the reference for the homography, to be
  // differentiated from HomographyReferenceFrame, which is the frame used
  // for display reference.

  // Transform current reference frame to homography reference
  int refFrame = this->ViewCoreInstance->getHomographyReferenceFrame();
  this->ViewCoreInstance->getHomography(refFrame, homography.GetPointer());
  yFlip->SetElement(
    1, 3, this->ViewCoreInstance->getHomographyReferenceImageHeight());
  vtkVgApplyHomography(currentPos, yFlip.GetPointer(), currentPos);
  vtkVgApplyHomography(currentPos, homography.GetPointer(), currentPos);
  double pt[3];
  for (int i = 0; i < bundleRegionPts->GetNumberOfPoints(); ++i)
    {
    bundleRegionPts->GetPoint(i, pt);
    vtkVgApplyHomography(pt, yFlip.GetPointer(), pt);
    vtkVgApplyHomography(pt, homography.GetPointer(), pt);
    bundleRegionPts->SetPoint(i, pt);
    }

  // Update the homography reference frame
  int imageHeight = this->ImageHeight;
  if (newReferenceFrame == 0)
    {
    imageHeight = this->ViewCoreInstance->getFirstImageY();
    }
  this->ViewCoreInstance->setHomographyReferenceFrame(newReferenceFrame,
                                                      imageHeight);

  // Transform from homography reference to new reference frame
  this->ViewCoreInstance->getHomography(newReferenceFrame,
                                        homography.GetPointer());
  homography->Invert();
  vtkVgApplyHomography(currentPos, homography.GetPointer(), newCameraPosition);
  yFlip->SetElement(1, 3, imageHeight);
  vtkVgApplyHomography(newCameraPosition, yFlip.GetPointer(),
                       newCameraPosition);
  for (int i = 0; i < bundleRegionPts->GetNumberOfPoints(); ++i)
    {
    bundleRegionPts->GetPoint(i, pt);
    vtkVgApplyHomography(pt, homography.GetPointer(), pt);
    vtkVgApplyHomography(pt, yFlip.GetPointer(), pt);
    bundleRegionPts->SetPoint(i, pt);
    }
  bundleRegionPts->Modified();

  this->ViewCoreInstance->moveCameraTo(newCameraPosition, false);
  this->ViewCoreInstance->update(true);
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::imageIncludeChanged(bool isChecked)
{
  this->Ui->numImages->setEnabled(false);
  qtScopedBlockSignals bs(this->Ui->numImages);
  this->Ui->numImages->setValue(
    this->Ui->numImages->value() + (isChecked ? 1 : -1));

  int index = ViewCoreInstance->getCurrentFrameIndex();
  this->Included[index] = isChecked;

  QListWidgetItem* currentItem = this->buildImageIncludeList(index);

  if (isChecked && currentItem)
    {
    this->Ui->includeList->setCurrentItem(currentItem);
    }

  this->CachedHomographies.clear();
  emit invalidateHomographyCaching();
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::onBundleSelectionComplete()
{
  this->completeSelection(true);

  // Stop listening for selections
  vtkDisconnect(
    this->ViewCoreInstance->getInteractorStyle(),
    vtkVgInteractorStyleRubberBand2D::SelectionCompleteEvent,
    this, SLOT(onBundleSelectionComplete()));
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::onSelectionComplete()
{
  this->SelectingSuperResCrop = false;
  this->completeSelection(false);

  // Stop listening for selections
  vtkDisconnect(
    this->ViewCoreInstance->getInteractorStyle(),
    vtkVgInteractorStyleRubberBand2D::SelectionCompleteEvent,
    this, SLOT(onSelectionComplete()));
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::completeSelection(bool isBundleCrop)
{
  vtkVgInteractorStyleRubberBand2D* rbi =
    this->ViewCoreInstance->getInteractorStyle();

  int* start = rbi->GetStartPosition();
  int* end = rbi->GetEndPosition();

  vtkRenderer* renderer = this->ViewCoreInstance->getSceneRenderer();

  double pt_start[4], pt_end[4];
  pt_start[2] = pt_end[2] = 0.0;
  pt_start[3] = pt_end[3] = 1.0;

  // We want pt_start to be the upper-left corner (and pt_end the lower-right)
  pt_start[0] = start[0] > end[0] ? end[0] : start[0];
  pt_end[0]   = start[0] > end[0] ? start[0] : end[0];
  pt_start[1] = start[1] > end[1] ? start[1] : end[1];
  pt_end[1]   = start[1] > end[1] ? end[1] : start[1];

  renderer->SetDisplayPoint(pt_start);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(pt_start);

  renderer->SetDisplayPoint(pt_end);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(pt_end);

  vtkActor* actor;
  vtkPoints* points;
  if (isBundleCrop)
    {
    actor = this->ViewCoreInstance->getBundleRegionActor();
    points = this->ViewCoreInstance->getBundleRegionPoints();
    }
  else
    {
    actor = this->CropRegionActor;
    points = this->CropRegionPoints;
    }

  if (pt_end[0] >= 0 && pt_end[1] <= (this->ImageHeight - 1.0) &&
      pt_start[0] < (this->ImageWidth - 1.0) && pt_start[1] >= 0)
    {
    actor->VisibilityOn();

    pt_start[0] = std::max(pt_start[0], 0.0);
    pt_start[1] = std::min(pt_start[1], this->ImageHeight - 1.0);
    pt_end[0] = std::min(pt_end[0], this->ImageWidth - 1.0);
    pt_end[1] = std::max(pt_end[1], 0.0);

    points->SetPoint(0, pt_start[0] + 0.5, pt_start[1] + 0.5, 0.0);
    points->SetPoint(1, pt_end[0] + 0.5, pt_start[1] + 0.5, 0.0);
    points->SetPoint(2, pt_end[0] + 0.5, pt_end[1] + 0.5, 0.0);
    points->SetPoint(3, pt_start[0] + 0.5, pt_end[1] + 0.5, 0.0);

    actor->GetMapper()->GetInput()->Modified();

    if (isBundleCrop)
      {
      this->Ui->runBundleAdjustmentButton->setEnabled(true);
      this->WorldUpperLeft[0] = pt_start[0];
      this->WorldUpperLeft[1] = pt_start[1];
      this->WorldLowerRight[0] = pt_end[0];
      this->WorldLowerRight[1] = pt_end[1];
      }
    else
      {
      this->Ui->width->setMaximum(
        qMax(this->ImageWidth - this->Ui->xmin->value() - 1, 0));
      this->Ui->height->setMaximum(
        qMax(this->ImageHeight - this->Ui->ymin->value() - 1, 0));

      this->Ui->xmin->setValue(pt_start[0]);
      this->Ui->ymin->setValue(this->ImageHeight - pt_start[1]);
      this->Ui->width->setValue(abs(pt_start[0] - pt_end[0]));
      this->Ui->height->setValue(abs(pt_start[1] - pt_end[1]));

      this->Ui->xmin->setEnabled(true);
      this->Ui->width->setEnabled(true);
      this->Ui->ymin->setEnabled(true);
      this->Ui->height->setEnabled(true);
      this->Ui->numImages->setEnabled(true);

      this->Ui->runButton->setEnabled(true);
      this->ViewCoreInstance->setHomographyReferenceFrame(
        this->ViewCoreInstance->getCurrentFrameIndex(), this->ImageHeight);

      this->updateImageInclusion();

      // Reference frame is included but not able to be excluded
      this->Ui->includeImage->setEnabled(false);
      }
    }
  else
    {
    actor->VisibilityOff();
    this->Ui->runButton->setEnabled(false);
    }

  rbi->SetRubberBandModeToZoom();
  this->ViewCoreInstance->render();
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::runBundleAdjustment()
{
  // Construct ROI 3D bounding box
  vgl_box_3d<double> roi;
  vtkVgGeoCoord ulGeoCoord, lrGeoCoord;
  qDebug() << "Bundle adjustment: starting corner"
           << this->WorldUpperLeft[0] << this->WorldUpperLeft[1]
           << this->Ui->elevationMin->value();
  ulGeoCoord = this->ViewCoreInstance->worldToGeo(this->WorldUpperLeft);
  lrGeoCoord = this->ViewCoreInstance->worldToGeo(this->WorldLowerRight);
  vgl_point_3d<double> ulPoint(ulGeoCoord.Easting, ulGeoCoord.Northing,
                               this->Ui->elevationMin->value());
  vgl_point_3d<double> lrPoint(lrGeoCoord.Easting, lrGeoCoord.Northing,
                               this->Ui->elevationMax->value());
  roi.add(ulPoint);
  roi.add(lrPoint);

  qDebug() << "Bundle adjustment: ROI"
           << roi.min_x() << roi.min_y() << roi.min_z() << "to"
           << roi.max_x() << roi.max_y() << roi.max_z();

  vpFileDataSource* imgDataSrc = this->ViewCoreInstance->getImageDataSource();
  std::vector<maptk::path_t> sourceNitfFiles;
  for (int i = 0; i < this->ViewCoreInstance->getNumberOfFrames(); ++i)
    {
    sourceNitfFiles.push_back(maptk::path_t(imgDataSrc->getDataFile(i)));
    }

  // Only do maptk register of modules once...
  static bool first = true;
  if (first)
    {
    maptk::register_modules();
    first = false;
    }

  // Get the desired roi from each image and extract krtd cameras
  std::vector<vgl_box_2d<unsigned int> > pixelRois;
  std::vector<maptk::camera_sptr> krtdCameras;
  maptk::nitf_roi_krtd_extraction_utility(roi, sourceNitfFiles,
                                          pixelRois, krtdCameras);

  std::vector<vgl_box_2d<unsigned int> >::const_iterator roiIter;
  std::vector<maptk::path_t>::const_iterator fileIter;
  vtkNew<vtkVgGDALReader> gdalReader;
  vtkNew<vtkImageShiftScale> scale;
  scale->SetScale(1.0 / 8.0);
  scale->SetOutputScalarTypeToUnsignedChar();
  vtkNew<vtkImageData> image;
  scale->SetInputData(image.GetPointer());

  std::vector<maptk::image_container_sptr> pixelObjects;
  std::vector<int> yDimension;
  for (roiIter = pixelRois.begin(), fileIter = sourceNitfFiles.begin();
       roiIter != pixelRois.end(); ++roiIter, ++fileIter)
    {
    gdalReader->SetFileName(fileIter->string().c_str());

    // Need to do y-flip, thus need raster dimensions
    gdalReader->UpdateInformation();
    int rasterDimensions[2];
    gdalReader->GetRasterDimensions(rasterDimensions);
    yDimension.push_back(rasterDimensions[1]);
    gdalReader->SetReadExtents(roiIter->min_x(), roiIter->max_x(),
                               rasterDimensions[1] - roiIter->max_y() - 1,
                               rasterDimensions[1] - roiIter->min_y() - 1);
    gdalReader->SetOutputResolution(roiIter->max_x() - roiIter->min_x() + 1,
                                    roiIter->max_y() - roiIter->min_y() + 1);

    gdalReader->Update();
    // Shift origin to align with the extents (mainly needed, I think, when
    // writing out images.
    image->ShallowCopy(gdalReader->GetOutput());
    image->SetOrigin(roiIter->min_x(),
                     rasterDimensions[1] - roiIter->max_y() - 1, 0.0);
    scale->Update();

    // Convert vtk to vil and store for later use
    vil_image_view<vxl_byte> vilImage;
    vtkVgAdapt(scale->GetOutput(), vilImage);
    maptk::image_container_sptr
      maptkImage(new maptk::vxl::image_container(vilImage));
    pixelObjects.push_back(maptkImage);
    }

  // Read the configuration file, which needs to specify all required values
  maptk::config_block_sptr config;
  try
    {
    config = maptk::read_config_file(this->BundleAdjustmentConfigFile);
    }
  catch (...)
    {
    qWarning() << "Error reading bundle adjustment config file";
    return;
    }

  maptk::algo::track_features_sptr feature_tracker;
  maptk::algo::compute_ref_homography_sptr computeRefHomography;

  maptk::algo::track_features::set_nested_algo_configuration(
    "feature_tracker", config, feature_tracker);
  maptk::algo::track_features::get_nested_algo_configuration(
    "feature_tracker", config, feature_tracker);

  if (!maptk::algo::track_features::check_nested_algo_configuration(
         "feature_tracker", config))
    {
    qWarning() << "Failed to properly configure the feature tracker";
    return;
    }

  maptk::algo::compute_ref_homography::set_nested_algo_configuration(
    "compute_ref_homography", config, computeRefHomography);
  maptk::algo::compute_ref_homography::get_nested_algo_configuration(
    "compute_ref_homography", config, computeRefHomography);

  if (!maptk::algo::compute_ref_homography::check_nested_algo_configuration(
         "compute_ref_homography", config))
    {
    qWarning() << "Failed to properly configure compute_ref_homography";
    return;
    }

  // Track features on each frame sequentially
  maptk::track_set_sptr tracks;
  vtkSmartPointer<vtkMatrix4x4> vtkMatrix;
  vtkSmartPointer<vtkMatrix4x4> translate =
    vtkSmartPointer<vtkMatrix4x4>::New();
  for(unsigned i = 0; i < sourceNitfFiles.size(); ++i)
    {
    qDebug() << "processing frame" << i;
    tracks = feature_tracker->track(tracks, i, pixelObjects.at(i));
    maptk::f2f_homography_sptr h = computeRefHomography->estimate(i, tracks);
    vnl_matrix_fixed<double, 3, 3> vnlMatrix(h->data());
    vtkSmartPointer<vtkMatrix4x4> vtkMatrix = vtkVgAdapt(vnlMatrix);
    vnlMatrix.print(std::cerr);
    translate->SetElement(0, 3, - static_cast<int>(pixelRois[i].min_x()));
    translate->SetElement(1, 3, - static_cast<int>(pixelRois[i].min_y()));
    vtkMatrix4x4::Multiply4x4(vtkMatrix, translate, vtkMatrix);
    translate->SetElement(0, 3, static_cast<int>(pixelRois[0].min_x()));
    translate->SetElement(1, 3, static_cast<int>(pixelRois[0].min_y()));
    vtkMatrix4x4::Multiply4x4(translate, vtkMatrix, vtkMatrix);
    vtkMatrix->Print(std::cerr);
    this->ViewCoreInstance->setHomography(i, vtkMatrix);
    }
  this->ViewCoreInstance->setUseGeoCoordinates(false);
  this->ViewCoreInstance->setHomographyReferenceFrame(0, yDimension[0]);
  this->UsingBundleAdjustHomographies = true;

  // Update image info based on the first frame
  gdalReader->SetFileName(sourceNitfFiles[0].string().c_str());
  gdalReader->UpdateInformation();
  int rasterDimensions[2];
  gdalReader->GetRasterDimensions(rasterDimensions);

  this->ImageWidth = rasterDimensions[0];
  this->ImageHeight = rasterDimensions[1];

  this->Ui->width->setMaximum(
    std::max(this->ImageWidth - this->Ui->xmin->value() - 1, 0));
  this->Ui->height->setMaximum(
    std::max(this->ImageHeight - this->Ui->ymin->value() - 1, 0));
  this->Ui->xmin->setMaximum(this->ImageWidth);
  this->Ui->ymin->setMaximum(this->ImageHeight);

  // Don't allow rerunning bundle adjustment unless a new bundle adjust
  // region is selected.
  this->Ui->runBundleAdjustmentButton->setEnabled(false);
  this->Ui->regionBox->setEnabled(true);
  this->Ui->cropSelect->setEnabled(true);
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::onRunClicked()
{
  unsigned int index = this->ViewCoreInstance->getHomographyReferenceFrame();
  const int numFrames = ViewCoreInstance->getNumberOfFrames();
  vpFileDataSource* imgDataSrc = ViewCoreInstance->getImageDataSource();

  std::vector<std::string> filelist;
  int count = 0, ref = -1;
  for (int i = 0; i < numFrames; ++i)
    {
    if (i == index)
      {
      ref = count;
      }
    if (this->Included[i])
      {
      filelist.push_back(imgDataSrc->getDataFile(i));
      ++count;
      }
    }
  Q_ASSERT(ref >= 0);

  int xmin = this->Ui->xmin->value();
  int ymin = this->Ui->ymin->value();
  int width =  this->Ui->width->value();
  int height = this->Ui->height->value();

  std::ostringstream oss;
  oss << ref << " " << xmin << " " << width << " " << ymin << " " << height;

  super3d::super_res_params::TV_METHOD method;
  if (this->Ui->originalAlgorithmRadio->isChecked())
    {
    method = super3d::super_res_params::SUPER3D_BASELINE;
    }
  else if (this->Ui->gradientAlgorithmRadio->isChecked())
    {
    method = super3d::super_res_params::GRADIENTDATA_IMAGEPRIOR;
    }
  else if (this->Ui->illumAlgorithmRadio->isChecked())
    {
    method = super3d::super_res_params::IMAGEDATA_IMAGEPRIOR_ILLUMINATIONPRIOR;
    }
  else if (this->Ui->textureAlgorithmRadio->isChecked())
    {
    method = super3d::super_res_params::MEDIANDATA_IMAGEPRIOR;
    }

  super3d::super_res_params* srp = new super3d::super_res_params;
  double regularizationScale = this->Ui->regularization->value();
  srp->ref_frame = ref;
  this->readSuperResParams(method, srp, regularizationScale);

  bool registerImages = this->Ui->registerImagesRadio->isChecked();

  if (registerImages)
    {
    vpSuperResViewer* superResViewer = new vpSuperResViewer(this);
    superResViewer->show();
    superResViewer->setAttribute(Qt::WA_DeleteOnClose);

    std::vector<vgl_h_matrix_2d<double> > homographies;

    superResViewer->setWindowTitle(
      QString("Super Res 2D Viewer | Resolving frame %1 using %2 frames").
        arg(index + 1).arg(count));

    vtkNew<vtkMatrix4x4> homography;
    for (int i = 0; i < numFrames; ++i)
      {
      if (!this->Included[i])
        {
        continue;
        }

      this->ViewCoreInstance->getHomography(i, homography.GetPointer());
      vnl_double_3x3 hMatrix;
      if (homography.GetPointer())
        {
        hMatrix(0, 0) = homography->GetElement(0, 0);
        hMatrix(0, 1) = homography->GetElement(0, 1);
        hMatrix(0, 2) = homography->GetElement(0, 3);
        hMatrix(1, 0) = homography->GetElement(1, 0);
        hMatrix(1, 1) = homography->GetElement(1, 1);
        hMatrix(1, 2) = homography->GetElement(1, 3);
        hMatrix(2, 0) = homography->GetElement(3, 0);
        hMatrix(2, 1) = homography->GetElement(3, 1);
        hMatrix(2, 2) = homography->GetElement(3, 3);
        }
      else
        {
        hMatrix.set_identity();
        }

      homographies.push_back(vgl_h_matrix_2d<double>(hMatrix));
      }

    superResViewer->start(filelist,
                          homographies,
                          srp,
                          registerImages,
                          this->Ui->greyscale->isChecked(),
                          this->Ui->sigma->value(),
                          oss.str(),
                          xmin,
                          ymin,
                          width,
                          height,
                          ref);
    }
  else
    {
    vpSuperResDepthViewer* superResDepthViewer =
      new vpSuperResDepthViewer(this);
    superResDepthViewer->show();
    superResDepthViewer->setAttribute(Qt::WA_DeleteOnClose);

    superResDepthViewer->setWindowTitle(
      QString("Super Res 3D Viewer | Resolving frame %1 using %2 frames").
        arg(index + 1).arg(count));

    std::vector<vpgl_perspective_camera<double> > cameras;
    for (int i = 0; i < numFrames; ++i)
      {
      if (this->Included[i])
        {
        cameras.push_back(this->Cameras[i]);
        }
      }

    superResDepthViewer->start(
      filelist, cameras, this->Config,
      stdString(this->ViewCoreInstance->depthConfigFile()), srp,
      this->Ui->greyscale->isChecked(), this->Ui->sigma->value(),
      this->Ui->depthLambda->value(), oss.str(),
      xmin, ymin, width, height, ref);
    }
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::updateNumImages()
{
  this->CachedHomographies.clear();
  emit invalidateHomographyCaching();
  updateImageInclusion();
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::updateImageInclusion()
{
  int index = this->ViewCoreInstance->getHomographyReferenceFrame();
  if (index < 0)
    {
    return;
    }

  int range = this->Ui->numImages->value() / 2;
  int start_index = index - range;
  int end_index = index + range + this->Ui->numImages->value() % 2;
  if (start_index < 0)
    {
    end_index -= start_index;
    start_index = 0;
    }
  else if (end_index > this->ViewCoreInstance->getNumberOfFrames())
    {
    start_index -= end_index - this->ViewCoreInstance->getNumberOfFrames();
    end_index = this->Ui->numImages->maximum();
    }

  this->Included.resize(this->ViewCoreInstance->getNumberOfFrames());
  for (int i = 0; i < this->ViewCoreInstance->getNumberOfFrames(); ++i)
    {
    this->Included[i] = (i >= start_index && i < end_index);
    }

  QListWidgetItem* refItem = this->buildImageIncludeList(index);
  this->Ui->includeList->setCurrentItem(refItem);

  this->Ui->includeImage->setChecked(
    this->Included[this->ViewCoreInstance->getCurrentFrameIndex()]);
}

//-----------------------------------------------------------------------------
QListWidgetItem* vpSuperResWidget::buildImageIncludeList(int currentFrame)
{
  int referenceFrame = this->ViewCoreInstance->getHomographyReferenceFrame();

  this->Ui->includeList->clear();
  QListWidgetItem* currentItem = 0;
  for (int i = 0; i < this->Included.size(); ++i)
    {
    if (this->Included[i])
      {
      QListWidgetItem* item = new QListWidgetItem;
      item->setData(Qt::UserRole, QVariant(i));

      std::string filename =
        this->ViewCoreInstance->getImageDataSource()->getDataFile(i);
      std::string::size_type found = filename.find_last_of("/\\");
      if (found != std::string::npos)
        {
        filename = filename.substr(found + 1);
        }
      if (i == referenceFrame)
        {
        filename += "*";
        }
      item->setText(qtString(filename));
      this->Ui->includeList->addItem(item);

      if (i == currentFrame)
        {
        currentItem = item;
        }
      }
    }

  return currentItem;
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::onImageListSelection()
{
  QListWidgetItem* item = this->Ui->includeList->currentItem();
  if (item)
    {
    int frame = item->data(Qt::UserRole).toInt();
    this->ViewCoreInstance->setCurrentFrame(frame, -1);
    }
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::cacheHomographies(std::string cropregion,
  std::vector<vgl_h_matrix_2d<double> > homographies)
{
  this->CachedHomographies[cropregion] = homographies;
}

//-----------------------------------------------------------------------------
void vpSuperResWidget::readSuperResParams(
  super3d::super_res_params::TV_METHOD method,
  super3d::super_res_params* srp,
  double regularizationScale)
{
#define GET_CONFIG(key) this->Config->get_value<double>(#key)

  srp->cost_function = super3d::super_res_params::HUBER_NORM;
  srp->tv_method = method;
  srp->erosion_radius = GET_CONFIG(erosion_radius);
  srp->median_radius = GET_CONFIG(median_radius);
  srp->scale_factor = 2.0;
  srp->debug = false;
  srp->frame_step = 15;

  srp->lambda = GET_CONFIG(baseline_lambda) * regularizationScale;
  srp->sigma = GET_CONFIG(baseline_sigma);
  srp->tau = GET_CONFIG(baseline_tau);
  srp->epsilon_data = GET_CONFIG(baseline_epsilon_data);
  srp->epsilon_reg = GET_CONFIG(baseline_epsilon_reg);

  srp->alpha_a = GET_CONFIG(alpha_a);
  srp->sigma_qa = GET_CONFIG(sigma_qa);

  srp->alpha_r = GET_CONFIG(alpha_r);
  srp->lambda_r = GET_CONFIG(lambda_r) * regularizationScale;
  srp->sigma_pr = GET_CONFIG(sigma_pr);

  srp->alpha_g = GET_CONFIG(alpha_g);
  srp->sigma_qg = GET_CONFIG(sigma_qg);
  srp->lambda_g = GET_CONFIG(lambda_g) * regularizationScale;

  srp->alpha_l = GET_CONFIG(alpha_l);
  srp->lambda_l = GET_CONFIG(lambda_l) * regularizationScale;
  srp->sigma_pl = GET_CONFIG(sigma_pl);

  srp->gamma_a = 0.0;
  srp->gamma_g = 0.0;
  srp->gamma_l = 0.0;
  srp->gamma_r = 0.0;
}
