/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpViewCore.h"

#include <vgActivityType.h>
#include <vgColor.h>
#include <vgEventType.h>
#include <vgTrackType.h>
#include <vgFileDialog.h>
#include <vgMixerWidget.h>

#include "vpActivityConfig.h"
#include "vpAnnotation.h"
#include "vpBox.h"
#include "vpContour.h"
#include "vpEntityConfigWidget.h"
#include "vpEventConfig.h"
#include "vpEventTypeRegistry.h"
#include "vpFileDataSource.h"
#include "vpFrameMap.h"
#include "vpImageSourceFactory.h"
#include "vpInformaticsDialog.h"
#include "vpModelIO.h"
#include "vpNormalcyMaps.h"
#include "vpObjectInfoPanel.h"
#include "vpProject.h"
#include "vpProjectParser.h"
#include "vpQtViewer3dWidget.h"
#include "vpRenderWindowDialog.h"
#include "vpSessionView.h"
#include "vpTimelineDialog.h"
#include "vpTrackConfig.h"
#include "vpVideoAnimation.h"
#include "vtkVpTrackModel.h"

#ifdef VISGUI_USE_VIDTK
#include "vpVidtkFileIO.h"
#endif

#ifdef VISGUI_USE_KWIVER
#include "vpKwiverImproveTrackWorker.h"
#include "vpKwiverVideoSource.h"

#include <vital/plugin_loader/plugin_manager.h>
#include <vital/types/object_track_set.h>
#endif

// VTK includes.
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkBoundingBox.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCommand.h>
#include <vtkContourWidget.h>
#include <vtkCoordinate.h>
#include <vtkCursor2D.h>
#include <vtkDataSetAttributes.h>
#include <vtkDoubleArray.h>
#include <vtkElevationFilter.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkExtractVOI.h>
#include <vtkGraph.h>
#include <vtkGraphLayoutView.h>
#include <vtkGreedyTerrainDecimation.h>
#include <vtkHoverWidget.h>
#include <vtkImageActor.h>
#include <vtkImageBlend.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImageProperty.h>
#include <vtkIntArray.h>
#include <vtkJPEGReader.h>
#include <vtkJPEGWriter.h>
#include <vtkLegendBoxActor.h>
#include <vtkLinearTransform.h>
#include <vtkLineSource.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkMergeFilter.h>
#include <vtkNew.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkPolygon.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRandomGraphSource.h>
#include <vtkRegularPolygonSource.h>
#include <vtkRenderWindow.h>
#include <vtkRenderedGraphRepresentation.h>
#include <vtkRenderer.h>
#include <vtkSetGet.h>
#include <vtkStringArray.h>
#include <vtkTexture.h>
#include <vtkTextureMapToPlane.h>
#include <vtkTexturedActor2D.h>
#include <vtkTimerLog.h>
#include <vtkWarpScalar.h>
#include <vtkWindowToImageFilter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>

// VisGUI includes.
#include <vtkVgActivity.h>
#include <vtkVgActivityManager.h>
#include <vtkVgActivityTypeRegistry.h>
#include <vtkVgContourOperatorManager.h>
#include <vtkVgEvent.h>
#include <vtkVgEventFilter.h>
#include <vtkVgEventIconRepresentation.h>
#include <vtkVgEventModel.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgEventRepresentation.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVgGraphModel.h>
#include <vtkVgGraphRepresentation.h>
#include <vtkVgIconManager.h>
#include <vtkVgImageSource.h>
#include <vtkVgInteractorStyleRubberBand2D.h>
#include <vtkVgJPEGReader.h>
#include <vtkVgMultiResJpgImageReader2.h>
#include <vtkVgPNGReader.h>
#include <vtkVgPickData.h>
#include <vtkVgPicker.h>
#include <vtkVgRendererUtils.h>
#include <vtkVgTemporalFilters.h>
#include <vtkVgTrack.h>
#include <vtkVgTrackFilter.h>
#include <vtkVgTrackHeadRepresentation.h>
#include <vtkVgTrackRepresentation.h>
#include <vtkVgTrackTypeRegistry.h>

#include <vtkVpInteractionCallback.h>

#if defined(VISGUI_USE_GDAL)
  #include <vtkVgGDALReader.h>
#endif

#include <QVTKWidget.h>

// Qt includes.
#include <QApplication>
#include <QComboBox>
#include <QCursor>
#include <QDebug>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QInputDialog>
#include <QLayout>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QRegExp>
#include <QSettings>
#include <QScopedPointer>
#include <QTime>
#include <QTimer>
#include <QToolBar>

// C++ includes.
#include <cmath>
#include <cstdlib>
#include <limits>

// QtExtensions includes.
#include <qtStlUtil.h>

#include <vtksys/SystemTools.hxx>

// VIDTK includes
#ifdef VISGUI_USE_VIDTK
#include <tracking_data/track_state.h> // for track state attributes
#include <utilities/token_expansion.h>
#endif

#include <GeographicLib/Geodesic.hpp>

static double TrackEditColor[] = { 1.0, .274, .274 };
static double SelectedColor[] =  { 1.0, 0.08, 0.58 };

enum { DefaultTrackExpiration = 10 };

enum RegionEditMode { REM_Auto, REM_BoundingBox, REM_Polygon };

// Workaround name collision with macro from WinUser.h
#undef GetProp

// Constructor
//-----------------------------------------------------------------------------
vpViewCore::vpViewCore() :
  Playing(false),
  UsingTimeStampData(false),
  UsingTimeBasedIndexing(false),
  UseTimeStampDataIfAvailable(true),
  UseTimeBasedIndexing(true),
  EnableWorldDisplayIfAvailable(true),
  EnableTranslateImage(true),
  CurrentProjectId(0),
  CurrentTrackColorMode(vtkVgTrackRepresentationBase::TCM_Model),
  Contour(0),
  DrawingContour(false),
  NewTrackId(-1),
  TrackHeadBox(0),
  TrackHeadContour(0),
  TrackHeadRegion(0),
  HideTrackHeadIndicator(false),
  RegionEditMode(REM_Auto),
  ExternalExecuteMode(-1),
  TrackUpdateChunkSize(10),
  VideoAnimation(new vpVideoAnimation(this)),
  GraphRenderingEnabled(true),
  RulerEnabled(false)
{
#ifdef VISGUI_USE_KWIVER
  kwiver::vital::plugin_manager::instance().load_all_plugins();
#endif

  this->initializeDisplay();
  this->initializeData();
  this->initializeScene();
  this->initializeExtentsBounds();
  this->initializeSources();
  this->initializeAllOthers();

  this->ExternalProcess = new QProcess(this);
  this->FileSystemWatcher = new QFileSystemWatcher();
  // Force watcher to use polling since the normal engine doesn't work
  // with network shares.
  this->FileSystemWatcher->setObjectName(
    QLatin1String("_qt_autotest_force_engine_poller"));

  connect(this->VideoAnimation, SIGNAL(finished()), this,
          SIGNAL(reachedPlayBoundary()));

  connect(this->VideoAnimation, SIGNAL(rateChanged(qreal)), this,
          SIGNAL(playbackRateChanged(qreal)));
}

//-----------------------------------------------------------------------------
vpViewCore::~vpViewCore()
{
  if (this->ExternalProcess->state() == QProcess::Running)
    {
    this->ExternalProcess->kill();
    this->ExternalProcess->waitForFinished(5000);
    delete ExternalProcess;
    }
  delete this->FileSystemWatcher;

  this->cleanUp();
}

//-----------------------------------------------------------------------------
void vpViewCore::cleanUp()
{
  delete this->InformaticsDialog;   this->InformaticsDialog = 0;
  delete this->FrameMap;            this->FrameMap = 0;
  delete this->ImageDataSource;     this->ImageDataSource = 0;

  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    delete this->Projects[i];
    }

  delete this->ProjectParser;       this->ProjectParser = 0;
  delete this->TrackConfig;         this->TrackConfig = 0;
  delete this->EventConfig;         this->EventConfig = 0;
  delete this->ActivityConfig;      this->ActivityConfig = 0;
  delete this->NormalcyMaps;        this->NormalcyMaps = 0;
  delete this->Annotation;          this->Annotation = 0;
  delete this->Contour;             this->Contour = 0;
  delete this->TrackHeadBox;        this->TrackHeadBox = 0;
  delete this->TrackHeadContour;    this->TrackHeadContour = 0;

  for (size_t i = 0, end = this->Contours.size(); i < end; ++i)
    {
    delete this->Contours[i];
    }

  this->ActivityTypeRegistry->Delete();
  this->EventTypeRegistry->Delete();
  this->TrackTypeRegistry->Delete();

  vpImageSourceFactory::OnApplicationExit();
}

//-----------------------------------------------------------------------------
void vpViewCore::openProject()
{
  QString path = vgFileDialog::getOpenFileName(
    0, "File Open Dialog:", QString(), "vpView project (*.prj);;");

  if (path.isEmpty())
    {
    return;
    }

  if (!this->loadProject(qPrintable(path)))
    {
    return;
    }

  // Update view.
  if (this->Projects.size() == 1)
    {
    this->refreshView();
    }
}

namespace
{

class SetImageHeight
{
  unsigned int PrevHeight;
  vpModelIO* IO;

public:
  SetImageHeight(vpModelIO* io, unsigned int imageHeight) : IO(io)
    {
    this->PrevHeight = io->GetImageHeight();
    io->SetImageHeight(imageHeight);
    }

  ~SetImageHeight()
    {
    this->IO->SetImageHeight(this->PrevHeight);
    }
};

}

//-----------------------------------------------------------------------------
void vpViewCore::importProject()
{
#ifndef VISGUI_USE_VIDTK
  QMessageBox::warning(0, QString(),
                       "Cannot import project files without VidTK support.");
  return;
#else
  int session = this->SessionView->GetCurrentSession();
  vpProject* currentProject = this->Projects[session];

  vpVidtkFileIO* currentIO =
    dynamic_cast<vpVidtkFileIO*>(currentProject->ModelIO.data());
  if (!currentIO)
    {
    emit this->warningError(
      "Importing into non-file-based projects is not currently supported.");
    return;
    }

  QString filter = "*.prj;;";
  vgFileDialog fileDialog(0, "Import Project", QString(), filter);
  fileDialog.setObjectName("importProjectDialog");
  fileDialog.setFileMode(QFileDialog::ExistingFile);
  if (fileDialog.exec() != QDialog::Accepted)
    {
    return;
    }

  this->ProjectParser->SetFileName(
    qPrintable(fileDialog.selectedFiles().front()));
  this->ProjectParser->SetUseStream(false);

  QScopedPointer<vpProject> project(new vpProject(this->CurrentProjectId + 1));
  this->ProjectParser->Parse(project.data());

  if (project->AnalysisDimensions[0] == -1.0 ||
      project->AnalysisDimensions[1] == -1.0)
    {
    emit this->warningError("The source project does not have valid AOI "
                            "dimensions specified in the project file. "
                            "Please add a valid AnalysisDimensions tag to the "
                            "project file to be imported and try again.");
    return;
    }

  if (this->UseRawImageCoordinates)
    {
    if (project->AOIUpperLeftLatLon[0] == 444.0 ||
        project->AOIUpperLeftLatLon[1] == 444.0)
      {
      emit this->warningError("The source project does not have valid AOI "
                              "geocoordinates. It cannot be imported while in "
                              "translated image coordinate mode.");
      return;
      }
    }

  if (QDir::cleanPath(qtString(this->ImageDataSource->getDataSetSpecifier())) !=
      QDir::cleanPath(qtString(project->DataSetSpecifier)))
    {
    if (QMessageBox::warning(0, QString(),
                             "The project to be imported has a different "
                             "dataset specifier than the currently loaded "
                             "data. The import operation may produce invalid "
                             "tracks or events.\n\nContinue?",
                             QMessageBox::Ok | QMessageBox::Cancel) !=
        QMessageBox::Ok)
      {
      return;
      }
    }

  // Compute a reasonable offset for incoming object ids, assuming the source
  // ids start near 0.
  vtkIdType lastId =
    std::max(currentProject->TrackModel->GetNextAvailableId(),
             currentProject->EventModel->GetNextAvailableId());

  int idsOffset = static_cast<int>(((lastId + 1000 - 1) / 1000) * 1000);

  // Allow the user to tweak the offset as desired.
  bool ok;
  idsOffset =
    QInputDialog::getInt(0, QString(), "Offset to apply to imported object ids:",
                         idsOffset, INT_MIN, INT_MAX, 1, &ok);
  if (!ok)
    {
    return;
    }

  // Point current project's IO manager to the files we are importing from.
  currentIO->SetTracksFileName(project->TracksFile.c_str());
  currentIO->SetTrackTraitsFileName(project->TrackTraitsFile.c_str());
  currentIO->SetEventsFileName(project->EventsFile.c_str());
  currentIO->SetEventLinksFileName(project->EventLinksFile.c_str());
  currentIO->SetActivitiesFileName(project->ActivitiesFile.c_str());
  currentIO->SetFseTracksFileName(project->SceneElementsFile.c_str());

  // Temporarily change the AOI height to that of the input project, so that
  // imported object points get y-flipped correctly.
  SetImageHeight sih(currentIO,
                     static_cast<unsigned int>(project->AnalysisDimensions[1]));

  // Compute the offsets needed to transform the track data to the AOI of the
  // current project.
  float offsetX, offsetY;

  if (this->UseRawImageCoordinates)
    {
    // Compute the image coordinates of the AOI origin for the source project.
    double point[4] = { project->AOIUpperLeftLatLon[1],
                        project->AOIUpperLeftLatLon[0], 0.0, 1.0 };
    this->LatLonToImageReferenceMatrix->MultiplyPoint(point, point);
    if (point[3] != 0)
      {
      point[0] /= point[3];
      point[1] /= point[3];
      }

    // The desired offset is the difference between the AOI origins.
    offsetX = point[0] - this->ImageTranslationReferenceOffset[0];
    offsetY = this->ImageTranslationReferenceOffset[1] - point[1];
    }
  else if (!this->UseGeoCoordinates)
    {
    // Using inverted image coordinates.
    offsetX = static_cast<float>(currentProject->OverviewOrigin[0] -
                                 project->OverviewOrigin[0]);
    offsetY = static_cast<float>(currentProject->OverviewOrigin[1] -
                                 project->OverviewOrigin[1]);
    }
  else
    {
    // Geocoordinate mode, no offset required.
    offsetX = 0.0f;
    offsetY = 0.0f;
    }

  switch (project->IsValid(project->TracksFile))
    {
    case vpProject::FILE_EXIST:
      if (!currentIO->ImportTracks(idsOffset, offsetX, offsetY))
        {
        emit this->warningError("Error occurred while importing tracks.");
        return;
        }
      break;

    case vpProject::FILE_NAME_EMPTY:
      emit this->warningError("Project to import has no tracks file.");
      return;

    default:
      emit this->warningError("Tracks file not found.");
      return;
    }

  switch (project->IsValid(project->EventsFile))
    {
    case vpProject::FILE_EXIST:
      if (!currentIO->ImportEvents(idsOffset, offsetX, offsetY))
        {
        emit this->warningError("Error occurred while importing events.");
        }
      // fall through
    case vpProject::FILE_NAME_EMPTY:
      break;

    default:
      emit this->warningError("Events file not found.");
      break;
    }

  switch (project->IsValid(project->SceneElementsFile))
    {
    case vpProject::FILE_EXIST:
      if (!currentIO->ImportFseTracks(idsOffset, offsetX, offsetY))
        {
        emit this->warningError("Error occurred while importing scene elements.");
        return;
        }
      break;

    case vpProject::FILE_NAME_EMPTY:
      break;

    default:
      emit this->warningError("Scene elements file not found.");
      return;
    }


  // TODO: For completeness, we should import activities as well. But the lack
  // of id tracking in the activitiy manager makes that tricky at the moment.

  this->SessionView->Update(true);
  emit this->objectInfoUpdateNeeded();

  this->UpdateObjectViews = false;
  this->updateScene();

#endif // VISGUI_USE_VIDTK
}

//-----------------------------------------------------------------------------
bool vpViewCore::importTracksFromFile(vpProject* project)
{
  if (project->IsValid(project->TracksFile) == vpProject::FILE_NAME_NOT_EMPTY)
    {
    if (this->loadTracks(project) != VTK_OK)
      {
      const QString msgFormat =
        "Failed to load tracks from file/pattern \"%1\" (for %2)";
      emit this->warningError(msgFormat.arg(qtString(project->TracksFile),
                                            qtString(project->TracksFileTag)));
      }

    // load track traits file if one was given
    if (project->IsValid(project->TrackTraitsFile) == vpProject::FILE_EXIST)
      {
      this->loadTrackTraits(project);
      }

    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vpViewCore::importEventsFromFile(vpProject* project,
                                      bool importTracksSuccessful)
{
  bool result = true;

  // Even though track file may not be valid if user has specified a event file
  // we need to inform user whether or not it is valid or not. This does not include
  // the case when the file name is empty or not given.
  if (project->IsValid(project->EventsFile) == vpProject::FILE_NOT_EXIST)
    {
    this->handleFileNotFound(project->EventsFileTag, project->EventsFile);
    result = false;
    }

  // If track file is valid only then import events.
  if (importTracksSuccessful)
    {
    if (project->IsValid(project->EventsFile) == vpProject::FILE_EXIST)
      {
      this->loadEvents(project);
      }
    else
      {
      result = false;
      }
    }

  return result;
}

//-----------------------------------------------------------------------------
bool vpViewCore::importEventLinksFromFile(vpProject* project,
                                          bool eventsImportSuccessful)
{
  if (project->IsValid(project->EventLinksFile) ==
      vpProject::FILE_NOT_EXIST)
    {
    this->handleFileNotFound(project->EventLinksFileTag,
                             project->EventLinksFile);
    return false;
    }

  if (!eventsImportSuccessful)
    {
    return false;
    }

  if (project->IsValid(project->EventLinksFile) !=
      vpProject::FILE_EXIST)
    {
    return false;
    }

  this->loadEventLinks(project);
  return true;
}

//-----------------------------------------------------------------------------
bool vpViewCore::importActivitiesFromFile(vpProject* project,
                                          bool eventsImportSuccessful)
{
  bool result  = true;

  if (project->IsValid(project->ActivitiesFile) == vpProject::FILE_NOT_EXIST)
    {
    this->handleFileNotFound(project->ActivitiesFileTag,
                             project->ActivitiesFile);
    result = false;
    }

  // Only if events file is valid load the activities file.
  if (eventsImportSuccessful)
    {
    if (project->IsValid(project->ActivitiesFile) == vpProject::FILE_EXIST)
      {
      this->loadActivities(project);
      }
    else
      {
      result = false;
      }
    }


  return result;
}

//-----------------------------------------------------------------------------
bool vpViewCore::importSceneElementsFromFile(vpProject* project)
{
  if (project->IsValid(project->SceneElementsFile) == vpProject::FILE_NOT_EXIST)
    {
    this->handleFileNotFound(project->SceneElementsFileTag, project->SceneElementsFile);
    return false;
    }

  if (project->IsValid(project->SceneElementsFile) == vpProject::FILE_EXIST)
    {
    this->loadSceneElements(project);
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vpViewCore::importIconsFromFile(vpProject* project)
{
  bool result = true;

  if (project->IsValid(project->IconsFile) == vpProject::FILE_NOT_EXIST)
    {
    this->handleFileNotFound(project->IconsFileTag, project->IconsFile);
    result = false;
    }
  else if (project->IsValid(project->IconsFile) == vpProject::FILE_EXIST)
    {
    this->loadIcons(project);
    }
  else
    {
    result = false;
    }

  if (this->Projects.empty())
    {
    this->createEventLegend(project);
    }

  return result;
}

//-----------------------------------------------------------------------------
bool vpViewCore::importOverviewFromFile(vpProject* project)
{
  bool result = true;

  if (project->IsValid(project->OverviewFile) == vpProject::FILE_EXIST)
    {
    const int levelOfDetailToLoad = 3;
    this->ImageSource->SetFileName(project->OverviewFile.c_str());
    this->ImageSource->SetLevel(levelOfDetailToLoad);
    this->ImageSource->Update();
    this->ImageData[0]->DeepCopy(this->ImageSource->GetOutput());
    this->ImageData[0]->GetBounds(this->OverviewImageBounds);

    this->OverviewExtents[0] = this->OverviewImageBounds[0];
    this->OverviewExtents[1] = this->OverviewImageBounds[1];
    this->OverviewExtents[2] = this->OverviewImageBounds[2];
    this->OverviewExtents[3] = this->OverviewImageBounds[3];

    double origins[3];
    this->ImageData[0]->GetOrigin(origins);
    if (project->OverviewSpacing != -1)
      {
      this->ImageData[0]->SetSpacing(project->OverviewSpacing,
                                     project->OverviewSpacing, 1);
      }
    else
      {
      this->ImageData[0]->SetSpacing(
        1 << levelOfDetailToLoad, 1 << levelOfDetailToLoad, 1);
      }
    this->ImageData[0]->SetOrigin(project->OverviewOrigin[0],
                                  project->OverviewOrigin[1],
                                  0);
    this->emit overviewLoaded();
    }
  else
    {
    result = false;
    }

  return result;
}

//-----------------------------------------------------------------------------
bool vpViewCore::importNormalcyMapsFromFile(vpProject* project)
{
  if (project->IsValid(project->NormalcyMapsFile) == vpProject::FILE_EXIST)
    {
    return this->NormalcyMaps->LoadFromFile(
             qtString(project->NormalcyMapsFile));
    }

  return false;
}

//-----------------------------------------------------------------------------
void vpViewCore::exportTracksToFile()
{
  if (this->isEditingTrack())
    {
    emit this->warningError(
      "Cannot export tracks while a track is being edited.");
    return;
    }

  QString filter = "*.kw18;;*.json";
  vgFileDialog fileDialog(0, tr("Export Tracks"), QString(), filter);
  fileDialog.setObjectName("ExportTracks");
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setDefaultSuffix("kw18");
  if (fileDialog.exec() != QDialog::Accepted)
    {
    return;
    }

  QStringList files = fileDialog.selectedFiles();

  QMessageBox msgBox;
  msgBox.setWindowTitle("Writing tracks...");
  msgBox.setText("Writing tracks...");
  msgBox.show();

  int session = this->SessionView->GetCurrentSession();
  bool success;
  if (QFileInfo(files[0]).suffix() == "json")
    {
    success = this->Projects[session]->ModelIO->WriteFseTracks(
                qPrintable(files[0]), false);
    }
  else
    {
    success = this->Projects[session]->ModelIO->WriteTracks(
                qPrintable(files[0]));
    }

  if (!success)
    {
    QMessageBox::warning(0, "vpView", "Error writing tracks file.");
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::exportEventsToFile()
{
  QString filter = "*.txt;;";
  vgFileDialog fileDialog(0, tr("Export Events"), QString(), filter);
  fileDialog.setObjectName("ExportEvents");
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setDefaultSuffix("txt");
  if (fileDialog.exec() != QDialog::Accepted)
    {
    return;
    }

  QStringList files = fileDialog.selectedFiles();

  QMessageBox msgBox;
  msgBox.setWindowTitle("Writing events...");
  msgBox.setText("Writing events...");
  msgBox.show();

  int session = this->SessionView->GetCurrentSession();
  if (!this->Projects[session]->ModelIO->WriteEvents(qPrintable(files[0])))
    {
    QMessageBox::warning(0, "vpView", "Error writing events file.");
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::exportSceneElementsToFile()
{
  QString filter = "*.json;;";
  vgFileDialog fileDialog(0, tr("Export Scene Elements"), QString(), filter);
  fileDialog.setObjectName("ExportSceneElements");
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setDefaultSuffix("json");
  if (fileDialog.exec() != QDialog::Accepted)
    {
    return;
    }

  QStringList files = fileDialog.selectedFiles();

  QMessageBox msgBox;
  msgBox.setWindowTitle("Writing scene elements...");
  msgBox.setText("Writing scene elements...");
  msgBox.show();

  int session = this->SessionView->GetCurrentSession();
  vpProject* project = this->Projects[session];

  if (!project->ModelIO->WriteFseTracks(qPrintable(files[0])))
    {
    QMessageBox::warning(0, "vpView", "Error writing scene elements file.");
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::exportImageTimeStampsToFile()
{
  QString filter = "*.txt;;";
  vgFileDialog fileDialog(0, tr("Export Image Timestamps"), QString(), filter);
  fileDialog.setObjectName("ExportImageTimeStamps");
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setDefaultSuffix("txt");
  if (fileDialog.exec() != QDialog::Accepted)
    {
    return;
    }

  std::ofstream out(qPrintable(fileDialog.selectedFiles().front()));
  if (!out.is_open())
    {
    emit this->warningError("Could not open image timestamp file for writing.");
    return;
    }

  out.precision(20);
  std::vector<std::pair<std::string, double> > imageTimes;
  this->FrameMap->exportImageTimes(imageTimes);
  for (size_t i = 0, size = imageTimes.size(); i < size; ++i)
    {
    out << imageTimes[i].first << ' ' << imageTimes[i].second / 1e6 << '\n';
    }
}

//-----------------------------------------------------------------------------
bool vpViewCore::createTrack(int trackId, int session, bool isFse)
{
  vtkVgTrack* track = this->makeTrack(trackId, session);
  if (isFse)
    {
    track->SetDisplayFlags(vtkVgTrack::DF_SceneElement);
    }

  this->NewTrackId = trackId;

  // Compute the point in the center of the view bounds, so that the initial
  // follow of the track doesn't cause the camera to move.
  double bounds[4];
  vtkVgRendererUtils::GetBounds(this->SceneRenderer, bounds);
  double center[4] =
    {
    0.5 * (bounds[0] + bounds[1]),
    0.5 * (bounds[2] + bounds[3]),
    0.0,
    1.0
    };

  vtkMatrix4x4* trackToWorld =
    this->Projects[session]->TrackRepresentation->GetRepresentationMatrix();

  vtkSmartPointer<vtkMatrix4x4> worldToTrack = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkMatrix4x4::Invert(trackToWorld, worldToTrack);
  worldToTrack->MultiplyPoint(center, center);
  center[0] /= center[3];
  center[1] /= center[3];

  // Insert a placeholder point (to avoid warnings and other issues), but don't
  // display this track until the user adds a point of their own.
  this->NewTrackTimeStamp = this->CoreTimeStamp;
  track->InsertNextPoint(this->NewTrackTimeStamp, center, vtkVgGeoCoord());
  this->Projects[session]->TrackRepresentation->SetExcludedTrack(track);
  this->Projects[session]->TrackHeadRepresentation->SetExcludedTrack(track);
  this->Projects[session]->SceneElementRepresentation->SetExcludedTrack(track);

  return true;
}

//-----------------------------------------------------------------------------
int vpViewCore::createEvent(int type, vtkIdList* ids, int session)
{
  if (vtkVgEvent* event =
        this->Projects[session]->EventModel->CreateAndAddEvent(type, ids))
    {
    return event->GetId();
    }

  return -1;
}

//-----------------------------------------------------------------------------
void vpViewCore::improveTrack(int trackId, int session)
{
#ifdef VISGUI_USE_KWIVER
  namespace kv = kwiver::vital;

  // Get selected track
  auto project = this->Projects[session];
  auto* track = project->TrackModel->GetTrack(trackId);

  if (!track)
    {
    return;
    }

  // Ensure time map is complete
  if (this->UsingTimeStampData && !this->waitForFrameMapRebuild())
    {
    return;
    }

  // Set up progress dialog
  QProgressDialog progress;
  progress.setLabelText("Improving track...");
  progress.setCancelButton(nullptr);

  // Create video source
  const auto videoSource =
    std::make_shared<vpKwiverVideoSource>(this->ImageDataSource);
  const auto videoHeight =
    static_cast<double>(project->ModelIO->GetImageHeight());

  // Set up worker
  vpKwiverImproveTrackWorker worker;
  if (!worker.initialize(track, project->TrackModel, videoSource, videoHeight))
    {
    return;
    }

  connect(&worker, SIGNAL(progressRangeChanged(int, int)),
          &progress, SLOT(setRange(int, int)));
  connect(&worker, SIGNAL(progressValueChanged(int)),
          &progress, SLOT(setValue(int)));

  // Execute worker and get resulting track
  auto improvedTrack = worker.execute();
  if (!improvedTrack)
    {
    QMessageBox::critical(0, "Improvement Failed",
                          "The track could not be improved.");
    return;
    }

  // Add the new states to the track
  const auto& timeMap = this->FrameMap->getTimeMap();

  for (auto s : *improvedTrack)
    {
    auto state = std::dynamic_pointer_cast<kv::object_track_state>(s);
    if (!state || !state->detection)
      {
      // Bad state (shouldn't happen, but better safe than SEGV'd)
      continue;
      }

    // Get time stamp for frame
    const auto& ts = [&timeMap](const kv::object_track_state& state){
      const auto frame = static_cast<unsigned int>(state.frame());

      if (!timeMap.empty())
        {
        const auto& ti = timeMap.find(frame);
        return (ti == timeMap.end() ? vtkVgTimeStamp{}
                                    : vtkVgTimeStamp{ti->second});
        }

      return vtkVgTimeStamp{vgTimeStamp::InvalidTime(), frame};
    }(*state);

    if (!ts.IsValid())
      {
      // Unable to resolve time stamp (shouldn't happen?)
      continue;
      }

    // Replace track point
    auto narrow = [](double x){ return static_cast<float>(x); };
    const auto& bbox = state->detection->bounding_box();
    const auto points = std::array<float, 12>{{
      narrow(bbox.min_x()), narrow(videoHeight - bbox.min_y()), 0.0f,
      narrow(bbox.max_x()), narrow(videoHeight - bbox.min_y()), 0.0f,
      narrow(bbox.max_x()), narrow(videoHeight - bbox.max_y()), 0.0f,
      narrow(bbox.min_x()), narrow(videoHeight - bbox.max_y()), 0.0f
    }};

    const auto& center = bbox.center();

    track->SetPoint(ts, center.data(), {}, 4, points.data());
    }


  project->TrackModel->Modified();
  emit this->objectInfoUpdateNeeded();
#endif
}

//-----------------------------------------------------------------------------
bool vpViewCore::splitTrack(int trackId, int newTrackId, int session)
{
  auto project = this->Projects[session];
  vtkVgTrack* track = project->TrackModel->GetTrack(trackId);

  if (!track)
    {
    return false;
    }

  if (!project->TrackModel->GetIsKeyframe(trackId, this->CoreTimeStamp) ||
      track->GetStartFrame() >= this->CoreTimeStamp ||
      track->GetEndFrame() <= this->CoreTimeStamp)
    {
    return false;
    }

  // Initially, the new track contains all the data of the original
  vtkVgTrack* newTrack = this->cloneTrack(trackId, newTrackId, session);
  if (!newTrack)
    {
    return false;
    }

  // Trim beginning of new track up to (but not including) the split frame
  while (newTrack->GetStartFrame() < this->CoreTimeStamp)
    {
    newTrack->DeletePoint(newTrack->GetStartFrame());
    }

  // Trim end of the original track back to (but not including) the split frame
  while (track->GetEndFrame() > this->CoreTimeStamp)
    {
    track->DeletePoint(track->GetEndFrame());
    }

  std::vector<vtkVgEvent*> events;
  project->EventModel->GetEvents(trackId, events);

  int eventsCreated = 0;
  if (!events.empty())
    {
    // Fix track references in events, and split events where necesssary
    bool error = false;
    for (size_t i = 0, end = events.size(); i < end; ++i)
      {
      vtkVgEvent* e = events[i];
      for (unsigned int ti = 0; ti < e->GetNumberOfTracks(); ++ti)
        {
        vtkVgEventTrackInfoBase* eti = e->GetTrackInfo(ti);
        if (eti->TrackId != trackId)
          {
          continue;
          }

        if (this->CoreTimeStamp < eti->StartFrame)
          {
          // Split time before the start of track time range, so just fix up the
          // track reference to point to the newly created track
          e->ReplaceTrack(ti, newTrackId);
          e->SetTrackPtr(ti, newTrack);
          }
        else if (this->CoreTimeStamp > eti->StartFrame &&
                 this->CoreTimeStamp < eti->EndFrame)
          {
          // Time range intersects the split frame, the event must be split
          vtkVgEvent* eNew = this->cloneEvent(e->GetId(), session);
          if (!eNew)
            {
            error = true;
            break;
            }

          // In certain pathological cases, there may be events that refer
          // to the same track more than once. Make sure we fix each ref.
          for (unsigned int ti = 0; ti < e->GetNumberOfTracks(); ++ti)
            {
            vtkVgEventTrackInfoBase* eti = e->GetTrackInfo(ti);
            if (eti->TrackId == trackId &&
                this->CoreTimeStamp > eti->StartFrame &&
                this->CoreTimeStamp < eti->EndFrame)
              {
              eNew->ReplaceTrack(ti, newTrackId);
              eNew->SetTrackPtr(ti, newTrack);
              eNew->SetTrackStartFrame(ti, this->CoreTimeStamp);
              e->SetTrackEndFrame(ti, this->CoreTimeStamp);
              }
            }

          // Done with this event
          ++eventsCreated;
          break;
          }
        }
      }

    if (error)
      {
      QMessageBox::warning(0, "Warning", "Failed to split one or more events.");
      }
    }

  if (eventsCreated > 0)
    {
    project->EventModel->Modified();
    this->SessionView->Update(true);
    emit this->objectInfoUpdateNeeded();
    emit this->showStatusMessage(
      QString("%1 event(s) created").arg(eventsCreated));
    }

  this->update();
  return true;
}

//-----------------------------------------------------------------------------
bool vpViewCore::mergeTracks(int trackA, int trackB, int session)
{
  vtkVgTrack* a = this->Projects[session]->TrackModel->GetTrack(trackA);
  vtkVgTrack* b = this->Projects[session]->TrackModel->GetTrack(trackB);

  if (a == 0 || b == 0)
    {
    return false;
    }

  a->Merge(b);
  return true;
}

//-----------------------------------------------------------------------------
vtkVgTrack* vpViewCore::cloneTrack(int trackId, int newTrackId, int session)
{
  vtkVgTrack* track = this->Projects[session]->TrackModel->GetTrack(trackId);
  if (!track)
    {
    return 0;
    }

  // make sure id is available
  if (this->Projects[session]->TrackModel->GetTrack(newTrackId))
    {
    return 0;
    }

  vtkVgTrack* newTrack = this->makeTrack(newTrackId, session);
  newTrack->CopyData(track);
  return newTrack;
}

//-----------------------------------------------------------------------------
vtkVgEvent* vpViewCore::cloneEvent(int eventId, int session)
{
  vtkVgEventModel* eventModel = this->Projects[session]->EventModel;
  return eventModel->CloneEvent(eventId);
}

//-----------------------------------------------------------------------------
bool vpViewCore::setAntialiasing(bool state)
{
  if (this->RenderWindow)
    {
    this->RenderWindow->SetMultiSamples(state ? 1 : 0);
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool vpViewCore::getAntialiasing() const
{
  return this->RenderWindow->GetMultiSamples() > 0;
}

//-----------------------------------------------------------------------------
void vpViewCore::setImageFiltering(bool state)
{
  this->ImageActor[1]->SetInterpolate(state ? 1 : 0);
}

//-----------------------------------------------------------------------------
bool vpViewCore::getImageFiltering() const
{
  return this->ImageActor[1]->GetInterpolate() != 0;
}

//-----------------------------------------------------------------------------
void vpViewCore::setup3dWidget(vpQtViewer3dWidget* viewer3dWidget)
{
  if (!viewer3dWidget)
    {
    qDebug() << "ERROR: Invalid (null) viewer 3d widget";
    return;
    }

  viewer3dWidget->getViewer3d()->setProject(this->Projects[0]);
  viewer3dWidget->getViewer3d()->setContextDataSource(this->ImageDataSource);
  viewer3dWidget->getViewer3d()->addRepresentation(
    this->Projects[0]->TrackRepresentation);
  viewer3dWidget->getViewer3d()->addRepresentation(
    this->GraphRepresentation);

  viewer3dWidget->getViewer3d()->setViewCoreInstance(this);
}

//-----------------------------------------------------------------------------
void vpViewCore::setIdOfTrackToFollow(vtkIdType trackId)
{
  int projectId =
    this->Projects[this->SessionView->GetCurrentSession()]->ProjectId;

  if (this->IdOfTrackToFollow == trackId &&
      this->FollowTrackProjectId == projectId)
    {
    return;
    }

  this->IdOfTrackToFollow = trackId;
  this->FollowTrackProjectId = projectId;

  emit followTrackChange(static_cast<int>(this->IdOfTrackToFollow));

  if (this->IdOfTrackToFollow > -1)
    {
    this->updateTrackFollowCamera();
    this->render();
    }
}

//-----------------------------------------------------------------------------
int vpViewCore::loadTracks(vpProject* project)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle("Loading tracks...");
  msgBox.setText("Loading tracks (may take awhile)...");
  msgBox.setModal(false);
  msgBox.show();

  if (!project->ModelIO->ReadTracks())
    {
    return VTK_ERROR;
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vpViewCore::loadTrackTraits(vpProject* project)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle("Loading track traits...");
  msgBox.setText("Loading track traits (may take awhile)...");
  msgBox.setModal(false);
  msgBox.show();

  if (!project->ModelIO->ReadTrackTraits())
    {
    return VTK_ERROR;
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vpViewCore::loadEvents(vpProject* project)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle("Loading events...");
  msgBox.setText("Loading events (may take awhile)...");
  msgBox.setModal(false);
  msgBox.show();

  if (!project->ModelIO->ReadEvents())
    {
    return VTK_ERROR;
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vpViewCore::loadEventLinks(vpProject* project)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle("Loading event links...");
  msgBox.setText("Loading event links (may take awhile)...");
  msgBox.setModal(false);
  msgBox.show();

  if (!project->ModelIO->ReadEventLinks())
    {
    return VTK_ERROR;
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vpViewCore::loadActivities(vpProject* project)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle("Loading activities...");
  msgBox.setText("Loading activities (may take awhile)...");
  msgBox.setModal(false);
  msgBox.show();

  if (!project->ModelIO->ReadActivities())
    {
    return VTK_ERROR;
    }

  // Set disply full volume from the project file.
  if (project->PrecomputeActivity != 0)
    {
    project->ActivityManager->SetShowFullVolume(true);
    project->ActivityManager->SetVisibility(true);
    project->ActivityManager->UpdateActivityActors(this->CoreTimeStamp);
    project->ActivityManager->SetVisibility(false);
    project->ActivityManager->SetShowFullVolume(false);
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vpViewCore::loadSceneElements(vpProject* project)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle("Loading scene elements...");
  msgBox.setText("Loading scene elements (may take awhile)...");
  msgBox.setModal(false);
  msgBox.show();

  if (!project->ModelIO->ReadFseTracks())
    {
    return VTK_ERROR;
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vpViewCore::loadIcons(vpProject* project)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle("Loading icons...");
  msgBox.setText("Loading icons (may take awhile)...");
  msgBox.setModal(false);
  msgBox.show();

  if (project->IconManager->LoadIcons(project->IconsFile.c_str()) != VTK_OK)
    {
    return VTK_ERROR;
    }

  this->SceneRenderer->AddViewProp(project->IconManager->GetIconActor());
  emit this->iconsLoaded();

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vpViewCore::updateTracks()
{
  bool hasNonFileProjects = false;

#ifdef VISGUI_USE_VIDTK
  for (size_t i = 0, size = this->Projects.size(); i < size; ++i)
    {
    if (!this->Projects[i]->ModelIO.dynamicCast<vpVidtkFileIO>())
      {
      hasNonFileProjects = true;
      break;
      }
    }
#endif

  // If we only have file-backed projects then there is no point in updating.
  if (!hasNonFileProjects)
    {
    return VTK_ERROR;
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vpViewCore::dataChanged()
{
  emit this->dataLoaded();
  emit this->frameChanged();
}

//-----------------------------------------------------------------------------
void vpViewCore::refreshView()
{
  if (this->Projects.empty())
    {
    return;
    }

  // AOI outline.
  this->createDisplayOutline(this->Projects[0]);

  this->resetView();

  // Set overview (larger area) display. Order matters here.
  this->setOverviewDisplay(this->Projects[0]);
}

//-----------------------------------------------------------------------------
void vpViewCore::reset()
{
  this->sanitizeSceneRenderer();
}

//-----------------------------------------------------------------------------
void vpViewCore::initializeAllOthers()
{
  this->AdjudicationMode            = false;
  this->AdjudicationAllTracksState  = false;

  this->TrackOffset[0]  = this->TrackOffset[1] = 0;

  this->DisplayMode                 = this->OverViewMode;

  this->Loop                        = true;
  this->NumberOfFrames              = 0;
  this->FrameNumberOffset           = 0;
  this->UseZeroBasedFrameNumbers    = false;
  this->RightClickToEditEnabled     = true;
  this->AutoAdvanceDuringCreation   = true;
  this->SceneElementLineWidth       = 3.0;

  this->CoreTimeStamp.SetFrameNumber(0);
  this->CurrentFrame = 0;
  this->LastFrame = -1;
  this->CurrentTime = 0.0;

  this->TimeLogger                  = vtkSmartPointer<vtkTimerLog>::New();
  this->LoadRenderTimeLogger        = vtkSmartPointer<vtkTimerLog>::New();

  this->RenderingTime               = 0.0;
  this->RequestedFPS                = 2.0;

  this->VideoAnimation->setFrameInterval(1.0 / this->RequestedFPS);

  this->InteractionCallback         = vtkSmartPointer<vtkVpInteractionCallback>::New();
  this->InteractionCallback->ViewCoreInstance = this;

  this->RubberbandInteractorStyle->AddObserver(
    vtkCommand::InteractionEvent, this->InteractionCallback);
  this->RubberbandInteractorStyle->AddObserver(
    vtkVgInteractorStyleRubberBand2D::ZoomCompleteEvent, this->InteractionCallback);

  this->InformaticsDialog           = new vpInformaticsDialog(0);

  this->RenderTimeLogger = vtkSmartPointer<vtkTimerLog>::New();
  this->TotalRenderTime             = 0.0;
  this->TotalNumberOfRenderCalls    = 0.0;

  this->ImageDataSource             = new vpFileDataSource();
  this->FrameMap                    = new vpFrameMap(this->ImageDataSource);

  connect(this->ImageDataSource, SIGNAL(dataFilesChanged()), this,
          SLOT(reactToDataChanged()));

  this->ProjectParser               = new vpProjectParser();

  this->ActivityTypeRegistry        = vtkVgActivityTypeRegistry::New();
  this->EventTypeRegistry           = vpEventTypeRegistry::New();
  this->TrackTypeRegistry           = vtkVgTrackTypeRegistry::New();

  this->TrackConfig                 = new vpTrackConfig(this->TrackTypeRegistry);
  this->EventConfig                 = new vpEventConfig(this->EventTypeRegistry);
  this->ActivityConfig              = new vpActivityConfig(this->ActivityTypeRegistry);

  this->NormalcyMaps                = new vpNormalcyMaps();

  this->Annotation                  = new vpAnnotation;
  this->Annotation->SetViewCoreInstance(this);

  // Add actor for text annotations
  this->SceneRenderer->AddViewProp(this->Annotation->GetProp());
  this->ShowAnnotations             = true;

  this->TrackStorageMode            = vpTrackIO::TSM_InvertedImageCoords;
  this->UseGeoCoordinates           = false;
  this->UseRawImageCoordinates      = false;
  this->ImageTranslationReferenceLatLon[0] = 444;
  this->ImageTranslationReferenceLatLon[1] = 444;
  this->ImageTranslationOffset[0] = 0;
  this->ImageTranslationOffset[1] = 0;

  this->IdOfTrackToFollow           = -1;
  this->FollowTrackProjectId        = -1;

  this->SaveRenderedImages          = false;
  this->ImageOutputDirectory        = "";
}

//-----------------------------------------------------------------------------
void vpViewCore::initializeData()
{
  this->ImageCounter = 1;

  for (int i = 0; i < 2; i++)
    {
    this->ImageData[i] = vtkSmartPointer<vtkImageData>::New();
    this->ImageActor[i] = vtkSmartPointer<vtkImageActor>::New();
    this->ImageActor[i]->SetInputData(this->ImageData[i]);
    }

  this->AOIImage = vtkSmartPointer<vtkImageData>::New();

  this->MainImageData = this->ImageData[1];

  // Image souce intialized to NULL.
  this->ImageSource = NULL;

  this->ImageSourceLODFactor = 1.0;

  this->AOIOutlinePolyData = 0;
  this->AOIOutlineActor    = 0;

  this->VTKConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();

  // Legend stuff.
  this->EventLegend = vtkSmartPointer<vtkLegendBoxActor>::New();

  // setup outputing images from render window
  this->WindowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
  this->WindowToImageFilter->SetInput(this->RenderWindow);
  this->PNGWriter = vtkSmartPointer<vtkPNGWriter>::New();
  this->PNGWriter->SetInputConnection(this->WindowToImageFilter->GetOutputPort());

  // populate track state attributes
#ifdef VISGUI_USE_VIDTK
  this->TrackAttributes.Clear();
  this->TrackAttributes.SetMask("ForegroundTracking", "SSD",
    vidtk::track_state_attributes::ATTR_ASSOC_FG_SSD);
  this->TrackAttributes.SetMask("ForegroundTracking", "CSURF",
    vidtk::track_state_attributes::ATTR_ASSOC_FG_CSURF);

  this->TrackAttributes.SetMask("DataAssociationTracking", "Kinematic",
    vidtk::track_state_attributes::ATTR_ASSOC_DA_KINEMATIC);
  this->TrackAttributes.SetMask("DataAssociationTracking", "MultiFeatures",
    vidtk::track_state_attributes::ATTR_ASSOC_DA_MULTI_FEATURES);
  this->TrackAttributes.SetMask("DataAssociationTracking", "Multiple",
    vidtk::track_state_attributes::ATTR_ASSOC_DA_KINEMATIC |
    vidtk::track_state_attributes::ATTR_ASSOC_DA_MULTI_FEATURES);

  this->TrackAttributes.SetMask("KalmanFilter", "Extended",
    vidtk::track_state_attributes::ATTR_KALMAN_ESH);
  this->TrackAttributes.SetMask("KalmanFilter", "Linear",
    vidtk::track_state_attributes::ATTR_KALMAN_LVEL);

  this->TrackAttributes.SetMask("Interval", "Forward",
    vidtk::track_state_attributes::ATTR_INTERVAL_FORWARD);
  this->TrackAttributes.SetMask("Interval", "Back",
    vidtk::track_state_attributes::ATTR_INTERVAL_BACK);
  this->TrackAttributes.SetMask("Interval", "Init",
    vidtk::track_state_attributes::ATTR_INTERVAL_INIT);

  this->TrackAttributes.SetMask("Linking", "Start",
    vidtk::track_state_attributes::ATTR_LINKING_START);
  this->TrackAttributes.SetMask("Linking", "End",
    vidtk::track_state_attributes::ATTR_LINKING_END);
  this->TrackAttributes.SetMask("Linking", "Multiple",
    vidtk::track_state_attributes::ATTR_LINKING_START |
    vidtk::track_state_attributes::ATTR_LINKING_END);
#endif

  // turn off attributes that have been hidden in settings
  QSettings settings;
  settings.beginGroup("ShowTrackAttribute");
  foreach (const QString& group, settings.childKeys())
    {
    if (!settings.value(group).toBool())
      {
      this->TrackAttributes.SetGroupEnabled(stdString(group), false);
      }
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::initializeDisplay()
{
  this->ShowTracks = false;
  this->ShowTrackHeads = false;
  this->ShowEvents = false;
  this->ShowEventIcons = false;
  this->ShowActivities = false;
  this->ShowSceneElements = false;

  this->DisplayFullVolume = false;

  this->UpdatePending = false;
  this->UpdateObjectViews = false;
  this->ForceFullUpdate = false;
  this->RenderPending = false;
  this->LogRenderTime = true;

  this->RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  this->RenderWidget = 0;

  // Main Renderer
  this->SceneRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->SceneRenderer->GetActiveCamera()->ParallelProjectionOn();

  // "small" renderer in lower left which gives some sort of context for the large renderer
  this->ContextRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->ContextRenderer->GetActiveCamera()->ParallelProjectionOn();
  // background renderer to give border around the context renderer
  this->BackgroundRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->BackgroundRenderer->SetBackground(0.0, 0.0, 1.0);

  // initially context and background are both off (these should always be both on or both off)
  this->ContextRenderer->DrawOff();
  this->BackgroundRenderer->DrawOff();
  // and no interaction in context (or background)
  this->ContextRenderer->InteractiveOff();
  this->BackgroundRenderer->InteractiveOff();

  // SceneRenderer 1st, followed by BackgroundRenderer, and then ContextRenderer, so that
  // we know that result will be layered correctly
  this->RenderWindow->AddRenderer(this->SceneRenderer);
  this->RenderWindow->AddRenderer(this->BackgroundRenderer);
  this->RenderWindow->AddRenderer(this->ContextRenderer);

  // setup BackgroundRenderer for the lower-left corner
  this->BackgroundRenderer->SetViewport(0, 0, 0.25, 0.25);
  // the ContextRenderer will be set once the renderwindow has been "realized"...
  // not sure when that is so will be "over" doing it for now (every time a new image
  // is displayed when there is a context "window"

  this->RubberbandInteractorStyle =
    vtkSmartPointer<vtkVgInteractorStyleRubberBand2D>::New();
  this->RubberbandInteractorStyle->SetRenderer(this->SceneRenderer);

  this->TrackHeadIndicatorActor = vtkSmartPointer<vtkActor>::New();
  this->TrackHeadIndicatorActor->SetVisibility(0);

  // Set up the ruler line for measuring distance
    {
    this->RulerPoints = vtkSmartPointer<vtkPoints>::New();
    this->RulerPoints->SetNumberOfPoints(2);

    vtkNew<vtkPolyData> pd;
    vtkNew<vtkCellArray> ca;
    vtkNew<vtkPolyDataMapper2D> pdm;

    vtkIdType ids[] = { 0, 1 };
    ca->InsertNextCell(2, ids);
    pd->SetPoints(this->RulerPoints);
    pd->SetLines(ca.GetPointer());
    pd->SetVerts(ca.GetPointer());
    pdm->SetInputData(pd.GetPointer());

    this->RulerActor = vtkSmartPointer<vtkActor2D>::New();
    this->RulerActor->SetVisibility(0);
    this->RulerActor->SetMapper(pdm.GetPointer());
    this->RulerActor->GetProperty()->SetColor(1.0, 1.0, 0.0);
    this->RulerActor->GetProperty()->SetLineWidth(2.0f);
    this->RulerActor->GetProperty()->SetPointSize(3.0f);
    }

  this->SceneRenderer->AddViewProp(this->TrackHeadIndicatorActor);
  this->SceneRenderer->AddViewProp(this->RulerActor);

  this->EventExpirationMode = ShowUntilEventEnd;

  this->SessionView = 0;

  this->YFlipMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->YFlipMatrix->SetElement(1, 1, -1.0);  // y dim set later
  this->ImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->WorldToImageMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->LatLonToImageMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->LatLonToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->LatLonToImageReferenceMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
}

//-----------------------------------------------------------------------------
void vpViewCore::initializeExtentsBounds()
{
  for (int i = 0; i < 4; ++i)
    {
    this->ViewExtents[i] = -1.0;
    this->OverviewExtents[i] = -1.0;
    }

  for (int i = 0; i < 6; ++i)
    {
    this->WholeImageBounds[i] = 0.0;
    this->OverviewImageBounds[i] = 0.0;
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::initializeScene()
{
  this->SceneInitialized = false;
  this->OverlayOpacity = 0.3;
  this->SceneElementFillOpacity = 0.2;
  this->IconSize = 16;
  this->IconOffsetX = this->IconOffsetY = 5;

  this->TrackFilter = vtkSmartPointer<vtkVgTrackFilter>::New();
  this->TrackFilter->SetShowType(vtkVgTrack::Person, true);
  this->TrackFilter->SetShowType(vtkVgTrack::Vehicle, true);
  this->TrackFilter->SetShowType(vtkVgTrack::Other, true);
  this->TrackFilter->SetMinProbability(vtkVgTrack::Person, 0.0);
  this->TrackFilter->SetMinProbability(vtkVgTrack::Vehicle, 0.0);
  this->TrackFilter->SetMinProbability(vtkVgTrack::Other, 0.0);
  this->TrackFilter->SetMaxProbability(vtkVgTrack::Person, 1.0);
  this->TrackFilter->SetMaxProbability(vtkVgTrack::Vehicle, 1.0);
  this->TrackFilter->SetMaxProbability(vtkVgTrack::Other, 1.0);

  this->ContourOperatorManager =
    vtkSmartPointer<vtkVgContourOperatorManager>::New();

  this->TemporalFilters =
    vtkSmartPointer<vtkVgTemporalFilters>::New();

  this->TrackEditProjectId = -1;

  this->EventFilter = vtkSmartPointer<vtkVgEventFilter>::New();

  this->GraphModel = vtkSmartPointer<vtkVgGraphModel>::New();
  this->GraphRepresentation = vtkSmartPointer<vtkVgGraphRepresentation>::New();
  this->GraphRepresentation->SetGraphModel(this->GraphModel);
}

//-----------------------------------------------------------------------------
void vpViewCore::initializeViewInteractions()
{
  this->VTKConnect->Connect(
    this->RenderWindow->GetInteractor()->GetInteractorStyle(),
    vtkVgInteractorStyleRubberBand2D::KeyPressEvent_A,
    this, SLOT(resetToAOIView()));

  this->VTKConnect->Connect(
    this->RenderWindow->GetInteractor()->GetInteractorStyle(),
    vtkVgInteractorStyleRubberBand2D::KeyPressEvent_R,
    this, SLOT(resetToViewExtents()));

  this->VTKConnect->Connect(
    this->RenderWindow->GetInteractor()->GetInteractorStyle(),
    vtkVgInteractorStyleRubberBand2D::KeyPressEvent_Z,
    this, SLOT(decreaseTrackHeadSize()));

  this->VTKConnect->Connect(
    this->RenderWindow->GetInteractor()->GetInteractorStyle(),
    vtkVgInteractorStyleRubberBand2D::KeyPressEvent_S,
    this, SLOT(increaseTrackHeadSize()));

  this->VTKConnect->Connect(
    this->RenderWindow->GetInteractor()->GetInteractorStyle(),
    vtkVgInteractorStyleRubberBand2D::KeyPressEvent_P,
    this, SLOT(pickScene()));

  this->VTKConnect->Connect(
    this->RenderWindow->GetInteractor()->GetInteractorStyle(),
    vtkVgInteractorStyleRubberBand2D::KeyPressEvent_Delete,
    this, SLOT(deleteTrackPoint()));

  // setup widget for detecting mouse hover events
  this->HoveredActivity = 0;
  this->IsHovering = false;
  this->HoverWidget = vtkSmartPointer<vtkHoverWidget>::New();
  this->HoverWidget->SetInteractor(this->RenderWindow->GetInteractor());
  this->HoverWidget->SetTimerDuration(200);
  this->HoverWidget->On();
  this->HoverWidget->Render();

  this->VTKConnect->Connect(this->HoverWidget, vtkCommand::TimerEvent,
                            this, SLOT(onHoverStart()));
  this->VTKConnect->Connect(this->HoverWidget, vtkCommand::EndInteractionEvent,
                            this, SLOT(onHoverStop()));

}

//-----------------------------------------------------------------------------
void vpViewCore::initializeSources()
{
  RegisterVpImageSource registerPNGImageSouce(&vtkVgPNGReader::Create);
  RegisterVpImageSource registerMRJImageSouce(&vtkVgMultiResJpgImageReader2::Create);
  RegisterVpImageSource registerJP2ImageSource(&vtkVgImageSource::Create);
  RegisterVpImageSource registerJPEGImageSource(&vtkVgJPEGReader::Create);

#if defined(VISGUI_USE_GDAL)
  RegisterVpImageSource registerGDALImageSource(&vtkVgGDALReader::Create);
#endif
}

//-----------------------------------------------------------------------------
void vpViewCore::reinitialize()
{
  this->initializeScene();

  this->CoreTimeStamp.Reset();
  this->CoreTimeStamp.SetFrameNumber(0);
  this->CurrentFrame = 0;
  this->LastFrame = -1;
  this->CurrentTime = 0.0;

  this->IdOfTrackToFollow = -1;
  this->FollowTrackProjectId = -1;

  emit this->reinitialized();
}

//-----------------------------------------------------------------------------
void vpViewCore::setOverviewDisplayState(bool state)
{
  if (state)
    {
    this->BackgroundRenderer->DrawOn();
    this->ContextRenderer->DrawOn();
    int* wsize = this->RenderWindow->GetSize();
    // 3 pixel border
    double xDelta =  3.0 / wsize[0];
    double yDelta =  3.0 / wsize[1];
    this->ContextRenderer->SetViewport(0.0 + xDelta, 0.0 + yDelta, 0.25 - xDelta, 0.25 - yDelta);
    }
  else
    {
    this->BackgroundRenderer->DrawOff();
    this->ContextRenderer->DrawOff();
    }

  this->render();
}

//-----------------------------------------------------------------------------
void vpViewCore::setupRenderWidget(QVTKWidget* renderWidget)
{
  this->RenderWindow->SetInteractor(renderWidget->GetInteractor());

  renderWidget->SetRenderWindow(this->RenderWindow);

  renderWidget->GetInteractor()->SetInteractorStyle(this->RubberbandInteractorStyle);

  this->VTKConnect->Connect(
    renderWidget->GetInteractor(),
    vtkCommand::LeftButtonPressEvent,
    this, SLOT(onLeftPress()));

  this->VTKConnect->Connect(
    renderWidget->GetInteractor(),
    vtkCommand::LeftButtonReleaseEvent,
    this, SLOT(onLeftRelease()));

  this->VTKConnect->Connect(
    renderWidget->GetInteractor()->GetInteractorStyle(),
    vtkVgInteractorStyleRubberBand2D::LeftClickEvent,
    this, SLOT(onLeftClick()));

  this->VTKConnect->Connect(
    renderWidget->GetInteractor()->GetInteractorStyle(),
    vtkVgInteractorStyleRubberBand2D::SelectionCompleteEvent,
    this, SLOT(onSelectionComplete()));

  this->VTKConnect->Connect(
    renderWidget->GetInteractor(),
    vtkCommand::RightButtonReleaseEvent,
    this, SLOT(onRightClick()));

  this->VTKConnect->Connect(
    renderWidget->GetInteractor(),
    vtkCommand::MouseMoveEvent,
    this, SLOT(onMouseMove()));

  this->initializeViewInteractions();
  this->RenderWidget = renderWidget;
}

//-----------------------------------------------------------------------------
void vpViewCore::setupTrackFilters(vgMixerWidget* filterWidget)
{
  filterWidget->addItem(vtkVgTrack::Person, "Person");
  filterWidget->setValue(vtkVgTrack::Person, 0.0);

  filterWidget->addItem(vtkVgTrack::Vehicle, "Vehicle");
  filterWidget->setValue(vtkVgTrack::Vehicle, 0.0);

  filterWidget->addItem(vtkVgTrack::Other, "Other");
  filterWidget->setValue(vtkVgTrack::Other, 1.0);

  connect(filterWidget, SIGNAL(stateChanged(int, bool)),
          this, SLOT(SetTrackTypeDisplayState(int, bool)));
  connect(filterWidget, SIGNAL(valueChanged(int, double)),
          this, SLOT(SetTrackTypeProbabilityLimit(int, double)));
}

//-----------------------------------------------------------------------------
void vpViewCore::setupEventFilters(vgMixerWidget* filterWidget,
                                   QComboBox* presetWidget)
{
  filterWidget->clear();
  presetWidget->clear();
  filterWidget->blockSignals(true);
  presetWidget->blockSignals(true);

  for (int i = 0, end = this->EventConfig->GetNumberOfTypes(); i < end; ++i)
    {
    const vgEventType& et = this->EventConfig->GetEventTypeByIndex(i);
    if (et.GetIsUsed())
      {
      filterWidget->addItem(et.GetId(), et.GetName());
      filterWidget->setValue(et.GetId(), 1.0);
      presetWidget->addItem(et.GetName(), QVariant(et.GetId()));
      }
    }

  int k = presetWidget->count();
  presetWidget->insertSeparator(k);
  presetWidget->addItem("All Events", QVariant(-1));
  presetWidget->setCurrentIndex(k + 1);

  filterWidget->blockSignals(false);
  presetWidget->blockSignals(false);

  filterWidget->disconnect(this);

  connect(filterWidget, SIGNAL(stateChanged(int, bool)),
          this, SLOT(SetEventTypeDisplayState(int, bool)));
  connect(filterWidget, SIGNAL(invertedChanged(int,bool)),
          this, SLOT(SetEventInverseDisplayState(int, bool)));
  connect(filterWidget, SIGNAL(valueChanged(int, double)),
          this, SLOT(SetEventTypeNormalcyThreshold(int, double)));
}

//-----------------------------------------------------------------------------
void vpViewCore::setupActivityFilters(vgMixerWidget* filterWidget)
{
  filterWidget->disconnect();
  filterWidget->clear();

  int numTypes = this->ActivityConfig->GetNumberOfTypes();
  std::vector< std::pair<double, double> > ranges(
    numTypes, std::make_pair(0.0, 1.0));

  // Compute min and max saliencies for each type based on the loaded activities.
  for (size_t p = 0, end = this->Projects.size(); p < end; ++p)
    {
    vtkVgActivityManager* am = this->Projects[p]->ActivityManager;
    for (int i = 0, end = am->GetNumberOfActivities();
         i < end; ++i)
      {
      vtkVgActivity* a = am->GetActivity(i);
      double saliency = a->GetSaliency();

      std::pair<double, double>& range = ranges[a->GetType()];
      if (saliency < range.first)
        {
        range.first = saliency;
        }
      else if (saliency > range.second)
        {
        range.second = saliency;
        }
      }
    }

  for (int i = 0; i < numTypes; ++i)
    {
    const vgActivityType& at = this->ActivityConfig->GetActivityType(i);
    if (at.GetIsUsed())
      {
      filterWidget->addItem(i, at.GetName());

      // Round up or down to the next nearest step.
      // Otherwise the slider may not correctly cover the whole range.
      double step = filterWidget->singleStep(i);
      double minVal = ranges[i].first;
      minVal /= step;
      minVal = floor(minVal);
      minVal *= step;

      double maxVal = ranges[i].second;
      maxVal /= step;
      maxVal = ceil(maxVal);
      maxVal *= step;

      filterWidget->setRange(i, minVal, maxVal);
      filterWidget->setValue(i, minVal);

      for (size_t p = 0, end = this->Projects.size(); p < end; ++p)
        {
        this->Projects[p]->ActivityManager->SetSaliencyThreshold(i, minVal);
        }
      }
    }

  connect(filterWidget, SIGNAL(stateChanged(int, bool)),
          this, SLOT(SetActivityTypeDisplayState(int, bool)));
  connect(filterWidget, SIGNAL(valueChanged(int, double)),
          this, SLOT(SetActivityTypeSaliencyThreshold(int, double)));
}

//-----------------------------------------------------------------------------
void vpViewCore::setupNormalcyMapsFilters(vgMixerWidget* filterWidget)
{
  filterWidget->clear();

  QList<QString> keys;
  this->NormalcyMaps->GetAllNormalcyKeys(keys);

  for (int i = 0; i < keys.count(); ++i)
    {
    filterWidget->addItem(i, keys.at(i));
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::setupTrackAttributeColors()
{
  const vgColor one(Qt::green);
  const vgColor two(Qt::red);
  const vgColor three(Qt::blue);
  const vgColor oneAndTwo(0x80, 0x80, 0);

  const QString fgSsd           = "ForegroundTracking/SSD";
  const QString fgCsurf         = "ForegroundTracking/CSURF";
  const QString daKinematic     = "DataAssociationTracking/Kinematic";
  const QString daMultiFeatures = "DataAssociationTracking/MultiFeatures";
  const QString daMultiple      = "DataAssociationTracking/Multiple";
  const QString kfExtended      = "KalmanFilter/Extended";
  const QString kfLinear        = "KalmanFilter/Linear";
  const QString intForward      = "Interval/Forward";
  const QString intBack         = "Interval/Back";
  const QString intInit         = "Interval/Init";
  const QString linkStart       = "Linking/Start";
  const QString linkEnd         = "Linking/End";
  const QString linkMultiple    = "Linking/Multiple";

  QSettings settings;
  settings.beginGroup("TrackAttributeColors");

  // write sensible default color settings if none yet exist
  vgColor::read(settings, fgSsd, one).write(settings, fgSsd);
  vgColor::read(settings, fgCsurf, two).write(settings, fgCsurf);

  vgColor::read(settings, daKinematic, one).write(settings, daKinematic);
  vgColor::read(settings, daMultiFeatures, two).write(settings, daMultiFeatures);
  vgColor::read(settings, daMultiple, oneAndTwo).write(settings, daMultiple);

  vgColor::read(settings, kfExtended, one).write(settings, kfExtended);
  vgColor::read(settings, kfLinear, two).write(settings, kfLinear);

  vgColor::read(settings, intForward, one).write(settings, intForward);
  vgColor::read(settings, intBack, two).write(settings, intBack);
  vgColor::read(settings, intInit, three).write(settings, intInit);

  vgColor::read(settings, linkStart, one).write(settings, linkStart);
  vgColor::read(settings, linkEnd, two).write(settings, linkEnd);
  vgColor::read(settings, linkMultiple, oneAndTwo).write(settings, linkMultiple);
}

//-----------------------------------------------------------------------------
void vpViewCore::setupEventConfig(vpEntityConfigWidget* configWidget)
{
  configWidget->Initialize(this->EventConfig);
  connect(configWidget, SIGNAL(EntityTypeChanged(int)),
          SLOT(eventTypeChanged(int)));
}

//-----------------------------------------------------------------------------
void vpViewCore::setupActivityConfig(vpEntityConfigWidget* configWidget)
{
  configWidget->Initialize(this->ActivityConfig);
  connect(configWidget, SIGNAL(EntityTypeChanged(int)),
          SLOT(activityTypeChanged(int)));
}

//-----------------------------------------------------------------------------
void vpViewCore::onLeftPress()
{
  if (this->RulerEnabled)
    {
    vtkRenderWindowInteractor* interactor = this->RenderWindow->GetInteractor();
    int x = interactor->GetEventPosition()[0];
    int y = interactor->GetEventPosition()[1];
    this->RulerPoints->SetPoint(0, x, y, 0.0);
    this->RulerPoints->SetPoint(1, x, y, 0.0);
    this->RulerPoints->Modified();
    this->RulerActor->SetVisibility(1);
    this->RubberbandInteractorStyle->SetRubberBandModeToDisabled();
    this->render();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::onLeftRelease()
{
  if (this->RulerEnabled)
    {
    if (!(this->UseGeoCoordinates || this->UseRawImageCoordinates ||
          this->ImageToGcsMatrix))
      {
      emit this->geoDistanceMeasured(-1.0);
      this->RulerActor->SetVisibility(0);
      this->render();
      return;
      }

    // Compute the ground distance in meters between the ruler endpoints, after
    // first transforming them from display to image coordinates.
    double p1[4];
    this->SceneRenderer->SetDisplayPoint(this->RulerPoints->GetPoint(0));
    this->SceneRenderer->DisplayToWorld();
    this->SceneRenderer->GetWorldPoint(p1);

    this->WorldToImageMatrix->MultiplyPoint(p1, p1);
    p1[0] /= p1[3];
    p1[1] /= p1[3];

    double p2[4];
    this->SceneRenderer->SetDisplayPoint(this->RulerPoints->GetPoint(1));
    this->SceneRenderer->DisplayToWorld();
    this->SceneRenderer->GetWorldPoint(p2);

    this->WorldToImageMatrix->MultiplyPoint(p2, p2);
    p2[0] /= p2[3];
    p2[1] /= p2[3];

    double dummy;
    double dist = this->getGeoDistance(p1, p2, dummy, dummy);
    emit this->geoDistanceMeasured(dist);

    this->RulerActor->SetVisibility(0);
    this->render();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::onLeftClick()
{
  if (this->TrackEditProjectId == -1)
    {
    this->pickScene();
    return;
    }

  // placing track head
  vtkRenderWindowInteractor* interactor = this->RenderWindow->GetInteractor();

  int x = interactor->GetEventPosition()[0];
  int y = interactor->GetEventPosition()[1];

  double world[4];
  this->SceneRenderer->SetDisplayPoint(x, y, 0.0);
  this->SceneRenderer->DisplayToWorld();
  this->SceneRenderer->GetWorldPoint(world);
  world[2] = 0.0;
  world[3] = 1.0;

  this->WorldToImageMatrix->MultiplyPoint(world, world);
  world[0] /= world[3];
  world[1] /= world[3];

  if (!this->UseGeoCoordinates)
    {
    world[0] = vtkMath::Round(world[0]);
    world[1] = vtkMath::Round(world[1]);
    }

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->DeepCopy(this->TrackHeadIndicatorPolyData->GetPoints());

  // transform indicator points from actor-space to world-space
  for (vtkIdType i = 0, end = points->GetNumberOfPoints(); i != end; ++i)
    {
    double point[4];
    points->GetPoint(i, point);
    point[3] = 1.0;

    point[0] += world[0];
    point[1] += world[1];

    this->ImageToWorldMatrix->MultiplyPoint(point, point);
    point[0] /= point[3];
    point[1] /= point[3];

    points->SetPoint(i, point);
    }

  this->setTrackHeadAndAdvance(points);
}

//-----------------------------------------------------------------------------
void vpViewCore::onRightClick()
{
  if (this->isEditingTrack())
    {
    this->stopEditingTrack();
    return;
    }

  if (this->DrawingContour)
    {
    this->completeFilterRegion();
    return;
    }

  if (this->RightClickToEditEnabled &&
      this->pickScene() == vtkVgPickData::PickedTrack)
    {
    int session = this->SessionView->GetCurrentSession();
    this->beginEditingTrack(this->Projects[session]->Picker->GetPickedId());
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::onMouseMove()
{
  vtkRenderWindowInteractor* interactor = this->RenderWindow->GetInteractor();
  int dx = interactor->GetEventPosition()[0];
  int dy = interactor->GetEventPosition()[1];

  if (this->RulerActor->GetVisibility())
    {
    this->RulerPoints->SetPoint(1, dx, dy, 0.0);
    this->RulerPoints->Modified();
    this->render();
    }
  else if (this->isEditingTrack())
    {
    this->updateTrackHeadIndicator(dx, dy);
    this->render();
    }

  emit this->mouseMoved(dx, dy);
}

//-----------------------------------------------------------------------------
void vpViewCore::onSelectionComplete()
{
  if (!this->isEditingTrack())
    {
    return;
    }

  // this is a ctrl-drag drawing a new bbox
  int x1 = this->RubberbandInteractorStyle->GetStartPosition()[0];
  int y1 = this->RubberbandInteractorStyle->GetStartPosition()[1];

  int x2 = this->RubberbandInteractorStyle->GetEndPosition()[0];
  int y2 = this->RubberbandInteractorStyle->GetEndPosition()[1];

  // make sure box is adequate size
  if (abs((x2 - x1) * (y2 - y1)) < 25)
    {
    return;
    }

  double p1[4];
  this->SceneRenderer->SetDisplayPoint(x1, y1, 0.0);
  this->SceneRenderer->DisplayToWorld();
  this->SceneRenderer->GetWorldPoint(p1);

  double p2[4];
  this->SceneRenderer->SetDisplayPoint(x2, y2, 0.0);
  this->SceneRenderer->DisplayToWorld();
  this->SceneRenderer->GetWorldPoint(p2);

  double ul[2] =
    {
    std::min(p1[0], p2[0]),
    std::max(p1[1], p2[1])
    };

  double lr[2] =
    {
    std::max(p1[0], p2[0]),
    std::min(p1[1], p2[1])
    };

  vtkIdType rect[] = { 0, 1, 2, 3, 0 };

  // update indicator geometry
  vtkCellArray* ca = this->TrackHeadIndicatorPolyData->GetLines();
  ca->Reset();
  ca->InsertNextCell(5, rect);
  ca->Modified();

  double w = lr[0] - ul[0];
  double h = ul[1] - lr[1];

  // indicator point positions are relative to lower right corner
  vtkPoints* indicatorPts = this->TrackHeadIndicatorPolyData->GetPoints();
  indicatorPts->SetNumberOfPoints(4);
  indicatorPts->SetPoint(0,  -w,   h, 0.0);
  indicatorPts->SetPoint(1,  -w, 0.0, 0.0);
  indicatorPts->SetPoint(2, 0.0, 0.0, 0.0);
  indicatorPts->SetPoint(3, 0.0,   h, 0.0);
  indicatorPts->Modified();

  vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
  pts->SetNumberOfPoints(4);
  pts->SetPoint(0, ul[0], ul[1], 0.0);
  pts->SetPoint(1, ul[0], lr[1], 0.0);
  pts->SetPoint(2, lr[0], lr[1], 0.0);
  pts->SetPoint(3, lr[0], ul[1], 0.0);

  // update the track
  this->setTrackHeadAndAdvance(pts);
}

//-----------------------------------------------------------------------------
void vpViewCore::onEditTrackHeadRegion()
{
  vtkPolyData* pd = this->TrackHeadRegion->GetPolyData();
  vtkPoints* pts = pd->GetPoints();
  vtkIdType npts = pts->GetNumberOfPoints();

  for (vtkIdType i = 0; i != npts; ++i)
    {
    double point[3];
    pts->GetPoint(i, point);

    // round each point to nearest pixel
    point[0] = vtkMath::Round(point[0]);
    point[1] = vtkMath::Round(point[1]);
    pts->SetPoint(i, point);
    }

  pts->Modified();
  this->initTrackHeadIndicator(pd);

  this->setTrackHead(pts);
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::showAnnotation(int sessionId, int objectType,
                                vtkIdType objectId)
{
  vpProject* project = this->Projects[sessionId];

  bool show = true;
  switch (objectType)
    {
    default:
      std::cerr << "Unknown object type.\n";
      show = false;
      break;
    case vgObjectTypeDefinitions::Activity:
      {
      vtkVgActivity* activity = project->ActivityManager->GetActivity(objectId);
      this->Annotation->SetAnnotatedActivity(activity);
      break;
      }
    case vgObjectTypeDefinitions::Event:
      {
      vtkVgEvent* event = project->EventModel->GetEvent(objectId);
      this->Annotation->SetAnnotatedEvent(event);
      break;
      }
    case vgObjectTypeDefinitions::Track:
      {
      // don't show annotation on a freshly created track (not yet valid)
      if (objectId == this->NewTrackId)
        {
        show = false;
        break;
        }
      vtkVgTrack* track = project->TrackModel->GetTrack(objectId);
      this->Annotation->SetAnnotatedTrack(track);
      break;
      }
    case vgObjectTypeDefinitions::SceneElement:
      {
      // don't show annotation on a freshly created track (not yet valid)
      if (objectId == this->NewTrackId)
        {
        show = false;
        break;
        }
      vtkVgTrack* track = project->TrackModel->GetTrack(objectId);
      this->Annotation->SetAnnotatedSceneElement(
        track, project->TrackModel->GetSceneElementIdForTrack(objectId));
      }
    }

  if (!this->ShowAnnotations)
    {
    return;
    }

  this->Annotation->SetVisible(show);
  this->Annotation->Update();
  this->render();
}

//-----------------------------------------------------------------------------
void vpViewCore::hideAnnotation()
{
  if (this->Annotation->GetVisible())
    {
    this->Annotation->SetVisible(false);
    this->Annotation->Update();
    this->render();
    }
}

//-----------------------------------------------------------------------------
int vpViewCore::getCreateTrackId(int session)
{
  return this->Projects[session]->NextCreateTrackId;
}

//-----------------------------------------------------------------------------
void vpViewCore::setCreateTrackId(int id, int session)
{
  this->Projects[session]->NextCreateTrackId = id;
}

//-----------------------------------------------------------------------------
void vpViewCore::createEventLegend(vpProject* project)
{
  double color[3];
  int numberOfEventTypes = this->EventTypeRegistry->GetNumberOfTypes();

  this->EventLegend->SetPickable(true);

  int numUsedTypes = 0;
  for (int i = 0; i < numberOfEventTypes; ++i)
    {
    if (this->EventTypeRegistry->GetType(i).GetIsUsed())
      {
      ++numUsedTypes;
      }
    }

  this->EventLegend->SetNumberOfEntries(numUsedTypes);

  for (int i = 0, entry = 0; i < numberOfEventTypes; ++i)
    {
    const vgEventType& type = this->EventTypeRegistry->GetType(i);
    if (!type.GetIsUsed())
      {
      continue;
      }

    vtkSmartPointer<vtkLineSource> ls(vtkSmartPointer<vtkLineSource>::New());
    ls->SetPoint1(-0.5, 0.5, 0.0);
    ls->SetPoint2(0.5, 0.5, 0.0);
    vtkSmartPointer<vtkPolyData> pd = ls->GetOutput();

    type.GetColor(color[0], color[1], color[2]);

    if (project->IconManager->GetIconSheetFileName())
      {
      int icon = type.GetIconIndex();
      if (!project->IconManager->GetIconImage(icon))
        {
        std::cerr << "No valid icon image found." << std::endl;
        this->EventLegend->SetEntry(entry++, pd, type.GetName(), color);
        continue;
        }

      this->EventLegend->SetEntry(entry++, pd,
                                  project->IconManager->GetIconImage(icon),
                                  type.GetName(), color);
      }
    else
      {
      // No icons loaded for project.
      this->EventLegend->SetEntry(entry++, pd, type.GetName(), color);
      }
    }

  this->SceneRenderer->AddActor(this->EventLegend);

  this->EventLegend->SetVisibility(0);
  this->EventLegend->SetUseBackground(1);
  this->EventLegend->SetBackgroundColor(0.1, 0.1, 0.1);
  this->EventLegend->SetBackgroundOpacity(0.7);

  vtkCoordinate* pos1 = this->EventLegend->GetPositionCoordinate();
  vtkCoordinate* pos2 = this->EventLegend->GetPosition2Coordinate();
  pos1->SetCoordinateSystemToDisplay();
  pos2->SetCoordinateSystemToDisplay();
  pos2->SetValue(200.0, 20.0 * (numUsedTypes + 1));

  this->updateEventLegendPosition(this->SceneRenderer->GetSize()[0],
                                  this->SceneRenderer->GetSize()[1]);
}

//-----------------------------------------------------------------------------
void vpViewCore::updateEventLegendPosition(int w, int vtkNotUsed(h))
{
  double x = w - this->EventLegend->GetWidth();
  this->EventLegend->GetPositionCoordinate()->SetValue(x, 8.0);
}

//-----------------------------------------------------------------------------
void vpViewCore::updateEventLegendColors()
{
  for (int i = 0, entry = 0; i < this->EventTypeRegistry->GetNumberOfTypes(); ++i)
    {
    const vgEventType& type = this->EventTypeRegistry->GetType(i);
    if (!type.GetIsUsed())
      {
      continue;
      }

    double color[3];
    type.GetColor(color[0], color[1], color[2]);

    this->EventLegend->SetEntryColor(entry++, color);
    }
}

//-----------------------------------------------------------------------------
double vpViewCore::getCurrentScale(vtkRenderer* renderer) const
{
  // If not a valid renderer return 1.
  if (!renderer)
    {
    return 1.0;
    }

  int* viewPortSize = renderer->GetSize();
  double cameraWidth = renderer->GetActiveCamera()->GetParallelScale() *
                       ((double)viewPortSize[0] / viewPortSize[1]);

  // @NOTE: We are multiply it by 2.0 so that we increase the scale
  // which means that we are requesting even lower level of detail
  // than calculated by the image source.
  const double realTimeTuneParam  = 2.0;

  return ((cameraWidth * realTimeTuneParam) / (double)viewPortSize[0]);
}

//-----------------------------------------------------------------------------
void vpViewCore::focusBounds(double bounds[4])
{
  double width = bounds[1] - bounds[0];
  double height = bounds[3] - bounds[2];

  // pad out the bounds a small amount
  double padScale = 0.1;
  bounds[0] -= width * padScale;
  bounds[1] += width * padScale;
  bounds[2] -= height * padScale;
  bounds[3] += height * padScale;

  double paddedWidth = bounds[1] - bounds[0];
  double paddedHeight = bounds[3] - bounds[2];

  // get the current bounds of the viewport
  double cameraBounds[4];
  vtkVgRendererUtils::GetBounds(this->SceneRenderer, cameraBounds);

  // zoom out if the bounds won't fit in the viewport
  if (paddedWidth > cameraBounds[1] - cameraBounds[0] ||
      paddedHeight > cameraBounds[3] - cameraBounds[2])
    {
    this->doZoomToExtent(bounds);
    }
  else
    {
    // bounds will fit, so just pan the camera to center the extents
    double center[2];
    center[0] = 0.5 * (bounds[0] + bounds[1]);
    center[1] = 0.5 * (bounds[2] + bounds[3]);
    this->moveCameraTo(center);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::focusBounds(double bounds[4], vtkMatrix4x4* xform)
{
  double minPt[] = { bounds[0], bounds[2], 0.0, 1.0 };
  double maxPt[] = { bounds[1], bounds[3], 0.0, 1.0 };

  xform->MultiplyPoint(minPt, minPt);
  xform->MultiplyPoint(maxPt, maxPt);

  minPt[0] /= minPt[3];
  minPt[1] /= minPt[3];

  maxPt[0] /= maxPt[3];
  maxPt[1] /= maxPt[3];

  bounds[0] = std::min(minPt[0], maxPt[0]);
  bounds[1] = std::max(minPt[0], maxPt[0]);
  bounds[2] = std::min(minPt[1], maxPt[1]);
  bounds[3] = std::max(minPt[1], maxPt[1]);

  this->focusBounds(bounds);
}

//-----------------------------------------------------------------------------
void vpViewCore::focusTimeInterval(const vtkVgTimeStamp& start,
                                   const vtkVgTimeStamp& end)
{
  // advance or go back to the closest frame that is in the interval
  if (this->CoreTimeStamp < start)
    {
    this->setCurrentTime(start);
    }
  else if (this->CoreTimeStamp > end)
    {
    this->setCurrentTime(end);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::focusTrack(int sessionId, vtkIdType trackId,
                            vtkVgTimeStamp start, vtkVgTimeStamp end)
{
  vpProject* project = this->Projects[sessionId];
  vtkVgTrack* track = project->TrackModel->GetTrack(trackId);

  double bounds[4];
  track->GetFullBounds(bounds);

  this->focusBounds(bounds,
                    project->TrackRepresentation->GetRepresentationMatrix());

  if (!start.IsValid())
    {
    start = track->GetStartFrame();
    }
  if (!end.IsValid())
    {
    end = track->GetEndFrame();
    }

  this->focusTimeInterval(start, end);
}

//-----------------------------------------------------------------------------
void vpViewCore::focusEvent(int sessionId, vtkIdType eventId)
{
  vpProject* project = this->Projects[sessionId];
  vtkVgEvent* event = project->EventModel->GetEvent(eventId);

  double bounds[4];
  event->GetFullBounds(bounds);

  this->focusBounds(bounds,
                    project->EventRepresentation->GetRepresentationMatrix());

  this->focusTimeInterval(event->GetStartFrame(), event->GetEndFrame());
}

//-----------------------------------------------------------------------------
void vpViewCore::focusActivity(int sessionId, vtkIdType activityId)
{
  vpProject* project = this->Projects[sessionId];
  vtkVgActivity* activity = project->ActivityManager->GetActivity(activityId);

  double bounds[4];

  // compute the bounds of this activity from the component events
  for (unsigned int i = 0; i < activity->GetNumberOfEvents(); ++i)
    {
    vtkVgEvent* event = activity->GetEvent(i);

    double eventBounds[4];
    event->GetFullBounds(eventBounds);

    if (i == 0)
      {
      bounds[0] = eventBounds[0];
      bounds[1] = eventBounds[1];
      bounds[2] = eventBounds[2];
      bounds[3] = eventBounds[3];
      }
    else
      {
      bounds[0] = std::min(bounds[0], eventBounds[0]);
      bounds[1] = std::max(bounds[1], eventBounds[1]);
      bounds[2] = std::min(bounds[2], eventBounds[2]);
      bounds[3] = std::max(bounds[3], eventBounds[3]);
      }
    }

  this->focusBounds(bounds,
                    project->EventRepresentation->GetRepresentationMatrix());

  vtkVgTimeStamp startFrame, endFrame;
  activity->GetActivityFrameExtents(startFrame, endFrame);

  this->focusTimeInterval(startFrame, endFrame);
}

//-----------------------------------------------------------------------------
void vpViewCore::doZoomToExtent(double extents[4])
{
  // Extents and AOI will be updated via vtkVpInteractionCallback
  this->getInteractorStyle()->ZoomToExtents(this->SceneRenderer, extents);
}

//-----------------------------------------------------------------------------
void vpViewCore::getImageSpaceViewportExtents(int (&extents)[4])
{
  double dextents[4];
  vtkVgRendererUtils::GetBounds(this->SceneRenderer, dextents);

  double p1[] = { dextents[0], dextents[2] };
  double p2[] = { dextents[1], dextents[3] };

  int i1[2];
  int i2[2];
  this->worldToImage(p1, i1);
  this->worldToImage(p2, i2);

  extents[0] = i1[0];
  extents[1] = i2[0];
  extents[2] = i2[1];
  extents[3] = i1[1];
}

//-----------------------------------------------------------------------------
void vpViewCore::updateCropExtents(double newExtents[4])
{
  // No need to update crop extents if we never loaded the data.
  if (!this->ImageSource)
    {
    return;
    }

  if (this->UseGeoCoordinates)
    {
    double* out;
    double minPoint[4] = {newExtents[0], newExtents[2], 0, 1};
    double maxPoint[4] = {newExtents[1], newExtents[3], 0, 1};

    out = this->WorldToImageMatrix->MultiplyDoublePoint(minPoint);
    newExtents[0] = out[0] / out[3];
    newExtents[2] = out[1] / out[3];

    out = this->WorldToImageMatrix->MultiplyDoublePoint(maxPoint);
    newExtents[1] = out[0] / out[3];
    newExtents[3] = out[1] / out[3];
    }
  else if (this->UseRawImageCoordinates)
    {
    newExtents[0] += this->ImageTranslationOffset[0];
    newExtents[1] += this->ImageTranslationOffset[0];
    newExtents[2] += this->ImageTranslationOffset[1];
    newExtents[3] += this->ImageTranslationOffset[1];
    }

  bool outOfBounds = false;
  if (newExtents[0] >=  this->WholeImageBounds[1]) { outOfBounds = true; }
  if (newExtents[1] <=  this->WholeImageBounds[0]) { outOfBounds = true; }
  if (newExtents[2] >=  this->WholeImageBounds[3]) { outOfBounds = true; }
  if (newExtents[3] <=  this->WholeImageBounds[2]) { outOfBounds = true; }

  if (outOfBounds)
    {
    std::cerr << "outOfBounds: " << std::endl;
    return;
    }

  int usableExtents[4];
  usableExtents[0] = floor(newExtents[0] >= this->WholeImageBounds[0]
    ? (newExtents[0] - this->WholeImageBounds[0])
    : 0);

  usableExtents[1] = ceil(newExtents[1] >= this->WholeImageBounds[1]
    ? (this->WholeImageBounds[1] - this->WholeImageBounds[0])
    : (newExtents[1] - this->WholeImageBounds[0]));

  // Y
  usableExtents[2] = floor(newExtents[2] >= this->WholeImageBounds[2]
    ? (newExtents[2] - this->WholeImageBounds[2])
    : 0);

  usableExtents[3] = ceil(newExtents[3] >= this->WholeImageBounds[3]
    ? (this->WholeImageBounds[3] - this->WholeImageBounds[2])
    : (newExtents[3] - this->WholeImageBounds[2]));

  this->ImageSource->SetReadExtents(usableExtents);
}

//-----------------------------------------------------------------------------
void vpViewCore::updateAOIImagery()
{
  if (!this->ImageSource)
    {
    return;
    }

  this->ImageSource->SetLevel(-1);

  this->ImageSource->SetScale(this->getCurrentScale(this->SceneRenderer) *
                              this->ImageSourceLODFactor);
  this->ImageSource->Update();

  this->ImageData[1]->ShallowCopy(this->ImageSource->GetOutput());
}

//-----------------------------------------------------------------------------
void vpViewCore::updateViewExtents()
{
  double minPt[] =
    { this->WholeImageBounds[0], this->WholeImageBounds[2], 0.0, 1.0 };

  this->ImageToWorldMatrix->MultiplyPoint(minPt, minPt);
  minPt[0] /= minPt[3];
  minPt[1] /= minPt[3];

  double maxPt[] =
    { this->WholeImageBounds[1], this->WholeImageBounds[3], 0.0, 1.0 };

  this->ImageToWorldMatrix->MultiplyPoint(maxPt, maxPt);
  maxPt[0] /= maxPt[3];
  maxPt[1] /= maxPt[3];

  this->ViewExtents[0] = minPt[0];
  this->ViewExtents[1] = maxPt[0];
  this->ViewExtents[2] = minPt[1];
  this->ViewExtents[3] = maxPt[1];
}

//-----------------------------------------------------------------------------
void vpViewCore::calculateNewExtents(vtkRenderer* ren, double newExtents[4])
{
  if (!newExtents)
    {
    std::cerr << "Invalid input extents." << std::endl;
    return;
    }

  double cameraCurrentPosition[3];
  vtkCamera* camera = ren->GetActiveCamera();
  camera->GetPosition(cameraCurrentPosition);

  double* rendererAspect = ren->GetAspect();

  double renderedBounds[4];
  vtkVgRendererUtils::GetBounds(ren, renderedBounds);

  double newCenter[2] = {cameraCurrentPosition[0], cameraCurrentPosition[1]};
  double extentHalfHeight, extentHalfWidth;
  extentHalfWidth = (renderedBounds[1] - renderedBounds[0]) * 0.5;

  newExtents[0] = /*floor*/(newCenter[0] - extentHalfWidth);
  newExtents[1] = /*ceil*/(newCenter[0] + extentHalfWidth);
  extentHalfHeight =
    0.5 * (newExtents[1] - newExtents[0]) *
    (rendererAspect[1] / rendererAspect[0]);
  newExtents[2] = /*floor*/(newCenter[1] - extentHalfHeight);
  newExtents[3] = /*ceil*/(newCenter[1] + extentHalfHeight);
}

//-----------------------------------------------------------------------------
vpProject* vpViewCore::loadProject(const char* fileName)
{
  if (!fileName)
    {
    return 0;
    }

  // Parse the file and fill in the variables.
  this->ProjectParser->SetFileName(fileName);
  this->ProjectParser->SetUseStream(false);

  // Read data from configuration file now.
  QScopedPointer<vpProject> project(new vpProject(this->CurrentProjectId++));
  this->ProjectParser->Parse(project.data());

#ifdef VISGUI_USE_VIDTK
  QSharedPointer<vpVidtkFileIO> fileIO(new vpVidtkFileIO);
  fileIO->SetTracksFileName(project->TracksFile.c_str());
  fileIO->SetTrackTraitsFileName(project->TrackTraitsFile.c_str());
  fileIO->SetEventsFileName(project->EventsFile.c_str());
  fileIO->SetEventLinksFileName(project->EventLinksFile.c_str());
  fileIO->SetActivitiesFileName(project->ActivitiesFile.c_str());
  fileIO->SetFseTracksFileName(project->SceneElementsFile.c_str());
  project->ModelIO = fileIO;
  return this->processProject(project);
#else
  QMessageBox::warning(0, QString(),
                       "Cannot open project files without VidTK support.");
  return 0;
#endif
}

//-----------------------------------------------------------------------------
vpProject* vpViewCore::loadProject(const QSharedPointer<vpModelIO>& modelIO,
                                   const std::string& stream,
                                   const std::string& name)
{
  this->ProjectParser->SetStream(stream);
  this->ProjectParser->SetUseStream(true);

  // Not really a filename, but this string will be used in the UI as a label
  this->ProjectParser->SetFileName(name);

  QScopedPointer<vpProject> project(new vpProject(this->CurrentProjectId++));
  this->ProjectParser->Parse(project.data());

  project->ModelIO = modelIO;
  return this->processProject(project);
}

//-----------------------------------------------------------------------------
static void emitHomographyCountWarning()
{
  std::cerr << "One or more frames don't have homographies - track "
            << "states falling on those frames may be dropped.\n";
}

//-----------------------------------------------------------------------------
vpProject* vpViewCore::processProject(QScopedPointer<vpProject>& project)
{
  // If this is the first project loaded, set up the imagery data sources.
  // Subsequent projects will share this data.
  if (this->Projects.empty())
    {
    if (project->IsValid(project->OverviewFile) ==
        vpProject::FILE_NOT_EXIST)
      {
      this->handleFileNotFound(project->OverviewFileTag,
                               project->OverviewFile);
      }

    // Allow user to change the data path if not found
    std::string substitutePath;
    size_t pos = project->DataSetSpecifier.find_last_of("/\\");
    if (pos != std::string::npos)
      {
      QString path = qtString(project->DataSetSpecifier.substr(0, pos));
      if (!QDir(path).exists())
        {
        if (QMessageBox::warning(
              0, QString(),
              QString("%1\n\ndoesn't appear to be a valid path. Would you "
                      "like to change the base path for project image data?")
                      .arg(path), QMessageBox::Ok | QMessageBox::Cancel) ==
            QMessageBox::Ok)
          {
          for (;;)
            {
            QString newPath =
              QInputDialog::getText(0, QString(), "Path:", QLineEdit::Normal,
                                    path);

            if (newPath.isEmpty())
              {
              break;
              }

            if (!QDir(newPath).exists())
              {
              QMessageBox::warning(0, QString(), "Path does not exist.");
              }
            else
              {
              substitutePath = stdString(newPath);
              project->DataSetSpecifier.replace(0, pos, substitutePath);
              break;
              }
            }
          }
        }
      }

    bool hasFrameMap = project->ModelIO->ReadFrameMetaData(this->FrameMap,
                                                           substitutePath);
    if (hasFrameMap)
      {
      // Use the image filenames retrieved as the dataset
      this->ImageDataSource->setDataFiles(project->ModelIO->GetImageFiles());

      this->NumberOfFrames =
        static_cast<unsigned int>(this->ImageDataSource->getFileCount());

      // If the expected metadata is not there, fall back to determining the
      // dataset from the project parameters.
      if (this->NumberOfFrames == 0)
        {
        std::cerr << "Expected frame metadata not found.\n";
        hasFrameMap = false;
        }
      else
        {
        if (project->ModelIO->GetHomographyCount() <
            this->ImageDataSource->getFileCount())
          {
          emitHomographyCountWarning();
          }
        this->TrackStorageMode =
          vpTrackIO::TSM_HomographyTransformedImageCoords;
        }
      }

    if (!hasFrameMap)
      {
      this->ImageDataSource->setDataSetSpecifier(project->DataSetSpecifier);

      this->NumberOfFrames =
        static_cast<unsigned int>(this->ImageDataSource->getFileCount());

      if (this->NumberOfFrames == 0)
        {
        this->handleDataSetNotFound(project.data());
        return 0;
        }

      // TODO: Sink all this stuff into model io
      switch (project->IsValid(project->ImageTimeMapFile))
        {
        case vpProject::FILE_NOT_EXIST:
          emit this->warningError("Image time map file not found.");
          break;

        case vpProject::FILE_EXIST:
          {
          // Populate the image->time map from text file if available.
          std::ifstream file(project->ImageTimeMapFile.c_str());
          if (!file.is_open())
            {
            emit this->warningError("Unable to open image time map file.");
            break;
            }

          std::string imageFile;
          double seconds;
          while (file >> imageFile >> seconds)
            {
#ifdef VISGUI_USE_VIDTK
            // Expand any environment variable tokens
            imageFile = vidtk::token_expansion::expand_token(imageFile);
#endif
            this->FrameMap->setImageTime(imageFile, seconds * 1e6);
            }
          hasFrameMap = true;
          }
        }

      switch (project->IsValid(project->HomographyIndexFile))
        {
        case vpProject::FILE_NOT_EXIST:
          emit this->warningError("Homography index file not found.");
          break;

        case vpProject::FILE_EXIST:
          {
          // Populate the image->time map from text file if available.
          std::ifstream file(project->HomographyIndexFile.c_str());
          if (!file.is_open())
            {
            emit this->warningError("Unable to open Homography index file.");
            break;
            }

          int homographyCount = 0;
          bool endOfFile = false;
          int frameIndex;
          double timeStamp, matrixElement;
          vtkSmartPointer<vtkMatrix4x4> homography =
            vtkSmartPointer<vtkMatrix4x4>::New();
          while (file >> frameIndex >> timeStamp)
            {
            for (int i = 0; i < 3 && !endOfFile; ++i)
              {
              for (int j = 0; j < 3 && !endOfFile; ++j)
                {
                if (!(file >> matrixElement))
                  {
                  endOfFile = true;
                  break;
                  }
                homography->SetElement(i < 2 ? i : 3,
                                       j < 2 ? j : 3,
                                       matrixElement);
                }
              }
            // trusting that a negative frameIndex won't occur?
            if (frameIndex >= this->ImageDataSource->getFileCount())
              {
              break;
              }

            this->FrameMap->setImageHomography(
              this->ImageDataSource->getDataFile(frameIndex), homography);
            ++homographyCount;
            }
          if (homographyCount < this->ImageDataSource->getFileCount())
            {
            emitHomographyCountWarning();
            }
          this->TrackStorageMode =
            vpTrackIO::TSM_HomographyTransformedImageCoords;
          }
        }
      }

    // Now first initialize the image source.
    this->initializeImageSource();

    // If the image source is not initialized we cannot proceed.
    if (!this->ImageSource)
      {
      QString errorMsg = QString("Image source could not be initialized.\n"
                                 "Application cannot proceed.");
      emit this->criticalError(errorMsg);
      return 0;
      }

    // Read GCS matrix if any
    // @TODO: Currently we use it only for text display purposes
    if (project->ImageToGcsMatrix)
      {
      this->ImageToGcsMatrix = project->ImageToGcsMatrix;
      }

    this->importOverviewFromFile(project.data());

    // @NOTE: Currently we are assuming that we are having five (5)
    // level of detail in MRJ file. This can change and we need to
    // find a strategy about it.

    // This needs to set regardless.
    this->ImageSource->SetOrigin(project->OverviewOrigin[0],
                                 project->OverviewOrigin[1], 0.0);

    this->ImageSource->SetFileName(
      this->ImageDataSource->getDataFile(
        this->CoreTimeStamp.GetFrameNumber()).c_str());

    this->ImageSource->SetLevel(3);
    this->ImageSource->SetReadExtents(-1, -1, -1, -1);
    this->ImageSource->Update();
    this->ImageData[1]->ShallowCopy(this->ImageSource->GetOutput());
    int dim[2];
    this->ImageSource->GetDimensions(dim);

    this->UsingTimeStampData = false;
    if (this->UseTimeStampDataIfAvailable)
      {
      this->UsingTimeStampData =
        hasFrameMap || this->ImageSource->GetImageTimeStamp().IsValid();
      }

    if (this->UsingTimeStampData)
      {
      // Go ahead and kick off the background thread to decode image timestamps.
      // Even if we are using frame-based indexing right now, this will be nice
      // to have if the user switches to time-based indexing later, or wants
      // to export image timestamps.
      this->startFrameMapRebuild();

      if (this->UseTimeBasedIndexing)
        {
        this->UsingTimeBasedIndexing = true;
        if (this->waitForFrameMapRebuild())
          {
          vpFrame firstFrame;
          this->FrameMap->first(firstFrame);
          this->CoreTimeStamp = firstFrame.Time;
          }
        }
      }

    // FIXME - right now things will break if you mix projects that result
    // in UseGeoCoordinates or UseRawImageCoordinates being set differently.
    if (this->EnableWorldDisplayIfAvailable)
      {
      this->UseGeoCoordinates = this->ImageSource->GetGeoCornerPoints()
        ? true : false;
      if (this->UseGeoCoordinates)
        {
        if (this->TrackStorageMode ==
            vpTrackIO::TSM_HomographyTransformedImageCoords)
          {
          emit this->warningError("Currently don't support world mode in which"
                                  " tracks are transformed by homographies.\n"
                                  "Homographies not used!");
          }

        this->TrackStorageMode = vpTrackIO::TSM_TransformedGeoCoords;
        }
      }
    else if (this->EnableTranslateImage &&
             project->AOIUpperLeftLatLon[0] != 444 &&
             project->AOIUpperLeftLatLon[1] != 444 &&
             this->ImageSource->GetGeoCornerPoints())
      {
      // if both the AOI is specified in lat/lon AND there are lat/lon corner
      // points available (in the future, WAMI, the homography will suffice)
      // then we save track points in raw form and transform the
      // representations.  Ignored if UseGeoCoordinates is true
      this->UseRawImageCoordinates = true;
      if (this->TrackStorageMode ==
          vpTrackIO::TSM_HomographyTransformedImageCoords)
        {
        emit this->warningError("Currently don't support world mode in which"
                                " tracks are transformed by homographies.\n"
                                "Homographies not used!");
        }
      this->TrackStorageMode = vpTrackIO::TSM_ImageCoords;
      }

    if (this->UseGeoCoordinates || this->UseRawImageCoordinates)
      {
      if (vtkSmartPointer<vtkMatrix4x4> imageToLatLon =
          this->ImageSource->GetImageToWorldMatrix())
        {
        vtkMatrix4x4::Invert(imageToLatLon, this->LatLonToImageMatrix);
        }

      // Remember the initial lat-lon-to-image transform, since it can be used
      // to compute offsets between projects with different AOI's, such as when
      // when importing tracks from a different project.
      this->LatLonToImageReferenceMatrix->DeepCopy(this->LatLonToImageMatrix);

      // if we haven't yet set the reference lat/lon, do so now, and compute
      // the reference offset
      if (this->ImageTranslationReferenceLatLon[0] == 444)
        {
        this->ImageTranslationReferenceLatLon[0] =
          project->AOIUpperLeftLatLon[0];
        this->ImageTranslationReferenceLatLon[1] =
          project->AOIUpperLeftLatLon[1];
        double in[4] = {this->ImageTranslationReferenceLatLon[1],
                        this->ImageTranslationReferenceLatLon[0], 0, 1.0};
        double out[4];
        this->LatLonToImageMatrix->MultiplyPoint(in, out);
        if (out[3] != 0)
          {
          this->ImageTranslationReferenceOffset[0] = out[0] / out[3];
          this->ImageTranslationReferenceOffset[1] = out[1] / out[3];
          }
        else
          {
          std::cerr <<
            "Unable to compute reference offset for image translation\n";
          }
        }

      // Define the reference coordinate system using the initial
      // lat-lon-to-image transform. Using lat-lon directly as the coordinate
      // system is prone to loss of precision.
      if (this->UseGeoCoordinates)
        {
        this->LatLonToWorldMatrix->DeepCopy(this->LatLonToImageMatrix);
        }
      else
        {
        this->LatLonToWorldMatrix->Identity();
        }

      this->updateImageMatrices();

      if (this->UseRawImageCoordinates)
        {
        // Compute lat-lon-to-world
        vtkMatrix4x4::Multiply4x4(this->ImageToWorldMatrix,
                                  this->LatLonToImageMatrix,
                                  this->LatLonToWorldMatrix);
        }
      }
    if (this->UseGeoCoordinates)
      {
      this->ImageActor[1]->SetUserMatrix(this->ImageToWorldMatrix);
      this->WholeImageBounds[0] = 0;
      this->WholeImageBounds[1] = dim[0] - 1;
      this->WholeImageBounds[2] = 0;
      this->WholeImageBounds[3] = dim[1] - 1;

      // Set to world bounds
      this->updateViewExtents();
      }
    else
      {
      this->WholeImageBounds[0] = project->OverviewOrigin[0];
      this->WholeImageBounds[1] = this->WholeImageBounds[0] + dim[0] - 1;
      this->WholeImageBounds[2] = project->OverviewOrigin[1];
      this->WholeImageBounds[3] = this->WholeImageBounds[2] + dim[1] - 1;

      int aoiExtents[4] =
        {
        static_cast<int>(-project->OverviewOrigin[0]),
        static_cast<int>(-project->OverviewOrigin[0] + project->AnalysisDimensions[0] - 1),
        static_cast<int>(-project->OverviewOrigin[1]),
        static_cast<int>(-project->OverviewOrigin[1] + project->AnalysisDimensions[1] - 1)
        };

      this->ImageSource->SetReadExtents(aoiExtents);
      this->ImageSource->SetLevel(1);

      this->ViewExtents[0] = this->WholeImageBounds[0];
      this->ViewExtents[1] = this->WholeImageBounds[1];
      this->ViewExtents[2] = this->WholeImageBounds[2];
      this->ViewExtents[3] = this->WholeImageBounds[3];

      if (project->AnalysisDimensions[0] == -1)
        {
        project->AnalysisDimensions[0] =
          this->ViewExtents[1] - this->ViewExtents[0] + 1;
        }
      if (project->AnalysisDimensions[1] == -1)
        {
        project->AnalysisDimensions[1] =
          this->ViewExtents[3] - this->ViewExtents[2] + 1;
        }
      }

    if (!this->UsingTimeStampData)
      {
      this->FrameNumberOffset = project->FrameNumberOffset;
      this->CoreTimeStamp.SetFrameNumber(this->FrameNumberOffset);
      }

    this->VideoAnimation->setFrameRange(
      vtkVgTimeStamp(this->getMinimumTime()),
      vtkVgTimeStamp(this->getMaximumTime()));

    this->ImageSource->Update();
    this->AOIImage->DeepCopy(this->ImageSource->GetOutput());

    // The background image will be rendered before everything else.
    this->SceneRenderer->AddViewProp(this->ImageActor[1]);

    this->ImageActor[1]->GetProperty()->SetColorWindow(project->ColorWindow);
    this->ImageActor[1]->GetProperty()->SetColorLevel(project->ColorLevel);

    this->TrackConfig->MarkAllTypesUnused();
    this->EventConfig->MarkAllTypesUnused();
    this->ActivityConfig->MarkAllTypesUnused();
    }
  else
    {
    if (QDir::cleanPath(qtString(this->ImageDataSource->getDataSetSpecifier()))
        != QDir::cleanPath(qtString(project->DataSetSpecifier)))
      {
      QString str = "Warning: \"%1\" has a dataset specifier that doesn't match"
                    " the currently loaded data.";
      emit warningError(str.arg(QFileInfo(qtString(
        this->ProjectParser->GetProjectFileName())).fileName()));
      }
    if (project->FrameNumberOffset != this->FrameNumberOffset)
      {
      QString str = "Warning: \"%1\" has a frame number offset that doesn't match"
                    " the currently loaded data.";
      emit warningError(str.arg(QFileInfo(qtString(
        this->ProjectParser->GetProjectFileName())).fileName()));
      }
    }

  project->TrackModel->SetContourOperatorManager(this->ContourOperatorManager);
  project->TrackModel->SetTemporalFilters(this->TemporalFilters);
  project->TrackModel->Initialize();

  vtkVgTimeStamp offset = this->ObjectExpirationTime;
  if (!offset.IsValid())
    {
    offset.SetFrameNumber(DefaultTrackExpiration);
    offset.SetTime(1e6 * (DefaultTrackExpiration / this->RequestedFPS));
    }

  project->TrackModel->SetTrackExpirationOffset(offset);
  this->setTrackTrailLength(project.data(), this->TrackTrailLength);
  this->setSceneElementLineWidth(project.data(), this->SceneElementLineWidth);

  project->IconManager->SetRenderer(this->SceneRenderer);
  project->IconManager->SetMinimumIconSize(this->IconSize);
  project->IconManager->SetMaximumIconSize(this->IconSize);
  project->IconManager->SetIconOffset(this->IconOffsetX, this->IconOffsetY);
  for (int i = 0, end = this->EventConfig->GetNumberOfTypes(); i < end; ++i)
    {
    const vgEventType& type = this->EventConfig->GetEventTypeByIndex(i);
    const char* str = vpEventConfig::GetStringFromId(type.GetId());
    project->IconManager->RegisterStaticEventIcon(str, type.GetIconIndex());
    }

  project->TrackRepresentation->UseAutoUpdateOff();
  project->TrackRepresentation->SetTrackModel(project->TrackModel);
  project->TrackRepresentation->SetTrackFilter(this->TrackFilter);
  project->TrackRepresentation->SetVisible(this->ShowTracks ? 1 : 0);
  project->TrackRepresentation->SetColorMultiplier(project->ColorMultiplier);
  project->TrackRepresentation->SetZOffset(0.1);
  project->TrackRepresentation->SetDisplayMask(vtkVgTrack::DF_Normal);

  project->SelectedTrackRepresentation->SetOverrideColor(SelectedColor);
  project->SelectedTrackRepresentation->UseAutoUpdateOff();
  project->SelectedTrackRepresentation->SetTrackModel(project->TrackModel);
  project->SelectedTrackRepresentation->SetTrackFilter(this->TrackFilter);
  project->SelectedTrackRepresentation->SetVisible(this->ShowTracks ? 1 : 0);
  project->SelectedTrackRepresentation->SetDisplayMask(vtkVgTrack::DF_Normal |
                                                       vtkVgTrack::DF_Selected);
  project->SelectedTrackRepresentation->SetZOffset(0.4);

  project->TrackHeadRepresentation->UseAutoUpdateOff();
  project->TrackHeadRepresentation->SetTrackModel(project->TrackModel);
  project->TrackHeadRepresentation->SetTrackFilter(this->TrackFilter);
  project->TrackHeadRepresentation->SetDisplayAllHeads(false);
  project->TrackHeadRepresentation->SetVisible(this->ShowTrackHeads ? 1 : 0);
  project->TrackHeadRepresentation->SetColorMultiplier(project->ColorMultiplier);
  project->TrackHeadRepresentation->SetZOffset(0.1);
  project->TrackHeadRepresentation->SetDisplayMask(vtkVgTrack::DF_Normal);

  project->SelectedTrackHeadRepresentation->SetOverrideColor(SelectedColor);
  project->SelectedTrackHeadRepresentation->UseAutoUpdateOff();
  project->SelectedTrackHeadRepresentation->SetTrackModel(project->TrackModel);
  project->SelectedTrackHeadRepresentation->SetTrackFilter(this->TrackFilter);
  project->SelectedTrackHeadRepresentation->SetDisplayAllHeads(false);
  project->SelectedTrackHeadRepresentation->SetVisible(this->ShowTrackHeads ? 1 : 0);
  project->SelectedTrackHeadRepresentation->SetDisplayMask(vtkVgTrack::DF_Normal |
                                                           vtkVgTrack::DF_Selected);
  project->SelectedTrackHeadRepresentation->SetZOffset(0.4);

  project->EventModel->SetContourOperatorManager(this->ContourOperatorManager);
  project->EventModel->SetTemporalFilters(this->TemporalFilters);
  project->EventModel->SetTrackModel(project->TrackModel);
  project->EventModel->Initialize();

  this->updateEventDisplayEndFrame(project.data());

  project->EventRepresentation->UseAutoUpdateOff();
  project->EventRepresentation->SetEventFilter(this->EventFilter);
  project->EventRepresentation->SetEventModel(project->EventModel);
  project->EventRepresentation->SetDisplayMask(vtkVgEventBase::DF_TrackEvent);
  project->EventRepresentation->SetVisible(this->ShowEvents ? 1 : 0);
  project->EventRepresentation->SetColorMultiplier(project->ColorMultiplier);
  project->EventRepresentation->SetEventTypeRegistry(this->EventTypeRegistry);
  project->EventRepresentation->SetZOffset(0.2);
  project->EventRepresentation->Initialize();

  project->SelectedEventRepresentation->SetOverrideColor(SelectedColor);
  project->SelectedEventRepresentation->UseAutoUpdateOff();
  project->SelectedEventRepresentation->SetEventFilter(this->EventFilter);
  project->SelectedEventRepresentation->SetEventModel(project->EventModel);
  project->SelectedEventRepresentation->SetDisplayMask(vtkVgEventBase::DF_TrackEvent |
                                                       vtkVgEventBase::DF_Selected);
  project->SelectedEventRepresentation->SetVisible(this->ShowEvents ? 1 : 0);
  project->SelectedEventRepresentation->SetEventTypeRegistry(this->EventTypeRegistry);
  project->SelectedEventRepresentation->SetZOffset(0.5);
  project->SelectedEventRepresentation->Initialize();

  project->EventIconRepresentation->UseAutoUpdateOff();
  project->EventIconRepresentation->SetEventFilter(this->EventFilter);
  project->EventIconRepresentation->SetEventModel(project->EventModel);
  project->EventIconRepresentation->SetIconManager(project->IconManager);
  project->EventIconRepresentation->SetVisible(this->ShowEventIcons ? 1 : 0);
  project->EventIconRepresentation->SetEventTypeRegistry(
    this->EventTypeRegistry);

  project->EventRegionRepresentation->UseAutoUpdateOff();
  project->EventRegionRepresentation->SetEventFilter(this->EventFilter);
  project->EventRegionRepresentation->SetEventModel(project->EventModel);
  project->EventRegionRepresentation->SetDisplayMask(vtkVgEventBase::DF_RegionEvent);
  project->EventRegionRepresentation->SetVisible(this->ShowEvents ? 1 : 0);
  project->EventRegionRepresentation->SetColorMultiplier(project->ColorMultiplier);
  project->EventRegionRepresentation->SetEventTypeRegistry(
    this->EventTypeRegistry);
  project->EventRegionRepresentation->SetRegionZOffset(0.3);

  project->ActivityManager->SetOverlayOpacity(this->OverlayOpacity);
  project->ActivityManager->SetColorMultiplier(project->ColorMultiplier);
  project->ActivityManager->Initialize();
  project->ActivityManager->SetRenderer(this->SceneRenderer);
  project->ActivityManager->SetVisibility(this->ShowActivities ? 1 : 0);
  project->ActivityManager->SetIconManager(project->IconManager);
  project->ActivityManager->SetEventModel(project->EventModel);
  project->ActivityManager->SetActivityTypeRegistry(this->ActivityTypeRegistry);
  project->ActivityManager->InitializeTypeInfo();

  project->SceneElementRepresentation->SetFillOpacity(this->SceneElementFillOpacity);
  project->SceneElementRepresentation->UseAutoUpdateOff();
  project->SceneElementRepresentation->SetTrackModel(project->TrackModel);
  project->SceneElementRepresentation->SetDisplayAllHeads(false);
  project->SceneElementRepresentation->SetVisible(this->ShowSceneElements ? 1 : 0);
  project->SceneElementRepresentation->SetColorMultiplier(project->ColorMultiplier);
  project->SceneElementRepresentation->SetZOffset(0.2);
  project->SceneElementRepresentation->SetDisplayMask(vtkVgTrack::DF_SceneElement);

  project->SelectedSceneElementRepresentation->SetOverrideColor(SelectedColor);
  project->SelectedSceneElementRepresentation->UseAutoUpdateOff();
  project->SelectedSceneElementRepresentation->SetTrackModel(project->TrackModel);
  project->SelectedSceneElementRepresentation->SetDisplayAllHeads(false);
  project->SelectedSceneElementRepresentation->SetVisible(this->ShowSceneElements ? 1 : 0);
  project->SelectedSceneElementRepresentation->SetDisplayMask(vtkVgTrack::DF_SceneElement |
                                                              vtkVgTrack::DF_Selected);
  project->SelectedSceneElementRepresentation->SetZOffset(0.4);

  project->Picker->SetIconManager(project->IconManager);
  project->Picker->AddRepresentation(project->EventRegionRepresentation);
  project->Picker->AddRepresentation(project->EventRepresentation);
  project->Picker->AddRepresentation(project->TrackHeadRepresentation);
  project->Picker->AddRepresentation(project->TrackRepresentation);
  project->Picker->AddRepresentation(project->SceneElementRepresentation);
  project->Picker->SetActivityManager(project->ActivityManager);

  // We only support a single AOI at the moment (that of the original project).
  // If the project being loaded is not the first and doesn't have explicit AOI
  // dimensions in the project file, AnalysisDimensions will be -1 here anyways.
  unsigned int imageHeight =
    static_cast<unsigned int>(this->Projects.empty()
                                ? project->AnalysisDimensions[1]
                                : this->Projects[0]->AnalysisDimensions[1]);

  project->ModelIO->SetImageHeight(imageHeight);

  project->ModelIO->SetTrackModel(project->TrackModel, this->TrackStorageMode,
                                  this->UsingTimeStampData ?
                                    vpTrackIO::TTM_TimeAndFrameNumber :
                                    vpTrackIO::TTM_FrameNumberOnly,
                                  this->TrackTypeRegistry,
                                  this->LatLonToWorldMatrix,
                                  this->FrameMap);

  project->ModelIO->SetEventModel(project->EventModel, this->EventTypeRegistry);

  project->ModelIO->SetActivityModel(project->ActivityManager,
                                     this->ActivityConfig);

  if (this->Projects.empty())
    {
    // The graph model will just use the data from the first loaded project.
    // TODO: Add multi-model support to the graph model?
    this->GraphModel->SetTrackModel(project->TrackModel);
    this->GraphModel->SetEventModel(project->EventModel);
    this->GraphModel->SetActivityManager(project->ActivityManager);
    this->GraphModel->SetEventFilter(this->EventFilter);

    this->GraphRepresentation->SetEventTypeRegistry(this->EventTypeRegistry);

    project->Picker->AddRepresentation(this->GraphRepresentation);
    project->Picker->SetImageActor(this->ImageActor[0]);
    }

  if (project->HasTrackColorOverride)
    {
    project->ModelIO->SetTrackOverrideColor(project->TrackColorOverride);
    }

  bool tracksImportSuccessful =
    this->importTracksFromFile(project.data());

  bool eventsImportSuccessful =
    this->importEventsFromFile(project.data(), tracksImportSuccessful);

  this->importSceneElementsFromFile(project.data());

  this->addTracks(project.data());
  this->addTrackHeads(project.data());
  this->addEvents(project.data());

  this->importEventLinksFromFile(project.data(), eventsImportSuccessful);
  this->importActivitiesFromFile(project.data(), eventsImportSuccessful);
  this->importNormalcyMapsFromFile(project.data());

  this->importIconsFromFile(project.data());

  // Set display full volume from the project file.
  if (project->PrecomputeActivity != 0)
    {
    project->ActivityManager->SetShowFullVolume(true);
    project->ActivityManager->UpdateActivityActors(this->CoreTimeStamp);
    }

  // Set next "create" track id to be an even multiple of 1000.
  project->NextCreateTrackId =
    1000 * ((project->TrackModel->GetNextAvailableId() + 999) / 1000);

  vpProject* projPtr = project.take();
  this->Projects.push_back(projPtr);

  if (projPtr->Name.empty())
    {
    QFileInfo fi(qtString(this->ProjectParser->GetProjectFileName()));
    projPtr->Name = stdString(fi.fileName());
    }
  this->SessionView->AddSession(this, projPtr->ActivityManager,
                                projPtr->EventModel, projPtr->TrackModel,
                                this->EventFilter, this->TrackFilter,
                                this->EventTypeRegistry,
                                this->TrackTypeRegistry,
                                projPtr->Name.c_str());



  // Load bundled filters
  if (projPtr->IsValid(projPtr->FiltersFile) == vpProject::FILE_EXIST)
    {
    this->loadFilters(projPtr->FiltersFile.c_str());
    }

  // Prevent the interactor from rendering the window before we have performed
  // the initial update, which will occur at the end of this event loop.
  this->RenderWindow->GetInteractor()->SetEnableRender(false);

  // Make sure the next update computes new transforms for the representations
  this->ForceFullUpdate = true;

  // Notify observers.
  this->dataChanged();

  return this->Projects.back();
}

//-----------------------------------------------------------------------------
void vpViewCore::closeProject(int sessionId)
{
  vpProject* project = this->Projects[sessionId];
  this->removeTracks(project);
  this->removeTrackHeads(project);
  this->removeEvents(project);
  this->SceneRenderer->RemoveViewProp(project->IconManager->GetIconActor());
  this->Projects.erase(this->Projects.begin() + sessionId);
  delete project;
}

//-----------------------------------------------------------------------------
void vpViewCore::loadConfig(const char* fileName)
{
  if (!QFileInfo(fileName).exists())
    {
    qDebug() << "Project file not found:" << fileName;
    return;
    }
  this->TrackConfig->LoadFromFile(fileName);
  this->EventConfig->LoadFromFile(fileName);
  this->ActivityConfig->LoadFromFile(fileName);
}

//-----------------------------------------------------------------------------
void vpViewCore::writeSpatialFilter(
  vtkPoints* contourPoints, vtkPoints* filterPoints, unsigned int frameNumber,
  double time, vtkMatrix4x4* worldtoImageMatrix,
  const std::string& name, std::ostream& out)
{
  bool writeLatLon = this->UseGeoCoordinates || this->UseRawImageCoordinates;
  out << (writeLatLon ? (worldtoImageMatrix ? "SpatialFilterLatLonAndImage\n"
                                            : "SpatialFilterLatLonV2")
                      : "SpatialFilterV2\n");
  out << name << '\n';
  out << contourPoints->GetNumberOfPoints();

  if (writeLatLon)
    {
    vtkSmartPointer<vtkMatrix4x4> worldToLatLon =
      vtkSmartPointer<vtkMatrix4x4>::New();
    vtkMatrix4x4::Invert(this->LatLonToWorldMatrix, worldToLatLon);

    out.precision(15);

    double point[4];
    vtkIdType i, npts;
    // Convert from world coords to lat-long points
    for (i = 0, npts = contourPoints->GetNumberOfPoints(); i < npts; ++i)
      {
      contourPoints->GetPoint(i, point);
      point[2] = 0.0;
      point[3] = 1.0;

      worldToLatLon->MultiplyPoint(point, point);
      point[0] /= point[3];
      point[1] /= point[3];

      out << ' ' << point[1] << ' ' << point[0];
      }
    if (worldtoImageMatrix)
      {
      out << "\n" << npts;
      for (i = 0; i < npts; ++i)
        {
        contourPoints->GetPoint(i, point);
        point[2] = 0.0;
        point[3] = 1.0;

        worldtoImageMatrix->MultiplyPoint(point, point);
        point[0] /= point[3];
        point[1] /= point[3];

        out << ' ' << vtkMath::Round(point[0]) << ' ' << vtkMath::Round(point[1]);
        }
      // write out the matrix, so (if/when) we can edit (upon loading of the filter), we
      // can maintain the correct "image" mapping
      out << "\n";
      for (int i = 0; i < 4; ++i)
        {
        for (int j = 0; j < 4; ++j)
          {
          out << worldtoImageMatrix->GetElement(i, j) << " ";
          }
        }
      }
    }
  else
    {
    // Normal inverted-y coord mode, just unflip the filter points
    double maxY = this->Projects[0]->AnalysisDimensions[1] - 1;
    for (vtkIdType i = 0, npts = filterPoints->GetNumberOfPoints(); i < npts;
         ++i)
      {
      double point[3];
      filterPoints->GetPoint(i, point);
      point[1] = maxY - point[1];
      out << ' ' << vtkMath::Round(point[0]) << ' ' << vtkMath::Round(point[1]);
      }
    }
  // write frame number and time of the frame the filter was started on
  out << "\n" << frameNumber << " " << time;

  out << "\n\n";
}


//-----------------------------------------------------------------------------
void vpViewCore::writeTemporalFilter(int type, double start, double end,
                                     const std::string& name,
                                     std::ostream& out)
{
  out << "TemporalFilter " << (this->getUsingTimeStampData() ? "seconds\n"
                                                             : "frame#\n");
  out << name << "\n";
  out << type << "\n";
  if (this->getUsingTimeStampData())
    {
    out.precision(15);
    // Adjust time to be in seconds (not microseconds)
    if (start != -1)
      {
      start *= 1e-6;
      }
    if (end != -1)
      {
      end *= 1e-6;
      }
    }
  else if (!this->UseZeroBasedFrameNumbers)
    {
    // Adjust frame number by 1 if not zero-based indexing
    if (start != -1)
      {
      start -= 1.0;
      }
    if (end != -1)
      {
      end -= 1.0;
      }
    }

  out << start << " " << end << "\n\n";
}

//-----------------------------------------------------------------------------
static void skipLines(istream& in, int n)
{
  while (n--)
    {
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::loadFilters()
{
  QString path = vgFileDialog::getOpenFileName(
    0, "Load Filters", QString(), "vpView filters (*.txt);;");

  if (!path.isEmpty())
    {
    this->loadFilters(qPrintable(path));
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::loadFilters(const char* fileName)
{
  std::ifstream file(fileName);
  if (!file.is_open())
    {
    emit this->warningError("Unable to open filter file.");
    return;
    }

  vtkSmartPointer<vtkMatrix4x4> worldToImageMatrix =
    vtkSmartPointer<vtkMatrix4x4>::New();
  vtkMatrix4x4* matrixPtr;
  std::string type;
  std::string name;
  int numPoints;
  std::vector<double> points;
  vtkVgTimeStamp timeStamp;
  bool needLatLon = this->UseGeoCoordinates || this->UseRawImageCoordinates;
  bool warn = false;
  bool hasTimeStampInfo = false;
  bool unitsInSeconds = true;
  bool readSpatialFilter;
  while (file >> type)
    {
    if (type == "SpatialFilter" || type == "SpatialFilterV2")
      {
      readSpatialFilter = true;
      hasTimeStampInfo = (type == "SpatialFilterV2");
      if (needLatLon)
        {
        qWarning().nospace() << "Ignoring image coordinate spatial filter "
                             << " in geo coordinate mode\n";
        skipLines(file, (hasTimeStampInfo ? 4 : 3));
        warn = true;
        continue;
        }
      }
    else if (type == "SpatialFilterLatLon" || type == "SpatialFilterLatLonV2")
      {
      readSpatialFilter = true;
      hasTimeStampInfo = (type == "SpatialFilterLatLonV2");
      if (!needLatLon)
        {
        qWarning().nospace() << "Ignoring lat-lon spatial filter "
                             << "in image coordinate mode\n";
        skipLines(file, (hasTimeStampInfo ? 4 : 3));
        warn = true;
        continue;
        }
      }
    else if (type == "SpatialFilterLatLonAndImage")
      {
      readSpatialFilter = true;
      hasTimeStampInfo = true;
      if (!needLatLon)
        {
        qWarning().nospace() << "Filter has Lat/Lon and image coordinates but "
                             << "currently only supports using Lat/Lon\n";
        skipLines(file, 6);
        warn = true;
        continue;
        }
      }
    else if (type == "TemporalFilter")
      {
      readSpatialFilter = false;
      std::string units;
      file >> units;
      unitsInSeconds = (units == "seconds");
      if (this->getUsingTimeStampData() != unitsInSeconds)
        {
        qWarning().nospace() << "Temporal units mismatch; "
                             << "ignoring temporal filter\n";
        skipLines(file, 4);
        warn = true;
        continue;
        }
      }
    else
      {
      qWarning().nospace() << "Ignoring filter of unknown type "
                           << '"' << qtString(type) << '"' << '\n';
      // hmmm... if we don't recognize, how do we know how many lines to skip?
      skipLines(file, 3);
      warn = true;
      continue;
      }

    skipLines(file, 1); // finish the line we were reading
    std::getline(file, name);

    if (readSpatialFilter)
      {
      file >> numPoints;
      points.resize(numPoints * 2);
      for (int i = 0; i < numPoints; ++i)
        {
        double x, y;
        if (file >> x >> y)
          {
          points[i * 2 + 0] = x;
          points[i * 2 + 1] = y;
          }
        else
          {
          emit this->warningError(
            QString("Error occurred while reading points for filter \"%1\"")
            .arg(qtString(name)));
          return;
          }
        }

      matrixPtr = 0;
      if (type == "SpatialFilterLatLonAndImage")
        {
        // next are image coordinates, which we skip
        file >> numPoints;
        for (int i = 0; i < numPoints; ++i)
          {
          double x, y;
          if (!(file >> x >> y))
            {
            emit this->warningError(
              QString("Error occurred while reading points for filter \"%1\"")
              .arg(qtString(name)));
            return;
            }
          }
        // and now the vtkMatrix used to map from world to image coordinates
        matrixPtr = worldToImageMatrix;
        for (int i = 0; i < 4; ++i)
          {
          for (int j = 0; j < 4; ++j)
            {
            double value;
            if (!(file >> value))
              {
              emit this->warningError(
                QString("Error occurred while reading matrix for filter \"%1\"")
                .arg(qtString(name)));
              return;
              }
            matrixPtr->SetElement(i, j, value);
            }
          }
        }

      timeStamp.Reset();
      if (hasTimeStampInfo)
        {
        unsigned int frameNumber;
        double time;
        file >> frameNumber >> time;
        timeStamp.SetFrameNumber(frameNumber);
        timeStamp.SetTime(time);
        }

      this->addFilterRegion(name, points, matrixPtr, timeStamp);
      }
    else
      {
      int temporalFilterType;
      file >> temporalFilterType;
      double start, end;
      file >> start >> end;
      vtkVgTimeStamp startTime, endTime;
      if (this->getUsingTimeStampData())
        {
        // convert time (in seconds) to microseconds, as used in timestamp
        if (start != -1)
          {
          start *= 1e6;
          startTime.SetTime(start);
          }
        if (end != -1)
          {
          end *= 1e6;
          endTime.SetTime(end);
          }
        }
      else
        {
        startTime.SetFrameNumber(start);
        endTime.SetFrameNumber(end);
        if (!this->UseZeroBasedFrameNumbers)
          {
          // if not zer-based, need to offset frame number by 1 for tree view
          start += 1;
          end += 1;
          }
        }
      int id = this->addTemporalFilter(temporalFilterType, startTime, endTime);
      emit this->temporalFilterReady(id, qtString(name), temporalFilterType,
                                     start, end);
      }
    }

  if (warn)
    {
    emit this->warningError(
      "One or more filters were not loaded due to having an unrecognized type "
      "or require vpView to be in a different coordinate / time mode.");
    }

  this->update();
}

//-----------------------------------------------------------------------------
static inline void writePixel(vtkImageData* imageData, int x, int y,
                              unsigned char value)
{
  unsigned char* pixel =
    static_cast<unsigned char*>(imageData->GetScalarPointer(x, y, 0));

  for (int i = 0, nc = imageData->GetNumberOfScalarComponents(); i < nc; ++i)
    {
    pixel[i] = value;
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::exportForWeb(const char* path, int paddingFrames)
{
  QString fullPath = QString("%1/%2").arg(path).arg("context.png");

  vtkSmartPointer<vtkPNGWriter> imageWriter =
    vtkSmartPointer<vtkPNGWriter>::New();

  int currentExtents[4];
  int aoiExtents[4] =
    {
    static_cast<int>(-this->Projects[0]->OverviewOrigin[0]),
    static_cast<int>(-this->Projects[0]->OverviewOrigin[0] +
                     this->Projects[0]->AnalysisDimensions[0] - 1),
    static_cast<int>(-this->Projects[0]->OverviewOrigin[1]),
    static_cast<int>(-this->Projects[0]->OverviewOrigin[1] +
                     this->Projects[0]->AnalysisDimensions[1] - 1)
    };

  this->ImageSource->GetReadExtents(currentExtents);

  int currentLevel = this->ImageSource->GetLevel();
  this->ImageSource->SetReadExtents(aoiExtents);
  this->ImageSource->SetLevel(0);
  this->ImageSource->Update();
  imageWriter->SetInputData(this->ImageSource->GetOutput());
  imageWriter->SetFileName(qPrintable(fullPath));
  imageWriter->Write();
  this->ImageSource->SetLevel(currentLevel);
  this->ImageSource->SetReadExtents(currentExtents);

  std::string fileName = this->ImageDataSource->getDataFile(0);

  vtkSmartPointer<vtkVgBaseImageSource> imageSource;
  imageSource.TakeReference(
    vpImageSourceFactory::GetInstance()->Create(fileName));

  if (!imageSource)
    {
    emit this->warningError("Image source could not be initialized.");
    return;
    }

  vtkSmartPointer<vtkExtractVOI> extractVOI =
    vtkSmartPointer<vtkExtractVOI>::New();

  int session = this->SessionView->GetCurrentSession();
  vpProject* project = this->Projects[session];

  QProgressDialog progress("Generating event clip images...", "Cancel", 1,
                           project->EventModel->GetNumberOfEvents());
  progress.setWindowModality(Qt::ApplicationModal);
  progress.setMinimumDuration(0);
  progress.setAutoClose(false);
  progress.setAutoReset(false);
  progress.setValue(0);

  int count = 0;
  vtkVgEventInfo eventInfo;
  project->EventModel->InitEventTraversal();
  while ((eventInfo = project->EventModel->GetNextEvent()).GetEvent())
    {
    if (progress.wasCanceled())
      {
      break;
      }
    progress.setValue(++count);

    if (!eventInfo.GetDisplayEvent())
      {
      continue;
      }

    vtkVgEvent* event = eventInfo.GetEvent();
    if (event->GetNumberOfTracks() == 0)
      {
      continue; // ignoring node events for now.
      }

    fullPath = QString("%1/event-%2/").arg(path).arg(event->GetId());
    if (!QDir().mkpath(fullPath))
      {
      qDebug() << "Failed to create path" << fullPath;
      continue;
      }

    vtkVgEventTrackInfo* eti =
      vtkVgEventTrackInfo::SafeDownCast(event->GetTrackInfo(0));

    vtkVgTrack* track = eti->GetTrack();
    vtkPoints* points = track->GetPoints();

    // First pass: Compute the largest bbox occurring in the event. If this is
    // larger than our default clip image size, we'll need to downsample the
    // image to make sure the entire object fits in view.
    vtkIdType id;
    vtkVgTimeStamp timeStamp;
    track->InitPathTraversal();

    // Skip to first frame
    while ((id = track->GetNextPathPt(timeStamp)) != -1)
      {
      if (timeStamp >= eti->StartFrame)
        {
        break;
        }
      }

    double maxWidth = 0.0;
    double maxHeight = 0.0;

    // Expand bbox by the track head region
    for (; id != -1; id = track->GetNextPathPt(timeStamp))
      {
      if (timeStamp > eti->EndFrame)
        {
        break;
        }

      auto bbox = track->GetHeadBoundingBox(timeStamp);
      if (bbox.IsValid())
        {
        maxWidth = std::max(maxWidth, bbox.GetLength(0));
        maxHeight = std::max(maxHeight, bbox.GetLength(1));
        }
      }

    // Bbox's are double original size
    maxWidth *= 2.0;
    maxHeight *= 2.0;

    int clipSampleRate = 1;
    int thumbSampleRate = 1;
    int clipImageDim[] =  { 200, 200 };
    int thumbImageDim[] = { 100, 100 };

    // Expand the input dimensions and downsample if 1:1 is not big enough
    if (maxWidth > clipImageDim[0] || maxHeight > clipImageDim[1])
      {
      qDebug() << "Event region dimensions exceed clip image dimensions, "
                  "downsampling will occur";

      int xSampleRate = 1;
      int divX = maxWidth / clipImageDim[0];
      while (++xSampleRate < 4 && (divX >>= 1));

      int ySampleRate = 1;
      int divY = maxHeight / clipImageDim[1];
      while (++ySampleRate < 4 && (divY >>= 1));

      clipSampleRate = std::max(xSampleRate, ySampleRate);
      clipImageDim[0] *= clipSampleRate;
      clipImageDim[1] *= clipSampleRate;
      }

    // Expand the input dimensions and downsample if 1:1 is not big enough
    if (maxWidth > thumbImageDim[0] || maxHeight > thumbImageDim[1])
      {
      int xSampleRate = 1;
      int divX = maxWidth / thumbImageDim[0];
      while (++xSampleRate < 4 && (divX >>= 1));

      int ySampleRate = 1;
      int divY = maxHeight / thumbImageDim[1];
      while (++ySampleRate < 4 && (divY >>= 1));

      thumbSampleRate = std::max(xSampleRate, ySampleRate);
      thumbImageDim[0] *= thumbSampleRate;
      thumbImageDim[1] *= thumbSampleRate;
      }

    // Second pass: Iterate over the track points again, this time writing a
    // clip image for each point.
    track->InitPathTraversal();

    vtkVgTimeStamp videoStartFrame, videoEndFrame;

    videoStartFrame.SetFrameNumber(
      std::max<int>(eti->StartFrame.GetFrameNumber() - paddingFrames,
                    this->FrameNumberOffset));

    videoEndFrame.SetFrameNumber(
      std::min<int>(eti->EndFrame.GetFrameNumber() + paddingFrames,
                    this->FrameNumberOffset + this->NumberOfFrames - 1));

    // Position track iterator at first frame of event
    vtkVgTimeStamp trackTimeStamp;
    while ((id = track->GetNextPathPt(trackTimeStamp)) != -1)
      {
      if (trackTimeStamp >= eti->StartFrame)
        {
        break;
        }
      }

    bool madeThumbnail = false;
    timeStamp = videoStartFrame;
    for (int frame = 0; timeStamp <= videoEndFrame;
         timeStamp.SetFrameNumber(timeStamp.GetFrameNumber() + 1), ++frame)
      {
      // Advance the track iterator
      while (trackTimeStamp < timeStamp &&
             trackTimeStamp < eti->EndFrame)
        {
        vtkIdType newId = track->GetNextPathPt(trackTimeStamp);
        if (newId == -1)
          {
          break;
          }
        id = newId;
        }

      std::string fileName =
        this->ImageDataSource->getDataFile(timeStamp.GetFrameNumber() -
                                           this->FrameNumberOffset);
      if (fileName.empty())
        {
        qDebug() << "Image for frame"
                 << timeStamp.GetFrameNumber() - this->FrameNumberOffset
                 << "not found, skipping";
        continue;
        }

again:
      int dim[2];
      int sampleRate;
      if (!madeThumbnail &&
          timeStamp.GetFrameNumber() == trackTimeStamp.GetFrameNumber())
        {
        dim[0] = thumbImageDim[0];
        dim[1] = thumbImageDim[1];
        sampleRate = thumbSampleRate;
        }
      else
        {
        dim[0] = clipImageDim[0];
        dim[1] = clipImageDim[1];
        sampleRate = clipSampleRate;
        }

      double point[3];
      points->GetPoint(id, point);

      // TODO: Transform world-based points
      point[0] -= project->OverviewOrigin[0];
      point[1] -= project->OverviewOrigin[1];

      // compute the cropped region of the image
      int extents[6];
      extents[0] = std::max(qRound(point[0] - dim[0] / 2), 0);
      extents[2] = std::max(qRound(point[1] - dim[1] / 2), 0);
      extents[1] = extents[0] + dim[0] - 1;
      extents[3] = extents[2] + dim[1] - 1;
      extents[4] = extents[5] = 0;

      imageSource->SetFileName(fileName.c_str());
      imageSource->UpdateInformation();

      int imageDimensions[2];
      this->ImageSource->GetDimensions(imageDimensions);

      // shift extents if part of the crop region falls outside the image
      if (extents[1] >= imageDimensions[0])
        {
        int shift = extents[1] - (imageDimensions[0] - 1);
        extents[0] -= shift;
        extents[1] -= shift;
        }
      if (extents[3] >= imageDimensions[1])
        {
        int shift = extents[3] - (imageDimensions[1] - 1);
        extents[2] -= shift;
        extents[3] -= shift;
        }

      imageSource->SetReadExtents(extents);
      imageSource->SetLevel(0);
      imageSource->Update();

      // crop to the area of interest / downsample to output dimensions
      extractVOI->SetInputConnection(imageSource->GetOutputPort());
      extractVOI->SetVOI(extents);
      extractVOI->SetSampleRate(sampleRate, sampleRate, 1);
      extractVOI->UpdateWholeExtent();

      vtkImageData* image = extractVOI->GetOutput();

      // draw the track head bounding box on top of the image
      if (timeStamp.GetFrameNumber() == trackTimeStamp.GetFrameNumber())
        {
        vtkIdType npts, *ptIds, ptId;
        track->GetHeadIdentifier(trackTimeStamp, npts, ptIds, ptId);
        if (sampleRate == 1 && npts > 1)
          {
          vtkBoundingBox bbox;
          for (int i = 0; i < npts - 1; ++i)
            {
            double point[3];
            points->GetPoint(ptIds[i], point);
            point[0] -= project->OverviewOrigin[0];
            point[1] -= project->OverviewOrigin[1];
            bbox.AddPoint(point);
            }

          // draw bbox at 2x scale
          double center[3], minPt[3], maxPt[3];
          bbox.GetCenter(center);
          bbox.GetMinPoint(minPt[0], minPt[1], minPt[2]);
          bbox.GetMaxPoint(maxPt[0], maxPt[1], maxPt[2]);
          minPt[0] -= center[0];
          minPt[1] -= center[1];
          maxPt[0] -= center[0];
          maxPt[1] -= center[1];
          minPt[0] *= 2.0;
          minPt[1] *= 2.0;
          maxPt[0] *= 2.0;
          maxPt[1] *= 2.0;
          minPt[0] += center[0];
          minPt[1] += center[1];
          maxPt[0] += center[0];
          maxPt[1] += center[1];
          bbox.SetMinPoint(minPt);
          bbox.SetMaxPoint(maxPt);

          int p0[] = { qRound(bbox.GetMinPoint()[0]),
                       qRound(bbox.GetMinPoint()[1]) };
          int p1[] = { qRound(bbox.GetMaxPoint()[0]),
                       qRound(bbox.GetMaxPoint()[1]) };

          p0[0] = qBound(extents[0], p0[0], extents[1]);
          p0[1] = qBound(extents[2], p0[1], extents[3]);
          p1[0] = qBound(extents[0], p1[0], extents[1]);
          p1[1] = qBound(extents[2], p1[1], extents[3]);

          for (int i = p0[0]; i <= p1[0]; ++i)
            {
            writePixel(image, i, p0[1], 255);
            writePixel(image, i, p1[1], 255);
            }

          for (int i = p0[1] + 1; i < p1[1]; ++i)
            {
            writePixel(image, p0[0], i, 255);
            writePixel(image, p1[0], i, 255);
            }
          }
        }

      QString file = madeThumbnail ||
                     trackTimeStamp.GetFrameNumber() != timeStamp.GetFrameNumber()
                       ? QString("%1.png").arg(frame, 6, 10, QChar('0'))
                       : QString("event-%1.png").arg(event->GetId());
      imageWriter->SetInputData(image);
      imageWriter->SetFileName(qPrintable(fullPath + file));
      imageWriter->Write();

      // if we just generated a thumbnail, go back and write the real frame now
      if (!madeThumbnail &&
          trackTimeStamp.GetFrameNumber() == timeStamp.GetFrameNumber())
        {
        madeThumbnail = true;
        goto again;
        }
      }
    }
}

//-----------------------------------------------------------------------------
vtkRenderer* vpViewCore::getSceneRenderer()
{
  return this->SceneRenderer;
}

//-----------------------------------------------------------------------------
vtkVgInteractorStyleRubberBand2D* vpViewCore::getInteractorStyle()
{
  return this->RubberbandInteractorStyle;
}

//-----------------------------------------------------------------------------
void vpViewCore::moveCameraTo(double pos[2], bool forceRender/*=true*/)
{
  vtkCamera* camera = this->getSceneRenderer()->GetActiveCamera();

  double lastFocalPt[3], lastPos[3];
  camera->GetFocalPoint(lastFocalPt);
  camera->GetPosition(lastPos);

  camera->SetFocalPoint(pos[0], pos[1], lastFocalPt[2]);
  camera->SetPosition(pos[0], pos[1], lastPos[2]);

  this->updateExtents();

  if (forceRender)
    {
    this->render();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::render(bool logTime)
{
  if (!this->SceneInitialized)
    {
    return;
    }
  if (!this->RenderPending)
    {
    this->RenderPending = true;
    this->LogRenderTime = logTime;
    QMetaObject::invokeMethod(this, "forceRender", Qt::QueuedConnection);
    }
  else if (logTime)
    {
    this->LogRenderTime = true;
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::forceRender()
{
  this->RenderPending = false;

  this->RenderTimeLogger->StartTimer();
  this->RenderWindow->Render();
  this->RenderTimeLogger->StopTimer();

  if (this->LogRenderTime)
    {
    this->TotalRenderTime += this->RenderTimeLogger->GetElapsedTime();
    this->TotalNumberOfRenderCalls += 1.0;

    // Reset if exceed the limit.
    if (this->TotalRenderTime >= VTK_DOUBLE_MAX ||
        this->TotalNumberOfRenderCalls >= VTK_DOUBLE_MAX)
      {
      this->TotalRenderTime = this->RenderTimeLogger->GetElapsedTime();
      this->TotalNumberOfRenderCalls = 1.0;
      }
    }

  if (this->SaveRenderedImages)
    {
    this->WindowToImageFilter->Modified();
    QChar zero('0');
    QString outputFileName =
      this->ImageOutputDirectory +
        QString("/vpViewImage%1.png").arg(this->ImageCounter++, 6, 10, zero);
    this->PNGWriter->SetFileName(outputFileName.toAscii());
    this->PNGWriter->Write();
    }

  emit this->frameRendered();
}

//-----------------------------------------------------------------------------
void vpViewCore::reactToDataChanged()
{
  this->NumberOfFrames =
    static_cast<unsigned int>(this->ImageDataSource->getFileCount());

  if (this->UsingTimeStampData)
    {
    this->startFrameMapRebuild();

    if (this->UsingTimeBasedIndexing)
      {
      this->waitForFrameMapRebuild();
      }
    }

  this->VideoAnimation->setFrameRange(
    vtkVgTimeStamp(this->getMinimumTime()),
    vtkVgTimeStamp(this->getMaximumTime()));

  double currentTime = this->VideoAnimation->currentTime();

  // Resume the video animation if we were waiting on additional stream frames.
  if (this->Playing)
    {
    this->VideoAnimation->setState(QAbstractAnimation::Running);
    }
  else
    {
    // Try to set the state to paused. If the video was previously stopped
    // because the last frame was reached, but we now are no longer at the
    // end due to receiving more frames, we no longer want to be stopped.
    this->VideoAnimation->setState(QAbstractAnimation::Paused);
    }

  // Playing from a stopped state causes the position to be reset, so un-reset
  this->VideoAnimation->setCurrentTime(currentTime);

  emit this->dataSetChanged();
}

//-----------------------------------------------------------------------------
void vpViewCore::refreshSelectionPanel()
{
  if (this->SessionView)
    {
    this->SessionView->Update();
    }
  emit this->objectInfoUpdateNeeded();
}

//-----------------------------------------------------------------------------
void vpViewCore::setOverviewDisplay(vpProject* project)
{
  // Only add the overview when the background imagery exist.
  if (project->IsValid(project->OverviewFile) ==
      vpProject::FILE_EXIST)
    {
    this->ContextRenderer->AddViewProp(this->ImageActor[0]);
    this->ContextRenderer->ResetCamera();

    // Set the camera so that the rendering fits the image right on.
    vtkCamera* camera = this->ContextRenderer->GetActiveCamera();
    camera->SetParallelScale((this->OverviewExtents[3] - this->OverviewExtents[2]) / 2.0);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::resetToAOIView()
{
  // If we only have the upper left of the AOI, just move the camera so that
  // the upper left of the AOI is at the upper-mid-left of the camera.
  vtkPoints* points = this->AOIOutlinePolyData->GetPoints();
  double point1[3], point3[3];
  points->GetPoint(1, point1);
  points->GetPoint(3, point3);
  if (point1[0] == point3[0] &&
      point1[1] == point3[1])
    {
    double bounds[4];
    vtkVgRendererUtils::GetBounds(this->SceneRenderer, bounds);
    double point[] = { point3[0] + 0.25 * (bounds[1] - bounds[0]),
                       point3[1] + 0.25 * (bounds[2] - bounds[3]) };
    this->moveCameraTo(point);
    return;
    }

  double extents[4];
  this->getAOIExtents(extents);
  this->doZoomToExtent(extents);
}

//-----------------------------------------------------------------------------
void vpViewCore::resetView()
{
  if (this->UseGeoCoordinates || this->UseRawImageCoordinates)
    {
    this->updateViewExtents();
    }

  this->doZoomToExtent(this->ViewExtents);
  this->SceneInitialized = true;
}

//-----------------------------------------------------------------------------
void vpViewCore::resetToViewExtents()
{
  if (this->UseGeoCoordinates || this->UseRawImageCoordinates)
    {
    this->updateViewExtents();
    }

  this->doZoomToExtent(this->ViewExtents);
}

//-----------------------------------------------------------------------------
void vpViewCore::getAOIExtents(double extents[4])
{
  double bounds[6];
  this->AOIOutlinePolyData->GetBounds(bounds);
  extents[0] = bounds[0];
  extents[1] = bounds[1];
  extents[2] = bounds[2];
  extents[3] = bounds[3];
}

//-----------------------------------------------------------------------------
void vpViewCore::decreaseTrackHeadSize()
{
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    vtkPropCollection* props =
      this->Projects[i]->TrackHeadRepresentation->GetActiveRenderObjects();

    props->InitTraversal();
    while (vtkProp* prop = props->GetNextProp())
      {
      vtkActor* actor = vtkActor::SafeDownCast(prop);
      float pointSize = actor->GetProperty()->GetPointSize();
      if (pointSize >= 2.0)
        {
        actor->GetProperty()->SetPointSize(pointSize - 1);
        }
      }
    }

  this->render();
}

//-----------------------------------------------------------------------------
void vpViewCore::increaseTrackHeadSize()
{
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    vtkPropCollection* props =
      this->Projects[i]->TrackHeadRepresentation->GetActiveRenderObjects();

    props->InitTraversal();
    while (vtkProp* prop = props->GetNextProp())
      {
      vtkActor* actor = vtkActor::SafeDownCast(prop);
      actor->GetProperty()->SetPointSize(actor->GetProperty()->GetPointSize() + 1);
      }
    }

  this->render();
}

//-----------------------------------------------------------------------------
void vpViewCore::exitAdjudicationMode()
{
  this->AdjudicationMode = false;

  this->Projects[0]->TrackModel->SetDisplayAllTracks(
    this->AdjudicationAllTracksState);

  for (int i = 0, end = this->EventConfig->GetNumberOfTypes(); i < end; ++i)
    {
    const vgEventType& et = this->EventConfig->GetEventTypeByIndex(i);
    this->EventFilter->SetShowType(et.GetId(), this->AdjudicationEventsState[i]);
    }

  this->Projects[0]->ActivityManager->UpdateActivityDisplayStates();
  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::setDisplayAOIOutlineState(bool flag /*=true*/)
{
  this->AOIOutlineActor->SetVisibility(flag ? 1 : 0);
  this->render();
}

//-----------------------------------------------------------------------------
int vpViewCore::pickScene()
{
  if (this->AdjudicationMode)
    {
    this->Projects[0]->ActivityManager->TurnOffAllActivities();
    this->Projects[0]->ActivityManager->SetActivityState(
      this->Projects[0]->ActivityManager->GetNextAdjudicationActivity(), true);
    this->update();
    return -1;
    }

  vtkRenderWindowInteractor* interactor = this->RenderWindow->GetInteractor();
  int x = interactor->GetEventPosition()[0];
  int y = interactor->GetEventPosition()[1];

  // Projects appearing earlier in the list will be picked against first.
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    int session = static_cast<int>(i);
    vpProject* project = this->Projects[i];
    switch (project->Picker->Pick(x, y, 0.0, this->SceneRenderer))
      {
      case vtkVgPickData::PickedTrack:
        {
        vtkIdType id = project->Picker->GetPickedId();
        vgObjectTypeDefinitions::enumObjectTypes objType =
          project->TrackModel->GetTrack(id)->GetDisplayFlags() &
          vtkVgTrack::DF_SceneElement ? vgObjectTypeDefinitions::SceneElement
                                      : vgObjectTypeDefinitions::Track;
        this->SessionView->SetCurrentSession(session);
        this->SessionView->SelectItem(objType, id);
        emit this->trackPicked(project->Picker->GetPickedId(), session);
        return vtkVgPickData::PickedTrack;
        }
      case vtkVgPickData::PickedEvent:
        {
        this->SessionView->SetCurrentSession(session);
        this->SessionView->SelectItem(vgObjectTypeDefinitions::Event,
                                      project->Picker->GetPickedId());
        return vtkVgPickData::PickedEvent;
        }
      case vtkVgPickData::PickedActivity:
        {
        this->SessionView->SetCurrentSession(session);
        this->SessionView->SelectItem(vgObjectTypeDefinitions::Activity,
                                      project->Picker->GetActivityIndex());
        return vtkVgPickData::PickedActivity;
        }
      }
    }

  return -1;
}

//-----------------------------------------------------------------------------
void vpViewCore::
CreateInformaticsDisplay(int vtkNotUsed(x), int vtkNotUsed(y), vtkImageData* vtkNotUsed(normalcy))
{
  if (!this->Projects[0]->IsValid(this->Projects[0]->InformaticsIconFile))
    {
    std::cerr << "ERROR: File ( " << this->Projects[0]->InformaticsIconFile
              << " ) does not exist or is invalid." << std::endl;
    return;
    }

  this->InformaticsDialog->SetIconsFile(this->Projects[0]->InformaticsIconFile);
  this->InformaticsDialog->Initialize();
  this->InformaticsDialog->exec();
}

//-----------------------------------------------------------------------------
void vpViewCore::
CreateBlendedImageDisplay(int vtkNotUsed(x), int vtkNotUsed(y), vtkImageData* normalcy)
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetNumberOfColors(3);
  lut->SetTableRange(0, 1);
  lut->SetHueRange(0.333, 0.333);
  lut->SetSaturationRange(1, 1);
  lut->SetValueRange(1, 1);
  lut->SetAlphaRange(0.0, 0.8);
  lut->Build();
  vtkSmartPointer<vtkImageMapToColors> lutMapper =
    vtkSmartPointer<vtkImageMapToColors>::New();
  lutMapper->SetLookupTable(lut);
  lutMapper->SetInputData(normalcy);

  vtkSmartPointer<vtkTexture> nTexture = vtkSmartPointer<vtkTexture>::New();
  nTexture->SetInputConnection(lutMapper->GetOutputPort());

  vtkSmartPointer<vtkPlaneSource> nPlane = vtkSmartPointer<vtkPlaneSource>::New();
  nPlane->SetXResolution(100);
  nPlane->SetYResolution(100);
  nPlane->Push(0.001);
  nPlane->Update();

  vtkSmartPointer<vtkPolyDataMapper> nMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  nMapper->SetInputConnection(nPlane->GetOutputPort());
  nMapper->ScalarVisibilityOff();

  vtkSmartPointer<vtkActor> nActor = vtkSmartPointer<vtkActor>::New();
  nActor->SetTexture(nTexture);
  nActor->SetMapper(nMapper);
  nActor->GetProperty()->SetOpacity(0.33);

  // Set up the image plane
  vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();
  texture->SetInputData(this->ImageActor[1]->GetInput());

  vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
  plane->SetXResolution(100);
  plane->SetYResolution(100);
  plane->Update();

  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(plane->GetOutputPort());

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetTexture(texture);
  actor->SetMapper(mapper);
}

//-----------------------------------------------------------------------------
void vpViewCore::onPlay()
{
  this->Playing = true;
  this->VideoAnimation->setState(QAbstractAnimation::Running);
  emit this->playbackStateChanged();
}

//-----------------------------------------------------------------------------
void vpViewCore::onPause()
{
  this->Playing = false;
  if (this->VideoAnimation->state() != QAbstractAnimation::Stopped)
    {
    this->VideoAnimation->setState(QAbstractAnimation::Paused);
    }
  emit this->playbackStateChanged();
}

//-----------------------------------------------------------------------------
void vpViewCore::setLoop(bool state)
{
  this->Loop = state;
}

//-----------------------------------------------------------------------------
void vpViewCore::onResize(int width, int height)
{
  this->updateEventLegendPosition(width, height);
  this->updateExtents();
  this->render(false);
}

//-----------------------------------------------------------------------------
void vpViewCore::setRequestFPS(double fps)
{
  this->RequestedFPS = fps;
  this->VideoAnimation->setFrameInterval(1.0 / this->RequestedFPS);

  if (!this->Projects.empty() && !this->UsingTimeBasedIndexing)
    {
    this->syncAnimationToCore();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::setIconSize(int size, bool render)
{
  if (size == this->IconSize)
    {
    return;
    }
  this->IconSize = size;
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->IconManager->SetMinimumIconSize(size);
    this->Projects[i]->IconManager->SetMaximumIconSize(size);
    this->Projects[i]->IconManager->UpdateIconActors(
      this->CoreTimeStamp.GetFrameNumber());
    }

  if (render)
    {
    this->render();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::setSessionView(vpSessionView* view)
{
  this->SessionView = view;
}

//-----------------------------------------------------------------------------
void vpViewCore::onShowObjectInfo(int sessionId, vpObjectInfoPanel* objectInfo)
{
  vpProject* project = this->Projects[sessionId];
  objectInfo->Initialize(this,
                         project->ActivityManager,
                         project->EventModel,
                         project->TrackModel,
                         this->EventTypeRegistry,
                         this->TrackConfig,
                         project->ModelIO->GetTrackIO());
}

//-----------------------------------------------------------------------------
void vpViewCore::showHideEventLegend(bool show, bool render)
{
  this->EventLegend->SetVisibility(show ? 1 : 0);
  this->EventLegend->Modified();

  if (render)
    {
    this->render();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::showHideTimelineDialog(vpTimelineDialog* dialog, bool show)
{
  if (show)
    {
    dialog->SetViewCoreInstance(this);
    dialog->SetEventModel(this->Projects[0]->EventModel);
    dialog->SetEventTypeRegistry(this->EventTypeRegistry);
    dialog->show();
    }
  else
    {
    dialog->hide();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::showHideObjectTags(bool show)
{
  this->ShowAnnotations = show;
  this->Annotation->SetVisible(show);
  this->Annotation->Update();
  this->render();
}

//-----------------------------------------------------------------------------
void vpViewCore::nextFrame()
{
  this->seekInternal(vtkVgTimeStamp(this->getCurrentTime()),
                                    vg::SeekNext);
}

//-----------------------------------------------------------------------------
void vpViewCore::prevFrame()
{
  this->seekInternal(vtkVgTimeStamp(this->getCurrentTime()),
                                    vg::SeekPrevious);
}

//-----------------------------------------------------------------------------
void vpViewCore::seekInternal(const vtkVgTimeStamp& position,
                              vg::SeekMode direction)
{
  this->VideoAnimation->seek(position, direction);
  if (this->Playing &&
      this->VideoAnimation->state() != QAbstractAnimation::Running)
    {
    // Try to unpause the animation if the UI is still in play mode and the
    // animation is no longer at the end.
    if (this->VideoAnimation->currentTime() <
        this->VideoAnimation->totalDuration())
      {
      this->VideoAnimation->setState(QAbstractAnimation::Running);
      }
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::seekToFrame(const vtkVgTimeStamp& position,
                             vg::SeekMode direction)
{
  if (!position.HasTime())
    {
    return;
    }

  if (this->NumberOfFrames == 0)
    {
    return;
    }

  if (this->UsingTimeBasedIndexing)
    {
    if (this->FrameMap->isEmpty())
      {
      return;
      }

    vpFrame frame;
    if (this->FrameMap->find(position, frame, direction))
      {
      // If we are playing (hence the seek mode is nearest frame), we want to
      // set the current time using the seek position so that the frame scrubber
      // can update continuously, not just when the displayed frame changes.
      this->setCurrentFrame(frame,
                            direction == vg::SeekNearest ? position.GetTime()
                                                         : -1);
      }
    else if (this->Loop)
      {
      // Handle loop-around
      if (direction == vg::SeekNext)
        {
        this->FrameMap->first(frame);
        this->setCurrentFrame(frame, -1.0);
        }
      else if (direction == vg::SeekPrevious)
        {
        this->FrameMap->last(frame);
        this->setCurrentFrame(frame, -1.0);
        }
      }

    return;
    }

  double baseTime = position.GetTime() - this->getMinimumTime();
  int frame = vtkMath::Round((baseTime / 1e6) * this->RequestedFPS);

  // Handle loop-around correctly in the case of "step" seeks. Looping during
  // playing will be handled internally by the video animation.
  int lastFrame = this->NumberOfFrames - 1;
  double currentTime = position.GetTime();
  switch (direction)
    {
    case vg::SeekNext:
      if (frame++ >= lastFrame)
        {
        if (this->Loop)
          {
          frame = 0;
          }
        else
          {
          frame = lastFrame;
          }
        }
      currentTime = -1.0;
      break;

    case vg::SeekPrevious:
      if (frame-- <= 0)
        {
        if (this->Loop)
          {
          frame = lastFrame;
          }
        else
          {
          frame = 0;
          }
        }
      currentTime = -1.0;
      break;

    default:
      break;
    }

  this->setCurrentFrame(frame, currentTime);
}

//-----------------------------------------------------------------------------
void vpViewCore::setPlaybackRate(double rate)
{
  this->VideoAnimation->setRate(rate);
}

//-----------------------------------------------------------------------------
double vpViewCore::getPlaybackRate()
{
  return this->VideoAnimation->rate();
}

//-----------------------------------------------------------------------------
void vpViewCore::update()
{
  this->UpdateObjectViews = true;
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::updateScene()
{
  if (!this->UpdatePending)
    {
    this->UpdatePending = true;
    QMetaObject::invokeMethod(this, "forceUpdate", Qt::QueuedConnection);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::updateExtents()
{
  double newExtents[4];
  this->calculateNewExtents(this->SceneRenderer, newExtents);
  this->updateCropExtents(newExtents);

  if (this->isEditingTrack())
    {
    int x, y;
    this->getMousePosition(&x, &y);
    this->updateTrackHeadIndicator(x, y);
    }

  if (this->SceneInitialized)
    {
    this->updateAOIImagery();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::updateRepresentation(vtkVgRepresentationBase* representation,
                                      vtkObject* filter,
                                      unsigned long prevModelUpdateTime,
                                      unsigned long modelUpdateTime)
{
  unsigned long repUpdateTime = representation->GetUpdateTime();

  // Only update the representation when needed.
  if (modelUpdateTime != prevModelUpdateTime ||
      (filter && filter->GetMTime() > repUpdateTime) ||
      repUpdateTime < representation->GetMTime() ||
      repUpdateTime < modelUpdateTime)
    {
    representation->Update();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::forceUpdate()
{
  this->UpdatePending = false;

  // Enable interactor rendering now that we know the scene has been
  // fully initialized.
  this->RenderWindow->GetInteractor()->SetEnableRender(true);

  unsigned long prevModelUpdateTime;
  unsigned long modelUpdateTime;

  this->LoadRenderTimeLogger->StartTimer();

  // Update the image data if the timestamp differs from last update
  if (this->ForceFullUpdate || this->CurrentFrame != this->LastFrame)
    {
    std::string imageFile =
      this->ImageDataSource->getDataFile(this->CurrentFrame);
    if (imageFile.empty())
      {
      return;
      }

    this->ForceFullUpdate = false;
    this->LastFrame = this->CurrentFrame;
    this->ImageSource->SetFileName(imageFile.c_str());

    if (this->UseGeoCoordinates || this->UseRawImageCoordinates)
      {
      this->ImageSource->UpdateInformation();
      int dim[2];
      if (this->ImageSource->GetRasterDimensions(dim))
        {
        this->WholeImageBounds[0] = 0;
        this->WholeImageBounds[1] = dim[0] - 1;
        this->WholeImageBounds[2] = 0;
        this->WholeImageBounds[3] = dim[1] - 1;
        }

      if (this->updateImageMatrices())
        {
        this->ImageActor[1]->SetUserMatrix(this->ImageToWorldMatrix);
        this->TrackHeadIndicatorActor->SetUserMatrix(this->ImageToWorldMatrix);

        // Since the 'new' user matrix may just be the same allocated matrix
        // with new values, we have to force the transform to update since it
        // internally uses pointer equality to determine if the matrix has
        // changed...
        this->ImageActor[1]->GetUserTransform()->Modified();
        this->TrackHeadIndicatorActor->GetUserTransform()->Modified();
        }
      else
        {
        std::cerr << "Unable to update image to world transform\n";
        }

      if (this->UseRawImageCoordinates)
        {
        for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
          {
          vpProject* project = this->Projects[i];
          double in[4] = {project->AOIUpperLeftLatLon[1],
                          project->AOIUpperLeftLatLon[0], 0, 1.0};
          double out[4];
          this->LatLonToImageMatrix->MultiplyPoint(in, out);
          if (out[3] != 0)
            {
            vtkSmartPointer<vtkMatrix4x4> matrix =
              vtkSmartPointer<vtkMatrix4x4>::New();
            matrix->SetElement(0, 3,
              out[0]/out[3] - this->ImageTranslationOffset[0]);
            matrix->SetElement(1, 1, -1.0);
            matrix->SetElement(1, 3,
              out[1]/out[3] - this->ImageTranslationOffset[1]);
            project->TrackRepresentation->SetRepresentationMatrix(matrix);
            project->TrackHeadRepresentation->SetRepresentationMatrix(matrix);
            project->SelectedTrackRepresentation->SetRepresentationMatrix(matrix);
            project->SelectedTrackHeadRepresentation->SetRepresentationMatrix(matrix);
            project->EventRepresentation->SetRepresentationMatrix(matrix);
            project->SelectedEventRepresentation->SetRepresentationMatrix(matrix);
            project->SceneElementRepresentation->SetRepresentationMatrix(matrix);
            project->SelectedSceneElementRepresentation->SetRepresentationMatrix(matrix);
            this->Annotation->SetRepresentationMatrix(matrix);
            // FIXME - other reperesentations probably need to be handled
            }
          else
            {
            std::cerr << "Unable to compute final image coordinates.\n";
            }
          }
        }
      else // this->UseGeoCoordinates
        {
        // track heads and fse are in image coordinates and need to be
        // transformed, including y-flip
        int dim[2];
        this->ImageSource->GetRasterDimensions(dim);
        this->YFlipMatrix->SetElement(1, 3, dim[1]);
        vtkSmartPointer<vtkMatrix4x4> transformMatrix =
          vtkSmartPointer<vtkMatrix4x4>::New();
        vtkMatrix4x4::Multiply4x4(this->ImageToWorldMatrix, this->YFlipMatrix,
                                  transformMatrix);

        for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
          {
          vpProject* project = this->Projects[i];
          project->TrackHeadRepresentation->SetRepresentationMatrix(transformMatrix);
          project->SelectedTrackHeadRepresentation->SetRepresentationMatrix(transformMatrix);
          project->SceneElementRepresentation->SetRepresentationMatrix(transformMatrix);
          project->SelectedSceneElementRepresentation->SetRepresentationMatrix(transformMatrix);
          }
        }

      this->updateExtents();
      }
    else
      {
      if (this->TrackStorageMode ==
          vpTrackIO::TSM_HomographyTransformedImageCoords)
        {
        vtkSmartPointer<vtkMatrix4x4> inverseMatrix =
          vtkSmartPointer<vtkMatrix4x4>::New();
        vpFrame frameMetaData;
        if (this->FrameMap->getFrame(this->CurrentFrame, frameMetaData) &&
          frameMetaData.Homography)
          {
          vtkMatrix4x4::Invert(frameMetaData.Homography, inverseMatrix);
          }
        vtkSmartPointer<vtkMatrix4x4> flipMatrix =
          vtkSmartPointer<vtkMatrix4x4>::New();
        flipMatrix->SetElement(1, 1, -1.0);
        flipMatrix->SetElement(1, 3,
                               this->Projects[0]->ModelIO->GetImageHeight());
        vtkSmartPointer<vtkMatrix4x4> transformMatrix =
          vtkSmartPointer<vtkMatrix4x4>::New();
        vtkMatrix4x4::Multiply4x4(flipMatrix, inverseMatrix, transformMatrix);
        // If [3][3] component is negative, we're likely to (will?) run into
        // issues with OpenGL decided to clip away the data.  Therefore scale
        // to make sure the [3][3] element is positive.
        if (transformMatrix->Element[3][3] < 0)
          {
          for (int i = 0; i < 4; ++i)
            {
            for (int j = 0; j < 4; ++j)
              {
              transformMatrix->Element[i][j] *= -1;
              }
            }
          // we "Position" the non image actors "above" (+z) the image; the
          // [2][2] component must be 1 (versus -1, which it would be because
          // of the previously scaling by -1) to insure the z position isn't
          // altered to be behind the image.
          transformMatrix->Element[2][2] = 1.0;
          }
        for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
          {
          vpProject* project = this->Projects[i];
          project->TrackRepresentation->SetRepresentationMatrix(
            transformMatrix);
          project->SelectedTrackRepresentation->SetRepresentationMatrix(
            transformMatrix);
          project->EventRepresentation->SetRepresentationMatrix(
            transformMatrix);
          project->SelectedEventRepresentation->SetRepresentationMatrix(
            transformMatrix);
          project->TrackHeadRepresentation->SetRepresentationMatrix(
            flipMatrix);
          project->SelectedTrackHeadRepresentation->SetRepresentationMatrix(
            flipMatrix);
          project->SceneElementRepresentation->SetRepresentationMatrix(
            flipMatrix);
          project->SelectedSceneElementRepresentation->SetRepresentationMatrix(
            flipMatrix);
          }
        this->Annotation->SetRepresentationMatrix(transformMatrix);
        }

      this->updateAOIImagery();
      }
    }

  // If we're not doing time-based indexing but timestamp data is available,
  // go ahead and "fill out" the core timestamp with the image time, since
  // we still want to give a time-based timestamp to the models.
  if (this->UsingTimeStampData && !this->UsingTimeBasedIndexing)
    {
    if (this->CurrentFrame != this->LastFrame || !this->CoreTimeStamp.HasTime())
      {
      vpFrame frame;
      if (this->FrameMap->getFrame(this->CurrentFrame, frame))
        {
        this->CoreTimeStamp.SetTime(frame.Time.GetTime());
        }
      else
        {
        this->ImageSource->UpdateInformation();
        this->CoreTimeStamp.SetTime(
          this->ImageSource->GetImageTimeStamp().GetTime());
        }
      }
    }

  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    vpProject* project = this->Projects[i];

    // Update the event model
    prevModelUpdateTime = project->EventModel->GetUpdateTime();
    project->EventModel->Update(this->CoreTimeStamp);
    modelUpdateTime = project->EventModel->GetUpdateTime();

    // Update event representations
    this->updateRepresentation(project->EventRepresentation,
                               this->EventFilter,
                               prevModelUpdateTime, modelUpdateTime);
    this->updateRepresentation(project->SelectedEventRepresentation,
                               this->EventFilter,
                               prevModelUpdateTime, modelUpdateTime);
    this->updateRepresentation(project->EventRegionRepresentation,
                               this->EventFilter,
                               prevModelUpdateTime, modelUpdateTime);
    this->updateRepresentation(project->EventIconRepresentation,
                               this->EventFilter,
                               prevModelUpdateTime, modelUpdateTime);

    // Update the track model
    prevModelUpdateTime = project->TrackModel->GetUpdateTime();
    project->TrackModel->Update(this->CoreTimeStamp);
    modelUpdateTime = project->TrackModel->GetUpdateTime();

    // Update the track representations
    this->updateRepresentation(project->TrackRepresentation,
                               this->TrackFilter,
                               prevModelUpdateTime, modelUpdateTime);
    this->updateRepresentation(project->SelectedTrackRepresentation,
                               this->TrackFilter,
                               prevModelUpdateTime, modelUpdateTime);
    this->updateRepresentation(project->TrackHeadRepresentation,
                               this->TrackFilter,
                               prevModelUpdateTime, modelUpdateTime);
    this->updateRepresentation(project->SelectedTrackHeadRepresentation,
                               this->TrackFilter,
                               prevModelUpdateTime, modelUpdateTime);
    this->updateRepresentation(project->SceneElementRepresentation,
                               0,
                               prevModelUpdateTime, modelUpdateTime);
    this->updateRepresentation(project->SelectedSceneElementRepresentation,
                               0,
                               prevModelUpdateTime, modelUpdateTime);

    // Update the activity manager. If a new activity actor is added to the
    // renderer, we must re-add the annotation prop to ensure it still gets
    // rendered after everything else.
    if (project->ActivityManager->UpdateActivityActors(this->CoreTimeStamp))
      {
      this->SceneRenderer->RemoveViewProp(this->Annotation->GetProp());
      this->SceneRenderer->AddViewProp(this->Annotation->GetProp());
      }
    }

  // Update the track head editor widget
  if (this->isEditingTrack())
    {
    this->updateTrackHeadRegion();
    }

  // Update the graph model
  if (this->GraphRenderingEnabled)
    {
    prevModelUpdateTime = this->GraphModel->GetUpdateTime();
    this->GraphModel->Update(this->CoreTimeStamp);
    modelUpdateTime = this->GraphModel->GetUpdateTime();

    // Update the graph representation
    this->updateRepresentation(this->GraphRepresentation, this->EventFilter,
                               prevModelUpdateTime, modelUpdateTime);
    }

  this->Annotation->Update();

  // Update tracking camera
  if (this->IdOfTrackToFollow > -1)
    {
    this->updateTrackFollowCamera();
    }

  if (this->UpdateObjectViews)
    {
    this->refreshSelectionPanel();
    this->UpdateObjectViews = false;
    }

  this->forceRender();

  this->LoadRenderTimeLogger->StopTimer();
  this->RenderingTime = this->LoadRenderTimeLogger->GetElapsedTime() * 1000;
}

//-----------------------------------------------------------------------------
void vpViewCore::setProjectVisible(int index, bool visible)
{
  if (this->Projects[index]->IsVisible == visible)
    {
    return;
    }

  int tracksVisible = visible && this->ShowTracks ? 1 : 0;
  int trackHeadsVisible = visible && this->ShowTrackHeads ? 1 : 0;
  int eventsVisible = visible && this->ShowEvents ? 1 : 0;
  int eventIconsVisible = visible && this->ShowEventIcons ? 1 : 0;
  int activitiesVisible = visible && this->ShowActivities ? 1 : 0;
  int sceneElementsVisible = visible && this->ShowSceneElements ? 1 : 0;
  this->Projects[index]->IsVisible = visible;
  this->Projects[index]->TrackRepresentation->SetVisible(tracksVisible);
  this->Projects[index]->SelectedTrackRepresentation->SetVisible(tracksVisible);
  this->Projects[index]->TrackHeadRepresentation->SetVisible(trackHeadsVisible);
  this->Projects[index]->SelectedTrackHeadRepresentation->SetVisible(trackHeadsVisible);
  this->Projects[index]->EventRepresentation->SetVisible(eventsVisible);
  this->Projects[index]->SelectedEventRepresentation->SetVisible(eventsVisible);
  this->Projects[index]->EventRegionRepresentation->SetVisible(eventsVisible);
  this->Projects[index]->EventIconRepresentation->SetVisible(eventIconsVisible);
  this->Projects[index]->ActivityManager->SetVisibility(activitiesVisible);
  this->Projects[index]->SceneElementRepresentation->SetVisible(sceneElementsVisible);
  this->Projects[index]->SelectedSceneElementRepresentation->SetVisible(sceneElementsVisible);
  this->updateScene();

  emit this->projectVisibilityChanged(index, visible);
}

//-----------------------------------------------------------------------------
void vpViewCore::onViewTracks(bool state)
{
  this->ShowTracks = state;
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->TrackRepresentation->SetVisible(
      state && this->Projects[i]->IsVisible ? 1 : 0);
    this->Projects[i]->SelectedTrackRepresentation->SetVisible(
      state && this->Projects[i]->IsVisible ? 1 : 0);
    }
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::onViewTrackHeads(bool state)
{
  this->ShowTrackHeads = state;
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->TrackHeadRepresentation->SetVisible(
      state && this->Projects[i]->IsVisible ? 1 : 0);
    this->Projects[i]->SelectedTrackHeadRepresentation->SetVisible(
      state && this->Projects[i]->IsVisible ? 1 : 0);
    }
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::onViewEvents(bool state)
{
  this->ShowEvents = state;
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    int visible = (state && this->Projects[i]->IsVisible) ? 1 : 0;
    this->Projects[i]->EventRepresentation->SetVisible(visible);
    this->Projects[i]->SelectedEventRepresentation->SetVisible(visible);
    this->Projects[i]->EventRegionRepresentation->SetVisible(visible);
    }
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::onViewActivities(bool state)
{
  this->ShowActivities = state;
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->ActivityManager->SetVisibility(
      state && this->Projects[i]->IsVisible ? 1 : 0);
    }
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::onViewSceneElements(bool state)
{
  this->ShowSceneElements = state;
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->SceneElementRepresentation->SetVisible(
      state && this->Projects[i]->IsVisible ? 1 : 0);
    this->Projects[i]->SelectedSceneElementRepresentation->SetVisible(
      state && this->Projects[i]->IsVisible ? 1 : 0);
    }
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::onViewEventIcons(bool show)
{
  this->ShowEventIcons = show;
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->EventIconRepresentation->SetVisible(
      show && this->Projects[i]->IsVisible ? 1 : 0);
    }
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::onRandomEventColor(bool state)
{
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->EventRepresentation->RandomizeEventColors(state);
    }
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::onRandomTrackColor(bool state)
{
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    if (state)
      {
      this->Projects[i]->TrackRepresentation->SetColorModeToRandom();
      this->Projects[i]->TrackHeadRepresentation->SetColorModeToRandom();
      }
    else
      {
      this->Projects[i]->TrackRepresentation->SetColorModeToModel();
      this->Projects[i]->TrackHeadRepresentation->SetColorModeToModel();
      }
    }

  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::onDisplayFullVolume(bool state, bool render)
{
  this->DisplayFullVolume = state;

  // Full volume setting is overridden if an expiration window is set.
  if (this->ObjectExpirationTime.IsValid())
    {
    return;
    }

  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    vpProject* project = this->Projects[i];
    if (state)
      {
      project->EventModel->ShowEventsBeforeStartOn();
      project->EventModel->ShowEventsAfterExpirationOn();
      project->TrackModel->ShowTracksBeforeStartOn();
      project->TrackModel->ShowTracksAfterExpirationOn();
      }
    else
      {
      project->TrackModel->ShowTracksBeforeStartOff();
      project->TrackModel->ShowTracksAfterExpirationOff();
      project->EventModel->ShowEventsBeforeStartOff();
      this->updateEventDisplayEndFrame(project);
      }
    project->ActivityManager->SetShowFullVolume(state);
    }

  if (render)
    {
    this->updateScene();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::enableEventsChange(int eventType, bool state)
{
  this->EventFilter->SetShowType(eventType, state);
  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::enableInverseEventsChange(int eventType, bool state)
{
  this->EventFilter->SetInverse(eventType, state);
  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::UpdateEventDisplayStates(bool state)
{
  // now make sure the EventManager is setup correctly (proper events on/off)
  for (int i = 0, end = this->EventConfig->GetNumberOfTypes(); i < end; ++i)
    {
    const vgEventType& et = this->EventConfig->GetEventTypeByIndex(i);
    this->EventFilter->SetShowType(et.GetId(), state);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::SetTrackTypeDisplayState(int trackType, bool state)
{
  this->TrackFilter->SetShowType(trackType, state);
  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::SetTrackTypeProbabilityLimit(int trackType, double prob)
{
  // HACK: Invert the sense of the Other filter, so that it becomes low-pass
  // rather than high-pass. This should be configurable in the UI, via a range
  // slider filter or some other means.
  if (trackType == vtkVgTrack::Other)
    {
    this->TrackFilter->SetMaxProbability(trackType, prob);
    }
  else
    {
    this->TrackFilter->SetMinProbability(trackType, prob);
    }
  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::SetEventTypeDisplayState(int eventType, bool state)
{
  this->enableEventsChange(eventType, state);
}

//-----------------------------------------------------------------------------
void vpViewCore::SetEventInverseDisplayState(int eventType, bool state)
{
  this->enableInverseEventsChange(eventType, state);
}

//-----------------------------------------------------------------------------
void vpViewCore::SetEventTypeNormalcyThreshold(int eventType, double threshold)
{
  this->EventFilter->SetMaxNormalcy(eventType, threshold);
  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::SetActivityTypeDisplayState(int type, bool state)
{
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->ActivityManager->SetDisplayActivityType(type, state);
    }
  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::SetActivityTypeSaliencyThreshold(int type, double threshold)
{
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->ActivityManager->SetSaliencyThreshold(type, threshold);
    }
  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::onUseNormalcyCues(bool enable)
{
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->EventRepresentation->SetUseNormalcyCues(enable);
    this->Projects[i]->EventIconRepresentation->SetUseNormalcyCues(enable);
    }
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::onSwapNormalcyCues()
{
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->EventRepresentation->SwapNormalcyCues();
    this->Projects[i]->EventIconRepresentation->SwapNormalcyCues();
    }
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::createDisplayOutline(vpProject* project)
{
  vtkSmartPointer<vtkPoints> aoiOutlinePoints = vtkSmartPointer<vtkPoints>::New();
  aoiOutlinePoints->SetDataTypeToDouble();
  aoiOutlinePoints->SetNumberOfPoints(4);

  if (this->UseGeoCoordinates)
    {
    if (project->AOIUpperLeftLatLon[0] != 444)
      {
      double ul[] = { project->AOIUpperLeftLatLon[1],
                      project->AOIUpperLeftLatLon[0], 0.0, 1.0 };
      this->LatLonToWorldMatrix->MultiplyPoint(ul, ul);
      ul[0] /= ul[3];
      ul[1] /= ul[3];
      if (project->AOILowerRightLatLon[0] != 444)
        {
        double lr[] = { project->AOILowerRightLatLon[1],
                        project->AOILowerRightLatLon[0], 0.0, 1.0 };
        this->LatLonToWorldMatrix->MultiplyPoint(lr, lr);
        lr[0] /= lr[3];
        lr[1] /= lr[3];
        if (project->AOILowerLeftLatLon[0] != 444 &&
            project->AOIUpperRightLatLon[0] != 444)
          {
          // All 4 corners.
          double ll[] = { project->AOILowerLeftLatLon[1],
                          project->AOILowerLeftLatLon[0], 0.0, 1.0 };
          this->LatLonToWorldMatrix->MultiplyPoint(ll, ll);
          ll[0] /= ll[3];
          ll[1] /= ll[3];
          double ur[] = { project->AOIUpperRightLatLon[1],
                          project->AOIUpperRightLatLon[0], 0.0, 1.0 };
          this->LatLonToWorldMatrix->MultiplyPoint(ur, ur);
          ur[0] /= ur[3];
          ur[1] /= ur[3];
          aoiOutlinePoints->SetPoint(0, ll);
          aoiOutlinePoints->SetPoint(1, lr);
          aoiOutlinePoints->SetPoint(2, ur);
          aoiOutlinePoints->SetPoint(3, ul);
          }
        else
          {
          // Two corners.
          aoiOutlinePoints->SetPoint(0, ul[0], lr[1], 0);
          aoiOutlinePoints->SetPoint(1, lr[0], lr[1], 0);
          aoiOutlinePoints->SetPoint(2, lr[0], ul[1], 0);
          aoiOutlinePoints->SetPoint(3, ul[0], ul[1], 0);
          }
        }
      else
        {
        // Just one corner - show a 'notch' in the upper left corner.
        aoiOutlinePoints->SetPoint(0, ul[0], ul[1] - 100, 0);
        aoiOutlinePoints->SetPoint(1, ul[0], ul[1], 0);
        aoiOutlinePoints->SetPoint(2, ul[0] + 100, ul[1], 0);
        aoiOutlinePoints->SetPoint(3, ul[0], ul[1], 0);
        }
      }
    else
      {
      aoiOutlinePoints->SetPoint(0, this->ViewExtents[0], this->ViewExtents[2], 0);
      aoiOutlinePoints->SetPoint(1, this->ViewExtents[1], this->ViewExtents[2], 0);
      aoiOutlinePoints->SetPoint(2, this->ViewExtents[1], this->ViewExtents[3], 0);
      aoiOutlinePoints->SetPoint(3, this->ViewExtents[0], this->ViewExtents[3], 0);
      }
    }
  else if (this->UseRawImageCoordinates)
    {
    if (project->AOILowerRightLatLon[0] != 444)
      {
      double lr[4] = { project->AOILowerRightLatLon[1],
                       project->AOILowerRightLatLon[0], 0, 1.0 };

      this->LatLonToWorldMatrix->MultiplyPoint(lr, lr);
      if (lr[3] != 0)
        {
        lr[0] /= lr[3];
        lr[1] /= lr[3];
        }

      if (project->AOILowerLeftLatLon[0] != 444 &&
          project->AOIUpperRightLatLon[0] != 444)
        {
        // All four corners.
        double ll[4] = { project->AOILowerLeftLatLon[1],
                         project->AOILowerLeftLatLon[0], 0, 1.0 };

        this->LatLonToWorldMatrix->MultiplyPoint(ll, ll);
        if (ll[3] != 0)
          {
          ll[0] /= ll[3];
          ll[1] /= ll[3];
          }
        double ur[4] = { project->AOIUpperRightLatLon[1],
                         project->AOIUpperRightLatLon[0], 0, 1.0 };

        this->LatLonToWorldMatrix->MultiplyPoint(ur, ur);
        if (ur[3] != 0)
          {
          ur[0] /= ur[3];
          ur[1] /= ur[3];
          }

        aoiOutlinePoints->SetPoint(0, ll[0], ll[1], 0);
        aoiOutlinePoints->SetPoint(1, lr[0], lr[1], 0);
        aoiOutlinePoints->SetPoint(2, ur[0], ur[1], 0);
        aoiOutlinePoints->SetPoint(3, 0, 0, 0);
        }
      else
        {
        // Two corners.
        aoiOutlinePoints->SetPoint(0, 0, lr[1], 0);
        aoiOutlinePoints->SetPoint(1, lr[0], lr[1], 0);
        aoiOutlinePoints->SetPoint(2, lr[0], 0, 0);
        aoiOutlinePoints->SetPoint(3, 0, 0, 0);
        }
      }
    else
      {
      // Just one corner - show a 'notch' in the upper left corner.
      aoiOutlinePoints->SetPoint(0, 0, -100, 0);
      aoiOutlinePoints->SetPoint(1, 0, 0, 0);
      aoiOutlinePoints->SetPoint(2, 100, 0, 0);
      aoiOutlinePoints->SetPoint(3, 0, 0, 0);
      }
    }
  else // Inverted image coordinates
    {
    aoiOutlinePoints->SetPoint(0, 0, 0, 0);
    aoiOutlinePoints->SetPoint(1, project->AnalysisDimensions[0] - 1, 0, 0);
    aoiOutlinePoints->SetPoint(2, project->AnalysisDimensions[0] - 1,
                               project->AnalysisDimensions[1] - 1, 0);
    aoiOutlinePoints->SetPoint(3, 0, project->AnalysisDimensions[1] - 1, 0);
    }

  vtkSmartPointer<vtkCellArray> aoiOutlineLines = vtkSmartPointer<vtkCellArray>::New();
  vtkIdType ptIds[5] = {0, 1, 2, 3, 0};
  aoiOutlineLines->InsertNextCell(5, ptIds);

  this->AOIOutlinePolyData = vtkSmartPointer<vtkPolyData>::New();
  this->AOIOutlinePolyData->SetPoints(aoiOutlinePoints);
  this->AOIOutlinePolyData->SetLines(aoiOutlineLines);

  vtkSmartPointer<vtkPolyDataMapper> aoiOutlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  aoiOutlineMapper->SetInputData(this->AOIOutlinePolyData);

  this->AOIOutlineActor = vtkSmartPointer<vtkActor>::New();
  this->AOIOutlineActor->SetMapper(aoiOutlineMapper);
  this->AOIOutlineActor->GetProperty()->SetColor(1, 0.5, 0);

  this->SceneRenderer->AddViewProp(this->AOIOutlineActor);
}

//-----------------------------------------------------------------------------
void vpViewCore::handleDataSetNotFound(vpProject* project)
{
  emit this->criticalError(
    QString("Unable to read dataset using path (\"%1\").\n").arg(
      qtString(project->DataSetSpecifier)));
}

//-----------------------------------------------------------------------------
void vpViewCore::handleFileNotFound(const std::string& tag, const std::string& file)
{
  QString warningMsg = tr("Unable to find file ( %1 ) for %2")
                         .arg(QString(file.c_str()), QString(tag.c_str()));

  emit this->warningError(warningMsg);
}

//-----------------------------------------------------------------------------
void vpViewCore::onIncreaseIconXOffset()
{
  this->changeIconOffset(1, 0);
}

//-----------------------------------------------------------------------------
void vpViewCore::onDecreaseIconXOffset()
{
  this->changeIconOffset(-1, 0);
}

//-----------------------------------------------------------------------------
void vpViewCore::onIncreaseIconYOffset()
{
  this->changeIconOffset(0, 1);
}

//-----------------------------------------------------------------------------
void vpViewCore::onDecreaseIconYOffset()
{
  this->changeIconOffset(0, -1);
}

//-----------------------------------------------------------------------------
void vpViewCore::onIncreaseOverlayTransparency()
{
  this->changeOverlayOpacity(-0.1);
}

//-----------------------------------------------------------------------------
void vpViewCore::onDecreaseOverlayTransparency()
{
  this->changeOverlayOpacity(0.1);
}

//-----------------------------------------------------------------------------
void vpViewCore::onIncreaseSceneElementTransparency()
{
  this->changeSceneElementOpacity(-0.1);
}

//-----------------------------------------------------------------------------
void vpViewCore::onDecreaseSceneElementTransparency()
{
  this->changeSceneElementOpacity(0.1);
}

//-----------------------------------------------------------------------------
void vpViewCore::changeSceneElementOpacity(double delta)
{
  double opacity = this->SceneElementFillOpacity;
  opacity += delta;

  if (opacity < 0.1)
    {
    opacity = 0.1;
    }
  else if (opacity > 1.0)
    {
    opacity = 1.0;
    }

  this->SceneElementFillOpacity = opacity;
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->SceneElementRepresentation->SetFillOpacity(opacity);
    }

  this->render();
}

//-----------------------------------------------------------------------------
void vpViewCore::changeIconOffset(int deltaX, int deltaY)
{
  this->IconOffsetX += deltaX;
  this->IconOffsetY += deltaY;

  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->IconManager->SetIconOffset(this->IconOffsetX,
                                                  this->IconOffsetY);

    // This call is required to update the state.
    this->Projects[i]->IconManager->UpdateIconActors(
      this->CoreTimeStamp.GetFrameNumber());
    }

  this->render();
}

//-----------------------------------------------------------------------------
void vpViewCore::changeOverlayOpacity(double delta)
{
  double opacity = this->OverlayOpacity;
  opacity += delta;

  if (opacity < 0.1)
    {
    opacity = 0.1;
    }
  else if (opacity > 1.0)
    {
    opacity = 1.0;
    }

  this->OverlayOpacity = opacity;
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->ActivityManager->SetOverlayOpacity(opacity);
    }

  this->render();
}

//-----------------------------------------------------------------------------
void vpViewCore::initializeImageSource()
{
  // Check here the extension of the files.
  // Grab the first file.
  std::string fileName = this->ImageDataSource->getDataFile(0);
  this->ImageSource.TakeReference(vpImageSourceFactory::GetInstance()->Create(fileName));

  if (!this->ImageSource)
    {
    QString errorMsg = QString("Unable to create image souce for \"%1\"");
    emit this->criticalError(errorMsg.arg(qtString(fileName)));
    return;
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::addEvents(vpProject* project)
{
    {
    // normal event representation
    project->EventRepresentation->UpdateEventTypes();
    vtkPropCollection* eventProps =
      project->EventRepresentation->GetActiveRenderObjects();
    eventProps->InitTraversal();
    while (vtkProp* prop = eventProps->GetNextProp())
      {
      this->SceneRenderer->AddViewProp(prop);
      }
    }
    {
    // event region representation
    vtkPropCollection* eventProps =
      project->EventRegionRepresentation->GetActiveRenderObjects();
    eventProps->InitTraversal();
    while (vtkProp* prop = eventProps->GetNextProp())
      {
      this->SceneRenderer->AddViewProp(prop);
      }
    }
    {
    // selected event representation
    project->SelectedEventRepresentation->UpdateEventTypes();
    vtkPropCollection* eventProps =
      project->SelectedEventRepresentation->GetActiveRenderObjects();
    eventProps->InitTraversal();
    while (vtkProp* prop = eventProps->GetNextProp())
      {
      this->SceneRenderer->AddViewProp(prop);
      }
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::removeEvents(vpProject* project)
{
  // Remove events from renderer.
  vtkPropCollection* eventProps =
    project->EventRepresentation->GetActiveRenderObjects();
  eventProps->InitTraversal();
  while (vtkProp* prop = eventProps->GetNextProp())
    {
    this->SceneRenderer->RemoveViewProp(prop);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::addTrackHeads(vpProject* project)
{
    {
    vtkPropCollection* headProps =
      project->TrackHeadRepresentation->GetActiveRenderObjects();
    headProps->InitTraversal();
    while (vtkProp* prop = headProps->GetNextProp())
      {
      this->SceneRenderer->AddViewProp(prop);
      }
    }
    {
    vtkPropCollection* headProps =
      project->SelectedTrackHeadRepresentation->GetActiveRenderObjects();
    headProps->InitTraversal();
    while (vtkProp* prop = headProps->GetNextProp())
      {
      this->SceneRenderer->AddViewProp(prop);
      }
    }
    {
    vtkPropCollection* headProps =
      project->SceneElementRepresentation->GetActiveRenderObjects();
    headProps->InitTraversal();
    while (vtkProp* prop = headProps->GetNextProp())
      {
      this->SceneRenderer->AddViewProp(prop);
      }
    }
    {
    vtkPropCollection* headProps =
      project->SelectedSceneElementRepresentation->GetActiveRenderObjects();
    headProps->InitTraversal();
    while (vtkProp* prop = headProps->GetNextProp())
      {
      this->SceneRenderer->AddViewProp(prop);
      }
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::removeTrackHeads(vpProject* project)
{
  // Remove track heads from the renderer.
    {
    vtkPropCollection* headProps =
      project->TrackHeadRepresentation->GetActiveRenderObjects();
    headProps->InitTraversal();
    while (vtkProp* prop = headProps->GetNextProp())
      {
      this->SceneRenderer->RemoveViewProp(prop);
      }
    }
    {
    vtkPropCollection* headProps =
      project->SelectedTrackHeadRepresentation->GetActiveRenderObjects();
    headProps->InitTraversal();
    while (vtkProp* prop = headProps->GetNextProp())
      {
      this->SceneRenderer->RemoveViewProp(prop);
      }
    }
    {
    vtkPropCollection* headProps =
      project->SceneElementRepresentation->GetActiveRenderObjects();
    headProps->InitTraversal();
    while (vtkProp* prop = headProps->GetNextProp())
      {
      this->SceneRenderer->RemoveViewProp(prop);
      }
    }
    {
    vtkPropCollection* headProps =
      project->SelectedSceneElementRepresentation->GetActiveRenderObjects();
    headProps->InitTraversal();
    while (vtkProp* prop = headProps->GetNextProp())
      {
      this->SceneRenderer->RemoveViewProp(prop);
      }
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::addTracks(vpProject* project)
{
  // Add track actors to renderer. Track actors are added *after* events so that
  // tracks and events won't be alpha blended together and show misleading
  // colors.
    {
    vtkPropCollection* trackProps =
      project->TrackRepresentation->GetActiveRenderObjects();
    trackProps->InitTraversal();
    while (vtkProp* prop = trackProps->GetNextProp())
      {
      this->SceneRenderer->AddViewProp(prop);
      }
    }
    {
    vtkPropCollection* trackProps =
      project->SelectedTrackRepresentation->GetActiveRenderObjects();
    trackProps->InitTraversal();
    while (vtkProp* prop = trackProps->GetNextProp())
      {
      this->SceneRenderer->AddViewProp(prop);
      }
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::removeTracks(vpProject* project)
{
  // Remove tracks from renderer.
  vtkPropCollection* trackProps =
    project->TrackRepresentation->GetActiveRenderObjects();
  trackProps->InitTraversal();
  while (vtkProp* prop = trackProps->GetNextProp())
    {
    this->SceneRenderer->RemoveViewProp(prop);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::sanitizeSceneRenderer()
{
  // First remove actors from representations.
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->removeTracks(this->Projects[i]);
    this->removeTrackHeads(this->Projects[i]);
    this->removeEvents(this->Projects[i]);
    this->SceneRenderer->RemoveViewProp(
      this->Projects[i]->IconManager->GetIconActor());
    }

  if (this->SceneRenderer)
    {
    if (this->Annotation)
      {
      this->Annotation->SetVisible(false);
      }
    if (this->AOIOutlineActor)
      {
      this->SceneRenderer->RemoveViewProp(this->AOIOutlineActor);
      }
    if (this->EventLegend)
      {
      this->SceneRenderer->RemoveViewProp(this->EventLegend);
      }

    this->SceneRenderer->RemoveViewProp(this->ImageActor[1]);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::setSaveRenderedImages(bool state, QString* outputDir/*=0*/)
{
  this->SaveRenderedImages = state;
  if (outputDir)
    {
    this->ImageOutputDirectory = *outputDir;
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::onHoverStart()
{
  if (vtkVgActivity* a = this->getHoveredActivity())
    {
    if (this->HoveredActivity)
      {
      this->HoveredActivity->SetShowLabel(false);
      }

    this->IsHovering = true;
    this->HoveredActivity = a;
    this->HoveredActivity->SetShowLabel(true);
    this->render();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::onHoverStop()
{
  this->IsHovering = false;

  if (this->HoveredActivity)
    {
    QTimer::singleShot(500, this, SLOT(onHoverTimer()));
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::onHoverTimer()
{
  if (!this->IsHovering)
    {
    if (this->HoveredActivity && !this->getHoveredActivity())
      {
      this->HoveredActivity->SetShowLabel(false);
      this->HoveredActivity = 0;
      this->render();
      }
    }
}

//-----------------------------------------------------------------------------
vtkVgActivity* vpViewCore::getHoveredActivity()
{
  vtkRenderWindowInteractor* interactor = this->RenderWindow->GetInteractor();
  int x = interactor->GetEventPosition()[0];
  int y = interactor->GetEventPosition()[1];

  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    int pickStatus =
      this->Projects[i]->Picker->Pick(x, y, 0.0, this->SceneRenderer);

    if (pickStatus == vtkVgPickData::PickedActivity)
      {
      vtkIdType id = this->Projects[i]->Picker->GetActivityIndex();
      return this->Projects[i]->ActivityManager->GetActivity(id);
      }
    }

  return 0;
}

//-----------------------------------------------------------------------------
void vpViewCore::eventTypeChanged(int vtkNotUsed(type))
{
  this->updateEventLegendColors();
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->Projects[i]->EventRepresentation->Modified();
    }
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::activityTypeChanged(int vtkNotUsed(type))
{
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    if (this->Projects[i]->ActivityManager->GetNumberOfActivities() > 0)
      {
      this->Projects[i]->ActivityManager->UpdateActivityColors();
      }
    }

  this->render();
}

//-----------------------------------------------------------------------------
void vpViewCore::addFilterRegion(const std::string& name,
                                 const std::vector<double>& points,
                                 vtkMatrix4x4* worldToImageMatrix,
                                 vtkVgTimeStamp& timeStamp)
{
  vtkIdType numPoints = static_cast<vtkIdType>(points.size() / 2);
  vtkSmartPointer<vtkPoints> contourPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkIdList> contourIds = vtkSmartPointer<vtkIdList>::New();
  vtkSmartPointer<vtkPoints> filterPoints;
  contourPoints->SetNumberOfPoints(numPoints);
  contourIds->SetNumberOfIds(numPoints + 1);

  if (this->UseGeoCoordinates)
    {
    // Convert from lat-long points to world coords
    for (size_t i = 0, size = points.size(); i < size; i += 2)
      {
      double point[4] =
        {
        points[i + 1],
        points[i + 0],
        0.0, 1.0
        };

      this->LatLonToWorldMatrix->MultiplyPoint(point, point);
      point[0] /= point[3];
      point[1] /= point[3];
      point[2] = 0.0;

      vtkIdType id = static_cast<vtkIdType>(i / 2);
      contourPoints->SetPoint(id, point);
      contourIds->SetId(id, id);
      }

    // Track points are in the same coordinate system
    filterPoints = contourPoints;
    }
  else
    {
    if (this->UseRawImageCoordinates)
      {
      // Convert from lat-long points to track-space points
      double in[4] = { this->Projects[0]->AOIUpperLeftLatLon[1],
                       this->Projects[0]->AOIUpperLeftLatLon[0], 0, 1.0 };
      double out[4];
      this->LatLonToImageMatrix->MultiplyPoint(in, out);
      if (out[3] == 0.0)
        {
        qWarning() << "Lat-lon to image transform is invalid";
        return;
        }

      vtkSmartPointer<vtkMatrix4x4> worldToTrack =
        vtkSmartPointer<vtkMatrix4x4>::New();
      worldToTrack->SetElement(0, 3,
                               out[0]/out[3] - this->ImageTranslationOffset[0]);
      worldToTrack->SetElement(1, 1, -1.0);
      worldToTrack->SetElement(1, 3,
                               out[1]/out[3] - this->ImageTranslationOffset[1]);
      worldToTrack->Invert();

      filterPoints = vtkSmartPointer<vtkPoints>::New();
      filterPoints->SetNumberOfPoints(contourPoints->GetNumberOfPoints());

      for (size_t i = 0, size = points.size(); i < size; i += 2)
        {
        double point[4] =
          {
          points[i + 1],
          points[i + 0],
          0.0, 1.0
          };

        // First go from lat-lon to world
        this->LatLonToWorldMatrix->MultiplyPoint(point, point);
        point[0] /= point[3];
        point[1] /= point[3];
        point[2] = 0.0;
        point[3] = 1.0;

        vtkIdType id = static_cast<vtkIdType>(i / 2);
        contourPoints->SetPoint(id, point);
        contourIds->SetId(id, id);

        // Then to track space
        worldToTrack->MultiplyPoint(point, point);
        point[0] /= point[3];
        point[1] /= point[3];
        point[2] = 0.0;

        filterPoints->SetPoint(id, point);
        }
      }
    else
      {
      // Normal image coordinate mode, all we do is y-flip
      double maxY = this->Projects[0]->AnalysisDimensions[1] - 1;
      for (size_t i = 0, size = points.size(); i < size; i += 2)
        {
        double point[4] =
          {
          points[i + 0],
          maxY - points[i + 1],
          0.0, 1.0
          };

        vtkIdType id = static_cast<vtkIdType>(i / 2);
        contourPoints->SetPoint(id, point);
        contourIds->SetId(id, id);
        }

      // Track points are in the same coordinate system
      filterPoints = contourPoints;
      }
    }

  // Close the loop
  contourIds->SetId(contourIds->GetNumberOfIds() - 1, 0);

  vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
  cells->InsertNextCell(contourIds);

  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  polydata->SetPoints(contourPoints);
  polydata->SetLines(cells);

  vpContour* contour = this->makeFilterContour();
  contour->Initialize(polydata);
  contour->End();
  contour->Finalize();
  contour->SetVisible(0);
  contour->SetTimeStamp(timeStamp);
  contour->SetWorldToImageMatrix(worldToImageMatrix);
  this->Contours.push_back(contour);

  this->ContourOperatorManager->AddSelector(filterPoints);
  this->ContourOperatorManager->SetContourEnabled(filterPoints, false);
  emit this->filterRegionComplete(contour->GetPolyData()->GetPoints(),
                                  filterPoints, contour->GetTimeStampPtr(),
                                  contour->GetWorldToImageMatrix(),
                                  qtString(name), false);
}

//-----------------------------------------------------------------------------
void vpViewCore::drawFilterRegion()
{
  this->DrawingContour = true;
  emit this->showStatusMessage("Drawing filter region (right-click when done)");

  delete this->Contour;
  this->Contour = this->makeFilterContour();
  this->Contour->Initialize();
  this->Contour->Begin();
  this->Contour->SetTimeStamp(this->CoreTimeStamp);
  // in the case of geo coordinates, which is the main case trying to be
  // addressed by / when adding this, the head representation matrix
  // maps from raw (y down) to world; invert that to get world to raw image
  vtkSmartPointer<vtkMatrix4x4> tmpMatrix =
    vtkSmartPointer<vtkMatrix4x4>::New();
  tmpMatrix->DeepCopy(
    this->Projects[0]->TrackHeadRepresentation->GetRepresentationMatrix());
  tmpMatrix->Invert();
  this->Contour->SetWorldToImageMatrix(tmpMatrix);
}

//-----------------------------------------------------------------------------
void vpViewCore::completeFilterRegion()
{
  if (!this->Contour)
    {
    return;
    }

  emit this->showStatusMessage(QString());
  this->DrawingContour = false;
  this->Contour->End();
  this->Contour->Finalize();

  vtkPoints* points = this->Contour->GetPolyData()->GetPoints();
  if (points->GetNumberOfPoints() < 3)
    {
    delete this->Contour;
    this->Contour = 0;
    emit this->showStatusMessage("Not enough points in region", 3000);
    emit this->filterRegionComplete(0, 0, 0);
    this->render();
    }
  else
    {
    const vtkVgTimeStamp *timeStamp = this->Contour->GetTimeStampPtr();
    vtkMatrix4x4* matrix = this->Contour->GetWorldToImageMatrix();
    this->Contours.push_back(this->Contour);
    this->Contour = 0;

    // We need to make sure the contour points are in the same coordinate space
    // as the track points. This should already be true except in the case we
    // are using translated image coordinates, but just go ahead and do the
    // transform regardless.
    vtkSmartPointer<vtkPoints> xfPoints = vtkSmartPointer<vtkPoints>::New();
    xfPoints->SetNumberOfPoints(points->GetNumberOfPoints());

    vtkSmartPointer<vtkMatrix4x4> worldToTracks =
      vtkSmartPointer<vtkMatrix4x4>::New();

    // Using the representation matrix of the first project
    // FIXME: This won't work if we ever support loading multiple AOIs
    vtkMatrix4x4::Invert(
      this->Projects[0]->TrackRepresentation->GetRepresentationMatrix(),
      worldToTracks);

    // Transform the points of the contour to the space of the track data
    for (vtkIdType i = 0, npts = points->GetNumberOfPoints(); i < npts; ++i)
      {
      double pt[4];
      points->GetPoint(i, pt);
      pt[3] = 1.0;
      worldToTracks->MultiplyPoint(pt, pt);
      pt[0] /= pt[3];
      pt[1] /= pt[3];
      pt[2] = 0.0;
      xfPoints->SetPoint(i, pt);
      }

    this->ContourOperatorManager->AddSelector(xfPoints);
    emit this->filterRegionComplete(points, xfPoints, timeStamp, matrix);

    this->update();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::setContourVisible(vtkPoints* contourPoints, bool visible)
{
  for (size_t i = 0, end = this->Contours.size(); i < end; ++i)
    {
    if (this->Contours[i]->GetPolyData()->GetPoints() == contourPoints)
      {
      this->Contours[i]->SetVisible(visible ? 1 : 0);
      break;
      }
    }

  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::setFilterRegionEnabled(vtkPoints* filterPoints, bool enabled)
{
  this->ContourOperatorManager->SetContourEnabled(filterPoints, enabled);
  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::removeFilterRegion(vtkPoints* contourPoints,
                                    vtkPoints* filterPoints)
{
  this->ContourOperatorManager->RemoveSelector(filterPoints);
  for (size_t i = 0, end = this->Contours.size(); i < end; ++i)
    {
    if (this->Contours[i]->GetPolyData()->GetPoints() == contourPoints)
      {
      delete this->Contours[i];
      this->Contours.erase(this->Contours.begin() + i);
      break;
      }
    }

  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::removeAllFilterRegions()
{
  this->ContourOperatorManager->RemoveAllOperators();
  for (size_t i = 0, end = this->Contours.size(); i < end; ++i)
    {
    delete this->Contours[i];
    }
  this->Contours.clear();

  this->update();
}

//-----------------------------------------------------------------------------
int vpViewCore::getNumberOfActivities()
{
  int count = 0;
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    count += this->Projects[i]->ActivityManager->GetNumberOfActivities();
    }
  return count;
}

//-----------------------------------------------------------------------------
int vpViewCore::getNumberOfEvents()
{
  int count = 0;
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    count += this->Projects[i]->EventModel->GetNumberOfEvents();
    }
  return count;
}

//-----------------------------------------------------------------------------
int vpViewCore::getNumberOfTracks()
{
  int count = 0;
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    count += this->Projects[i]->TrackModel->GetNumberOfTracks();
    }
  return count;
}

//-----------------------------------------------------------------------------
void vpViewCore::saveCameraPosition()
{
  vtkVgRendererUtils::GetBounds(this->SceneRenderer, this->SavedViewExtents);
}

//-----------------------------------------------------------------------------
void vpViewCore::restoreCameraPosition()
{
  this->doZoomToExtent(this->SavedViewExtents);
}

//-----------------------------------------------------------------------------
int vpViewCore::addTemporalFilter(int type,
                                  const vtkVgTimeStamp& start,
                                  const vtkVgTimeStamp& end)
{
  int id =
    this->TemporalFilters->AddFilter(vtkVgTemporalFilters::FilterType(type),
                                     start, end);
  this->update();
  return id;
}

//-----------------------------------------------------------------------------
void vpViewCore::updateTemporalFilterStart(int id,
                                           const vtkVgTimeStamp& newStart)
{
  vtkVgTemporalFilters::FilterType type;
  vtkVgTimeStamp startTime;
  vtkVgTimeStamp endTime;
  bool enabled;

  this->TemporalFilters->GetFilterInfo(id, type, startTime, endTime, enabled);
  if (newStart != startTime)
    {
    this->TemporalFilters->SetFilter(id, type, newStart, endTime, enabled);
    this->update();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::updateTemporalFilterEnd(int id,
                                         const vtkVgTimeStamp& newEnd)
{
  vtkVgTemporalFilters::FilterType type;
  vtkVgTimeStamp startTime;
  vtkVgTimeStamp endTime;
  bool enabled;

  this->TemporalFilters->GetFilterInfo(id, type, startTime, endTime, enabled);
  if (newEnd != endTime)
    {
    this->TemporalFilters->SetFilter(id, type, startTime, newEnd, enabled);
    this->update();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::updateTemporalFilterType(int id, int newType)
{
  vtkVgTemporalFilters::FilterType type;
  vtkVgTimeStamp startTime;
  vtkVgTimeStamp endTime;
  bool enabled;

  this->TemporalFilters->GetFilterInfo(id, type, startTime, endTime, enabled);
  this->TemporalFilters->SetFilter(id,
                                   vtkVgTemporalFilters::FilterType(newType),
                                   startTime, endTime, enabled);
  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::enableTemporalFilter(int id, bool newState)
{
  vtkVgTemporalFilters::FilterType type;
  vtkVgTimeStamp startTime;
  vtkVgTimeStamp endTime;
  bool enabled;

  this->TemporalFilters->GetFilterInfo(id, type, startTime, endTime, enabled);
  if (enabled != newState)
    {
    this->TemporalFilters->SetFilter(id, type, startTime, endTime, newState);
    this->update();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::removeTemporalFilter(int id)
{
  this->TemporalFilters->RemoveFilter(id);
  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::removeAllTemporalFilters()
{
  this->TemporalFilters->RemoveAllFilters();
  this->update();
}

//-----------------------------------------------------------------------------
void vpViewCore::beginEditingTrack(int trackId)
{
  this->stopEditingTrack();

  int session = this->SessionView->GetCurrentSession();
  this->TrackEditProjectId = this->Projects[session]->ProjectId;

  this->EditingTrackId = trackId;
  this->RubberbandInteractorStyle->SetRubberBandSelectionWithCtrlKey(1);

  this->initTrackHeadIndicator();

  // place indicator next to mouse cursor
  int x, y;
  this->getMousePosition(&x, &y);
  this->updateTrackHeadIndicator(x, y);

  double* prevColor =
    this->Projects[session]->TrackModel->GetTrackColor(trackId);
  this->EditedTrackPrevColor[0] = prevColor[0];
  this->EditedTrackPrevColor[1] = prevColor[1];
  this->EditedTrackPrevColor[2] = prevColor[2];
  this->Projects[session]->TrackModel->SetTrackColor(trackId, TrackEditColor);
  this->Projects[session]->TrackModel->GetTrack(trackId)->SetModifiable(true);

  this->update();

  emit this->showStatusMessage(
    "Editing track (Ctrl+Drag to resize bounds, right click when done)");
}

//-----------------------------------------------------------------------------
void vpViewCore::stopEditingTrack(bool autoremove)
{
  if (!this->isEditingTrack())
    {
    return;
    }

  int projectId = this->TrackEditProjectId;

  this->TrackEditProjectId = -1;
  this->TrackHeadIndicatorActor->SetVisibility(0);
  this->RubberbandInteractorStyle->SetRubberBandSelectionWithCtrlKey(0);

  if (this->TrackHeadRegion)
    {
    this->TrackHeadRegion->SetVisible(0);
    }

  if (autoremove)
    {
    emit this->stoppedEditingTrack();
    }

  if (this->EditingTrackId == this->NewTrackId)
    {
    if (autoremove)
      {
      emit this->showStatusMessage("Auto-deleting track with no added points",
                                   3000);

      for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
        {
        if (this->Projects[i]->ProjectId == projectId)
          {
          this->deleteTrack(this->EditingTrackId, static_cast<int>(i));
          break;
          }
        }
      }
    }
  else
    {
    emit this->showStatusMessage(QString());

    if (vpProject* project = this->findProject(projectId))
      {
      vtkVgTrack* track = project->TrackModel->GetTrack(this->EditingTrackId);
      track->Close();
      track->SetModifiable(false);
      project->TrackModel->SetTrackColor(this->EditingTrackId,
                                         this->EditedTrackPrevColor);
      this->update();
      }
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::deleteTrack(int trackId, int session)
{
  if (this->isEditingTrack())
    {
    this->stopEditingTrack(false);
    }

  vpProject* project = this->Projects[session];

  std::vector<vtkVgEvent*> events;
  project->EventModel->GetEvents(trackId, events);

  if (!events.empty())
    {
    QMessageBox::StandardButton btn =
      QMessageBox::question(0, "Delete Events?",
                            QString(
                              "The track being deleted is referenced by %1 "
                              "events. Removing the track will cause these "
                              "events to be deleted.").arg(events.size()),
                            QMessageBox::Ok | QMessageBox::Cancel);

    if (btn != QMessageBox::Ok)
      {
      return;
      }

    for (size_t i = 0, end = events.size(); i < end; ++i)
      {
      project->EventModel->RemoveEvent(events[i]->GetId());
      }
    }

  project->TrackModel->RemoveTrack(trackId);

  this->SessionView->Update(true);
  emit this->objectInfoUpdateNeeded();

  this->UpdateObjectViews = false;
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::deleteEvent(int eventId, int session)
{
  vpProject* project = this->Projects[session];
  if (project->ActivityManager->IsEventUsedByActivity(eventId))
    {
    QMessageBox::information(0, "Event Deletion Failed",
                             "Cannot delete an event that is referenced by an "
                             "activity.", QMessageBox::Ok);
    return;
    }

  this->Projects[session]->EventModel->RemoveEvent(eventId);

  this->SessionView->Update(true);
  emit this->objectInfoUpdateNeeded();

  this->UpdateObjectViews = false;
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::setEventSelection(const QList<vtkIdType>& ids, int session)
{
  // Clear previous selection
  for (size_t i = 0, size = this->SelectedEvents.size(); i < size; ++i)
    {
    vtkVgEvent* event = this->SelectedEvents[i];
    event->SetDisplayFlags(event->GetDisplayFlags() &
                           ~vtkVgEventBase::DF_Selected);
    }
  this->SelectedEvents.clear();

  // Set new selection
  vpProject* project= this->Projects[session];
  foreach (vtkIdType id, ids)
    {
    vtkVgEvent* event = project->EventModel->GetEvent(id);
    event->SetDisplayFlags(event->GetDisplayFlags() |
                           vtkVgEventBase::DF_Selected);
    this->SelectedEvents.push_back(event);
    }

  project->SelectedEventRepresentation->Modified();
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::setTrackSelection(const QList<vtkIdType>& ids, int session)
{
  // Clear previous selection
  for (size_t i = 0, size = this->SelectedTracks.size(); i < size; ++i)
    {
    vtkVgTrack* track = this->SelectedTracks[i];
    track->SetDisplayFlags(track->GetDisplayFlags() & ~vtkVgTrack::DF_Selected);
    }
  this->SelectedTracks.clear();

  // Set new selection
  vpProject* project= this->Projects[session];
  foreach (vtkIdType id, ids)
    {
    vtkVgTrack* track = project->TrackModel->GetTrack(id);
    track->SetDisplayFlags(track->GetDisplayFlags() |
                           vtkVgTrack::DF_Selected);
    this->SelectedTracks.push_back(track);
    }

  project->SelectedTrackRepresentation->Modified();
  project->SelectedTrackHeadRepresentation->Modified();
  project->SelectedSceneElementRepresentation->Modified();
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::initTrackHeadIndicator(vtkPolyData* pd)
{
  this->initTrackHeadIndicator();

  this->TrackHeadIndicatorPolyData->DeepCopy(pd);
  double bounds[6];
  pd->GetBounds(bounds);

  vtkPoints* pts = this->TrackHeadIndicatorPolyData->GetPoints();

  // make points relative to the lower right corner
  for (vtkIdType i = 0, end = pts->GetNumberOfPoints(); i != end; ++i)
    {
    double point[3];
    pts->GetPoint(i, point);
    point[0] -= bounds[1];
    point[1] -= bounds[2];
    pts->SetPoint(i, point);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::initTrackHeadIndicator()
{
  if (this->TrackHeadIndicatorPolyData)
    {
    return;
    }

  this->TrackHeadIndicatorPolyData = vtkSmartPointer<vtkPolyData>::New();

  vtkPoints* points = vtkPoints::New();
  vtkCellArray* ca = vtkCellArray::New();
  vtkPolyDataMapper* pdm = vtkPolyDataMapper::New();

  // initially default to small box
  points->SetNumberOfPoints(4);
  points->SetPoint(0, -10.0, 10.0, 0.0);
  points->SetPoint(1, -10.0, 0.0, 0.0);
  points->SetPoint(2, 0.0, 0.0, 0.0);
  points->SetPoint(3, 0.0, 10.0, 0.0);

  vtkIdType ids[] = { 0, 1, 2, 3, 0 };
  ca->InsertNextCell(5, ids);

  this->TrackHeadIndicatorPolyData->SetPoints(points);
  this->TrackHeadIndicatorPolyData->SetLines(ca);
  pdm->SetInputData(this->TrackHeadIndicatorPolyData);

  this->TrackHeadIndicatorActor->SetMapper(pdm);
  this->TrackHeadIndicatorActor->GetProperty()->SetColor(TrackEditColor);
  this->TrackHeadIndicatorActor->SetUserMatrix(this->ImageToWorldMatrix);

  points->FastDelete();
  ca->FastDelete();
  pdm->FastDelete();
}

//-----------------------------------------------------------------------------
void vpViewCore::updateTrackHeadIndicator(int x, int y)
{
  if (this->HideTrackHeadIndicator)
    {
    return;
    }

  // hide the indicator if close enough to manipulate head contour
  if (this->TrackHeadRegion &&
      this->TrackHeadRegion->GetVisible() &&
      this->TrackHeadRegion->CanInteract(x, y))
    {
    this->TrackHeadIndicatorActor->SetVisibility(0);
    return;
    }

  this->TrackHeadIndicatorActor->SetVisibility(1);

  // screen to world
  double p[4];
  this->SceneRenderer->SetDisplayPoint(static_cast<double>(x),
                                       static_cast<double>(y), 0.0);
  this->SceneRenderer->DisplayToWorld();
  this->SceneRenderer->GetWorldPoint(p);

  this->WorldToImageMatrix->MultiplyPoint(p, p);
  p[0] /= p[3];
  p[1] /= p[3];

  this->TrackHeadIndicatorActor->SetPosition(vtkMath::Round(p[0]),
                                             vtkMath::Round(p[1]),
                                             0.5);
}

//-----------------------------------------------------------------------------
void vpViewCore::updateTrackHeadRegion()
{
  vpProject* project = this->findProject(this->TrackEditProjectId);
  if (!project)
    {
    return;
    }

  vtkVgTrack* track = project->TrackModel->GetTrack(this->EditingTrackId);

  vtkIdType npts, *ptIds, trackPointId;
  track->GetHeadIdentifier(project->TrackModel->GetCurrentTimeStamp(),
                           npts, ptIds, trackPointId);
  if (npts < 4)
    {
    if (this->TrackHeadRegion)
      {
      this->TrackHeadRegion->SetVisible(0);
      }
    return;
    }

  bool usePolygonEditor = false;
  switch (this->RegionEditMode)
    {
    case REM_Auto:
      // Use the contour editor if the current head isn't a box
      if (npts != 5)
        {
        usePolygonEditor = true;
        }
      else
        {
        double ul[3];
        double ll[3];
        double lr[3];
        double ur[3];
        vtkPoints* trackPts = track->GetPoints();
        trackPts->GetPoint(ptIds[0], ul);
        trackPts->GetPoint(ptIds[1], ll);
        trackPts->GetPoint(ptIds[2], lr);
        trackPts->GetPoint(ptIds[3], ur);

        usePolygonEditor =
          ul[0] != ll[0] ||
          ll[1] != lr[1] ||
          lr[0] != ur[0] ||
          ur[1] != ul[1];
        }
      break;

    case REM_Polygon:
      usePolygonEditor = true;
      break;
    }

  // Lazily create the contour on first use since it's an expensive operation
  if (usePolygonEditor)
    {
    if (!this->TrackHeadContour)
      {
      this->TrackHeadContour =
        new vpContour(this->RenderWindow->GetInteractor());

      this->VTKConnect->Connect(this->TrackHeadContour->GetWidget(),
                                vtkCommand::EndInteractionEvent,
                                this, SLOT(onEditTrackHeadRegion()));
      }
    this->TrackHeadRegion = this->TrackHeadContour;
    if (this->TrackHeadBox)
      {
      this->TrackHeadBox->SetVisible(0);
      }
    }
  else
    {
    if (!this->TrackHeadBox)
      {
      this->TrackHeadBox =
        new vpBox(this->RenderWindow->GetInteractor());

      this->VTKConnect->Connect(this->TrackHeadBox->GetWidget(),
                                vtkCommand::EndInteractionEvent,
                                this, SLOT(onEditTrackHeadRegion()));
      }
    this->TrackHeadRegion = this->TrackHeadBox;
    if (this->TrackHeadContour)
      {
      this->TrackHeadContour->SetVisible(0);
      }
    }

  if (!this->TrackHeadRegionPolyData)
    {
    this->TrackHeadRegionPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkCellArray* ca = vtkCellArray::New();
    vtkPoints* pts = vtkPoints::New();
    this->TrackHeadRegionPolyData->SetPoints(pts);
    this->TrackHeadRegionPolyData->SetLines(ca);
    pts->FastDelete();
    ca->FastDelete();
    }

  // Rebuild the polydata
  vtkPoints* pts = this->TrackHeadRegionPolyData->GetPoints();
  pts->SetNumberOfPoints(npts - 1);

  vtkMatrix4x4* trackToWorld =
    project->TrackHeadRepresentation->GetRepresentationMatrix();

  vtkCellArray* ca = this->TrackHeadRegionPolyData->GetLines();
  ca->Reset();
  ca->InsertNextCell(npts);
  for (int i = 0; i < npts - 1; ++i)
    {
    double point[4];
    track->GetPoints()->GetPoint(ptIds[i], point);
    point[2] = 0.0;
    point[3] = 1.0;

    // Transform to world point
    trackToWorld->MultiplyPoint(point, point);

    ca->InsertCellPoint(i);
    pts->SetPoint(i, point);
    }
  ca->InsertCellPoint(0);

  this->TrackHeadRegionPolyData->Modified();

  this->TrackHeadRegion->Initialize(this->TrackHeadRegionPolyData);
  this->TrackHeadRegion->SetVisible(1);

  // Show interpolated heads in a slightly darker color
  if (!project->TrackModel->GetIsKeyframe(this->EditingTrackId,
                                          this->CoreTimeStamp))
    {
    this->TrackHeadRegion->SetLineColor(0.0, 0.6, 0.6);
    }
  else
    {
    this->TrackHeadRegion->SetLineColor(0.0, 1.0, 1.0);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::updateTrackFollowCamera()
{
  if (vpProject* project = this->findProject(this->FollowTrackProjectId))
    {
    vtkVgTrack* trackToFollow =
      project->TrackModel->GetTrack(this->IdOfTrackToFollow);
    if (trackToFollow && trackToFollow->GetPointIds()->GetNumberOfIds() > 0)
      {
      double center[4];
      trackToFollow->GetClosestFramePt(this->CoreTimeStamp, center);
      center[2] = 0.0;
      center[3] = 1.0;
      project->TrackRepresentation->GetRepresentationMatrix()->MultiplyPoint(
        center, center);
      center[0] /= center[3];
      center[1] /= center[3];
      this->moveCameraTo(center, false);
      }
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::mouseLeftViewport()
{
  this->HideTrackHeadIndicator = true;
  if (this->isEditingTrack())
    {
    this->TrackHeadIndicatorActor->SetVisibility(0);
    this->render();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::mouseEnteredViewport()
{
  this->HideTrackHeadIndicator = false;
  if (this->isEditingTrack())
    {
    int x, y;
    this->getMousePosition(&x, &y);
    this->updateTrackHeadIndicator(x, y);
    this->render();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::setTrackHead(vtkPoints* points)
{
  vpProject* project = this->findProject(this->TrackEditProjectId);
  if (!project)
    {
    return;
    }

  double bounds[6];
  points->GetBounds(bounds);
  double worldCenter[2];
  worldCenter[0] = 0.5 * (bounds[0] + bounds[1]);
  worldCenter[1] = 0.5 * (bounds[2] + bounds[3]);

  // Incoming points are in world coordinates. In raw image mode and geo mode,
  // world space and track space are different coordinate systems, so transform
  // back now.
  if (this->UseRawImageCoordinates || this->UseGeoCoordinates)
    {
    const vtkMatrix4x4* trackToWorld =
      project->TrackHeadRepresentation->GetRepresentationMatrix();

    vtkSmartPointer<vtkMatrix4x4> worldToTrack =
      vtkSmartPointer<vtkMatrix4x4>::New();
    vtkMatrix4x4::Invert(trackToWorld, worldToTrack);

    for (vtkIdType i = 0, size = points->GetNumberOfPoints(); i < size; ++i)
      {
      double point[4];
      points->GetPoint(i, point);
      point[2] = 0.0;
      point[3] = 1.0;
      worldToTrack->MultiplyPoint(point, point);
      points->SetPoint(i, point);
      }
    points->Modified();
    }

  project->TrackModel->AddKeyframe(this->EditingTrackId, this->CoreTimeStamp);

  vtkVgTrack* track = project->TrackModel->GetTrack(this->EditingTrackId);

  points->GetBounds(bounds);

  double center[2];
  if (this->UseGeoCoordinates)
    {
    // the track point is stored in world coordinates while head is in
    // image coordinates
    center[0] = worldCenter[0];
    center[1] = worldCenter[1];
    }
  else
    {
    center[0] = 0.5 * (bounds[0] + bounds[1]);
    center[1] = 0.5 * (bounds[2] + bounds[3]);
    }

  vtkVgGeoCoord geoCoord;
  if (this->UseRawImageCoordinates || this->UseGeoCoordinates)
    {
    geoCoord = this->worldToGeo(worldCenter);
    }

  track->SetPoint(this->CoreTimeStamp, center, geoCoord,
    points->GetNumberOfPoints(), 0, points, 0);

  project->TrackModel->Modified();

  // remove dummy point if this is first user-created point, and show the track
  if (this->EditingTrackId == this->NewTrackId)
    {
    if (this->CoreTimeStamp != this->NewTrackTimeStamp)
      {
      track->DeletePoint(this->NewTrackTimeStamp);
      }

    this->NewTrackId = -1;
    this->NewTrackTimeStamp = vtkVgTimeStamp();
    project->TrackRepresentation->SetExcludedTrack(0);
    project->TrackHeadRepresentation->SetExcludedTrack(0);
    project->SceneElementRepresentation->SetExcludedTrack(0);
    }

  emit this->objectInfoUpdateNeeded();
}

//-----------------------------------------------------------------------------
void vpViewCore::setTrackHeadAndAdvance(vtkPoints* points)
{
  vpProject* project = this->findProject(this->TrackEditProjectId);
  if (!project)
    {
    return;
    }

  bool isNewTrack = this->EditingTrackId == this->NewTrackId;
  vtkVgTrack* track = project->TrackModel->GetTrack(this->EditingTrackId);
  vtkVgTimeStamp sf = track->GetStartFrame();
  vtkVgTimeStamp ef = track->GetEndFrame();

  this->setTrackHead(points);

  // If the newly added head is past the end of the track, or this is a new
  // track with no points, advance to the next frame. If the head is inserted
  // before the start of the track, rewind to the previous frame. Otherwise
  // stay put on the current frame.
  if (this->AutoAdvanceDuringCreation && (isNewTrack || track->GetEndFrame() > ef))
    {
    this->nextFrame();
    }
  else if (this->AutoAdvanceDuringCreation && track->GetStartFrame() < sf)
    {
    this->prevFrame();
    }
  else
    {
    this->update();
    }
}

//-----------------------------------------------------------------------------
vtkVgBaseImageSource* vpViewCore::getImageSource()
{
  return this->ImageSource;
}

//-----------------------------------------------------------------------------
void vpViewCore::setImageSourceLevelOfDetailFactor(double factor)
{
  if (this->ImageSource)
    {
    this->ImageSourceLODFactor = factor;

    this->updateAOIImagery();
    }
}

//-----------------------------------------------------------------------------
bool vpViewCore::hasMultiLevelOfDetailSource()
{
  if (this->ImageSource->GetNumberOfLevels() > 1)
    {
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
vtkVpTrackModel* vpViewCore::getTrackModel(int session)
{
  return this->Projects[session]->TrackModel;
}

//-----------------------------------------------------------------------------
vtkVgEventModel* vpViewCore::getEventModel(int session)
{
  return this->Projects[session]->EventModel;
}

//-----------------------------------------------------------------------------
vtkVgEventFilter* vpViewCore::getEventFilter()
{
  return this->EventFilter;
}

//-----------------------------------------------------------------------------
void vpViewCore::setTrackTrailLength(const vtkVgTimeStamp& duration)
{
  this->TrackTrailLength = duration;
  for (size_t i = 0, size = this->Projects.size(); i < size; ++i)
    {
    this->setTrackTrailLength(this->Projects[i], duration);
    }
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vpViewCore::getTrackTrailLength()
{
  return this->TrackTrailLength;
}

//-----------------------------------------------------------------------------
void vpViewCore::setSceneElementLineWidth(double lineWidth)
{
  if (this->SceneElementLineWidth == lineWidth)
    {
    return;
    }
  this->SceneElementLineWidth = lineWidth;
  for (size_t i = 0, size = this->Projects.size(); i < size; ++i)
    {
    this->setSceneElementLineWidth(this->Projects[i], lineWidth);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::setUseTimeBasedIndexing(bool state)
{
  if (state == this->UseTimeBasedIndexing)
    {
    return;
    }

  this->UseTimeBasedIndexing = state;
  if (this->UseTimeBasedIndexing && this->UsingTimeStampData)
    {
    this->UsingTimeBasedIndexing = true;
    this->waitForFrameMapRebuild();
    }
  else
    {
    this->UsingTimeBasedIndexing = false;
    }

  if (!this->Projects.empty())
    {
    this->syncAnimationToCore();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::setEnableWorldDisplayIfAvailable(bool state)
{
  this->EnableWorldDisplayIfAvailable = state;
}

//-----------------------------------------------------------------------------
void vpViewCore::setEnableTranslateImage(bool state)
{
  this->EnableTranslateImage = state;
}

//-----------------------------------------------------------------------------
void vpViewCore::deleteTrackPoint()
{
  if (this->isEditingTrack())
    {
    vpProject* project = this->findProject(this->TrackEditProjectId);
    if (!project)
      {
      return;
      }
    project->TrackModel->RemoveKeyframe(this->EditingTrackId,
                                        this->CoreTimeStamp);
    vtkVgTrack* track = project->TrackModel->GetTrack(this->EditingTrackId);
    track->DeletePoint(this->CoreTimeStamp);
    project->TrackModel->Modified();
    emit this->objectInfoUpdateNeeded();
    this->updateScene();
    emit this->showStatusMessage(
      QString("Deleted frame %1 of track %2")
      .arg(this->CoreTimeStamp.GetFrameNumber())
      .arg(this->EditingTrackId), 3000);
    }
  else
    {
    emit this->showStatusMessage("Track not in edit mode", 3000);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::setRegionEditMode(int mode)
{
  this->RegionEditMode = mode;
  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::setExternalExecuteMode(int mode)
{
  this->ExternalExecuteMode = mode;
}

//-----------------------------------------------------------------------------
void vpViewCore::setTrackColorMode(int mode, const QString& attributeGroup)
{
  QSettings settings;

  switch (mode)
    {
    case vtkVgTrackRepresentationBase::TCM_StateAttrs:
      {
      std::vector<vgAttribute> attrs =
        this->TrackAttributes.GetAttributes(stdString(attributeGroup));

      // Create LUT
      vtkSmartPointer<vtkLookupTable> lookupTable =
        vtkSmartPointer<vtkLookupTable>::New();
      lookupTable->SetNumberOfTableValues(
        static_cast<vtkIdType>(attrs.size() + 1));

      // first entry is used for 'unknown' attribute combinations
      lookupTable->SetTableValue(0, 1.0, 1.0, 1.0);

      // read attribute colors from settings
      settings.beginGroup("TrackAttributeColors");
      for (size_t i = 0, size = attrs.size(); i < size; ++i)
        {
        vgColor color(Qt::white);
        color.read(settings,
                   attributeGroup + QChar('/') + qtString(attrs[i].Name));
        lookupTable->SetTableValue(static_cast<vtkIdType>(i + 1),
                                   color.data().array);
        }

      // set up a 1-1 mapping
      lookupTable->SetTableRange(0.0, static_cast<double>(attrs.size()));

      // Set the LUT on each track rep
      for (size_t i = 0, size = this->Projects.size(); i < size; ++i)
        {
        vtkVgTrackRepresentation* rep = this->Projects[i]->TrackRepresentation;
        rep->SetLookupTable(lookupTable);
        rep->ClearStateAttributeMasks();
        for (size_t j = 0, size = attrs.size(); j < size; ++j)
          {
          rep->AddStateAttributeMask(attrs[j].Mask);
          }
        rep->SetStateAttributeGroupMask(
          this->TrackAttributes.GetGroupMask(stdString(attributeGroup)));

        this->Projects[i]->TrackModel->SetActiveScalars("StateAttributes");
        }

      emit this->showStatusMessage(
        QString("Coloring tracks by \'%1\' attribute").arg(attributeGroup),
        3000);

      break;
      }
    }

  this->CurrentTrackColorMode = mode;
  this->CurrentTrackAttributeGroup = attributeGroup;
  vtkVgTrackRepresentationBase::TrackColorMode trackColorMode =
    static_cast<vtkVgTrackRepresentationBase::TrackColorMode>(mode);
  for (size_t i = 0, size = this->Projects.size(); i < size; ++i)
    {
    this->Projects[i]->TrackRepresentation->SetColorMode(trackColorMode);
    this->Projects[i]->TrackHeadRepresentation->SetColorMode(trackColorMode);
    }

  this->updateScene();
}

//-----------------------------------------------------------------------------
void vpViewCore::previousTrackAttributeGroup()
{
  std::vector<std::string> groups = this->TrackAttributes.GetEnabledGroups();
  if (groups.empty())
    {
    return;
    }

  std::string currentGroup = stdString(this->CurrentTrackAttributeGroup);
  for (size_t i = 0, size = groups.size(); i < size; ++i)
    {
    if (currentGroup == groups[i])
      {
      this->setTrackColorMode(vtkVgTrackRepresentationBase::TCM_StateAttrs,
                              qtString(groups[i == 0 ? size - 1 : i - 1]));
      return;
      }
    }

  this->setTrackColorMode(vtkVgTrackRepresentationBase::TCM_StateAttrs,
                          qtString(groups.front()));
}

//-----------------------------------------------------------------------------
void vpViewCore::nextTrackAttributeGroup()
{
  std::vector<std::string> groups = this->TrackAttributes.GetEnabledGroups();
  if (groups.empty())
    {
    return;
    }

  std::string currentGroup = stdString(this->CurrentTrackAttributeGroup);
  for (size_t i = 0, size = groups.size(); i < size; ++i)
    {
    if (currentGroup == groups[i])
      {
      this->setTrackColorMode(vtkVgTrackRepresentationBase::TCM_StateAttrs,
                              qtString(groups[(i + 1) % size]));
      return;
      }
    }

  this->setTrackColorMode(vtkVgTrackRepresentationBase::TCM_StateAttrs,
                          qtString(groups.front()));
}

//-----------------------------------------------------------------------------
void vpViewCore::setStreamingEnabled(bool enable)
{
  this->ImageDataSource->setMonitoringEnabled(enable);
}

//-----------------------------------------------------------------------------
void vpViewCore::worldToImage(double in[2], int out[2])
{
  double p[] = { in[0], in[1], 0.0, 1.0 };
  this->WorldToImageMatrix->MultiplyPoint(p, p);
  p[0] /= p[3];
  p[1] /= p[3];

  out[0] = vtkMath::Floor(p[0] + 0.5);
  out[1] = vtkMath::Floor(p[1] + 0.5);
}

//-----------------------------------------------------------------------------
vtkVgGeoCoord vpViewCore::worldToGeo(double in[2])
{
  if (!(this->UseGeoCoordinates || this->UseRawImageCoordinates))
    {
    return vtkVgGeoCoord();
    }

  vtkSmartPointer<vtkMatrix4x4> worldToLatLon = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkMatrix4x4::Invert(this->LatLonToWorldMatrix, worldToLatLon);
  double point[] = { in[0], in[1], 0.0, 1.0 };
  worldToLatLon->MultiplyPoint(point, point);
  point[0] /= point[3];
  point[1] /= point[3];
  return vtkVgGeoCoord(point[1], point[0]);
}

//-----------------------------------------------------------------------------
bool vpViewCore::displayToImage(int in[2], int out[2])
{
  double p[4];
  this->SceneRenderer->SetDisplayPoint(in[0], in[1], 0);
  this->SceneRenderer->DisplayToWorld();
  this->SceneRenderer->GetWorldPoint(p);

  this->worldToImage(p, out);

  bool inside =
    out[0] >= this->WholeImageBounds[0] &&
    out[0] <= this->WholeImageBounds[1] &&
    out[1] >= this->WholeImageBounds[2] &&
    out[1] <= this->WholeImageBounds[3];

  out[0] -= this->WholeImageBounds[0];
  out[1] -= this->WholeImageBounds[2];

  out[1] = this->WholeImageBounds[3] - this->WholeImageBounds[2] - out[1];

  return inside;
}

//-----------------------------------------------------------------------------
bool vpViewCore::displayToAOI(int in[2], int out[2])
{
  double p[4];
  this->SceneRenderer->SetDisplayPoint(in[0], in[1], 0);
  this->SceneRenderer->DisplayToWorld();
  this->SceneRenderer->GetWorldPoint(p);
  p[2] = 0.0;

  vtkPoints* points = this->AOIOutlinePolyData->GetPoints();

  double ll[3], lr[3], ur[3], ul[3];
  points->GetPoint(0, ll);
  points->GetPoint(1, lr);
  points->GetPoint(2, ur);
  points->GetPoint(3, ul);

  double bounds[6];
  this->AOIOutlinePolyData->GetBounds(bounds);

  if (lr[0] == ul[0] && lr[1] == ul[1])
    {
    // Only have upper-left corner.
    if (p[0] < ul[0] || p[1] > ul[1])
      {
      return false;
      }
    }
  else
    {
    if (ll[0] != ul[0] ||
        lr[0] != ur[0] ||
        ll[1] != lr[1] ||
        ur[1] != ul[1])
      {
      // Non-rectangular AOI
      double normal[] = { 0, 0, 1 };
      vtkDoubleArray* ptArray = static_cast<vtkDoubleArray*>(points->GetData());
      if (vtkPolygon::PointInPolygon(p, 4, ptArray->GetPointer(0),
                                     bounds, normal) < 1)
        {
        return false;
        }
      }
    else if (p[0] < bounds[0] || p[0] > bounds[1] ||
             p[1] < bounds[2] || p[1] > bounds[3])
      {
      return false;
      }
    }

  this->worldToImage(p, out);

  double uli[4] = { ul[0], ul[1], 0.0, 1.0 };
  this->WorldToImageMatrix->MultiplyPoint(uli, uli);
  uli[0] /= uli[3];
  uli[1] /= uli[3];

  double lri[4] = { lr[0], lr[1], 0.0, 1.0 };
  this->WorldToImageMatrix->MultiplyPoint(lri, lri);
  lri[0] /= lri[3];
  lri[1] /= lri[3];

  // Coordinates are relative to the upper left corner.
  out[0] = vtkMath::Round(out[0] - uli[0]);
  out[1] = vtkMath::Round(uli[1] - out[1]);
  return true;
}

//-----------------------------------------------------------------------------
bool vpViewCore::displayToGeo(int in[2], double& northing, double& easting)
{
  if (!(this->UseGeoCoordinates || this->UseRawImageCoordinates ||
        this->ImageToGcsMatrix))
    {
    return false;
    }

  double p[4];
  this->SceneRenderer->SetDisplayPoint(in[0], in[1], 0);
  this->SceneRenderer->DisplayToWorld();
  this->SceneRenderer->GetWorldPoint(p);

  if (!this->ImageToGcsMatrix)
    {
    vtkSmartPointer<vtkMatrix4x4> worldToLatLon = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkMatrix4x4::Invert(this->LatLonToWorldMatrix, worldToLatLon);
    worldToLatLon->MultiplyPoint(p, p);
    }
  else
    {
    this->ImageToGcsMatrix->MultiplyPoint(p, p);
    }
  northing = p[1] / p[3];
  easting = p[0] / p[3];
  return true;
}

//-----------------------------------------------------------------------------
bool vpViewCore::getGsd(int displayPoint[2],
                        double& northingDist, double& eastingDist,
                        double& width, double& height)
{
  if (!(this->UseGeoCoordinates || this->UseRawImageCoordinates ||
        this->ImageToGcsMatrix))
    {
    return false;
    }

  double p[4];
  this->SceneRenderer->SetDisplayPoint(displayPoint[0], displayPoint[1], 0);
  this->SceneRenderer->DisplayToWorld();
  this->SceneRenderer->GetWorldPoint(p);

  this->WorldToImageMatrix->MultiplyPoint(p, p);
  p[0] /= p[3];
  p[1] /= p[3];

  // We will be measuring the distances across the center of the pixel, from
  // left to right then from bottom to top.
  double leftMid[] =
    {
    floor(p[0]), floor(p[1]) + 0.5, 0.0, 1.0
    };
  double rightMid[] =
    {
    leftMid[0] + 1.0, leftMid[1], 0.0, 1.0
    };
  double bottomMid[] =
    {
    floor(p[0]) + 0.5, floor(p[1]), 0.0, 1.0
    };
  double topMid[] =
    {
    bottomMid[0], bottomMid[1] + 1.0, 0.0, 1.0
    };

  double dummy;
  width = this->getGeoDistance(leftMid, rightMid, dummy, eastingDist);
  height = this->getGeoDistance(bottomMid, topMid, northingDist, dummy);
  return true;
}

//-----------------------------------------------------------------------------
bool vpViewCore::getGsd(double& northingDistPerPixel, double& eastingDistPerPixel,
                        double& widthPerPixel, double& heightPerPixel)
{
  if (!(this->UseGeoCoordinates || this->UseRawImageCoordinates || this->ImageToGcsMatrix))
    {
    return false;
    }

  double leftMid[] =
    {
    this->WholeImageBounds[0] - 0.5,
    0.5 * (this->WholeImageBounds[2] + this->WholeImageBounds[3]),
    0.0,
    1.0
    };

  double rightMid[] =
    {
    this->WholeImageBounds[1] + 0.5,
    leftMid[1],
    0.0,
    1.0
    };

  double bottomMid[] =
    {
    0.5 * (this->WholeImageBounds[0] + this->WholeImageBounds[1]),
    this->WholeImageBounds[2] - 0.5,
    0.0,
    1.0
    };

  double topMid[] =
    {
    bottomMid[0],
    this->WholeImageBounds[3] + 0.5,
    0.0,
    1.0
    };

  double dummy;
  double northingDist, eastingDist;
  double width = this->getGeoDistance(leftMid, rightMid, dummy, eastingDist);
  double height = this->getGeoDistance(bottomMid, topMid, northingDist, dummy);

  double imageWidth = this->WholeImageBounds[1] - this->WholeImageBounds[0] + 1.0;
  double imageHeight = this->WholeImageBounds[3] - this->WholeImageBounds[2] + 1.0;

  northingDistPerPixel = std::abs(northingDist) / imageHeight;
  eastingDistPerPixel = std::abs(eastingDist) / imageWidth;

  widthPerPixel = width / imageWidth;
  heightPerPixel = height / imageHeight;
  return true;
}

//-----------------------------------------------------------------------------
double vpViewCore::getGeoDistance(double imagePt1[4], double imagePt2[4],
                                  double& northingDist, double& eastingDist)
{
  // Currently the assumption is that imageToGcs will be in UTM
  vtkSmartPointer<vtkMatrix4x4> imageToGcs = this->ImageToGcsMatrix;

  if (!imageToGcs)
    {
    imageToGcs = this->ImageSource->GetImageToWorldMatrix();
    }

  imageToGcs->MultiplyPoint(imagePt1, imagePt1);
  double northing1 = imagePt1[1] / imagePt1[3];
  double easting1 = imagePt1[0] / imagePt1[3];

  imageToGcs->MultiplyPoint(imagePt2, imagePt2);
  double northing2 = imagePt2[1] / imagePt2[3];
  double easting2 = imagePt2[0] / imagePt2[3];

  northingDist = std::abs(northing2 - northing1);
  eastingDist = std::abs(easting2 - easting1);

  double distance;
  if (this->ImageToGcsMatrix)
    {
    distance = sqrt(northingDist * northingDist + eastingDist * eastingDist);
    }
  else
    {
    GeographicLib::Geodesic::WGS84.Inverse(
      northing1, easting1, northing2, easting2, distance);
    }

  return distance;
}

//-----------------------------------------------------------------------------
void vpViewCore::setRealTimeVideoPlayback(bool enable)
{
  this->VideoAnimation->setPlaybackMode(
    enable ? vpVideoAnimation::PM_RealTime : vpVideoAnimation::PM_Sequential);
}

//-----------------------------------------------------------------------------
void vpViewCore::setRulerEnabled(bool enable)
{
  this->RulerEnabled = enable;
  if (!enable)
    {
    this->RubberbandInteractorStyle->SetRubberBandModeToZoom();
    this->RulerActor->SetVisibility(0);
    this->render();
    }
}

//-----------------------------------------------------------------------------
bool vpViewCore::updateImageMatrices()
{
  vtkSmartPointer<vtkMatrix4x4> imageToLatLon =
    this->ImageSource->GetImageToWorldMatrix();

  if (!imageToLatLon)
    {
    this->ImageToWorldMatrix->Identity();
    this->WorldToImageMatrix->Identity();
    this->LatLonToImageMatrix->Identity();
    return true;
    }

  vtkMatrix4x4::Invert(imageToLatLon, this->LatLonToImageMatrix);

  if (this->UseGeoCoordinates)
    {
    vtkMatrix4x4::Multiply4x4(this->LatLonToWorldMatrix, imageToLatLon,
                              this->ImageToWorldMatrix);
    vtkMatrix4x4::Invert(this->ImageToWorldMatrix, this->WorldToImageMatrix);
    return true;
    }

  if (this->UseRawImageCoordinates)
    {
    double refPt[4] = { this->ImageTranslationReferenceLatLon[1],
                        this->ImageTranslationReferenceLatLon[0], 0, 1.0 };
    this->LatLonToImageMatrix->MultiplyPoint(refPt, refPt);

    if (refPt[3] != 0)
      {
      // Store the computed offset for later reference.
      this->ImageTranslationOffset[0] = refPt[0] / refPt[3];
      this->ImageTranslationOffset[1] = refPt[1] / refPt[3];

      // The reference point will be positioned at the world origin.
      this->ImageToWorldMatrix->Identity();
      this->ImageToWorldMatrix->SetElement(0, 3, -this->ImageTranslationOffset[0]);
      this->ImageToWorldMatrix->SetElement(1, 3, -this->ImageTranslationOffset[1]);
      vtkMatrix4x4::Invert(this->ImageToWorldMatrix, this->WorldToImageMatrix);
      return true;
      }

    // Bad image to lat-lon matrix?
    return false;
    }

  this->ImageToWorldMatrix->Identity();
  this->WorldToImageMatrix->Identity();
  return true;
}

//-----------------------------------------------------------------------------
void vpViewCore::setEventExpirationMode(enumEventExpirationMode mode,
                                        bool render)
{
  this->EventExpirationMode = mode;

  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    this->updateEventDisplayEndFrame(this->Projects[i]);
    }

  if (render)
    {
    this->updateScene();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::updateEventDisplayEndFrame(vpProject* project)
{
  switch (this->EventExpirationMode)
    {
    case ShowUntilEventEnd:
      project->EventModel->ShowEventsAfterExpirationOff();
      project->EventModel->ShowEventsUntilSupportingTracksExpireOff();
      break;

    case ShowUntilTrackEnd:
      project->EventModel->ShowEventsAfterExpirationOn();
      project->EventModel->ShowEventsUntilSupportingTracksExpireOn();
      break;
    }
}
//-----------------------------------------------------------------------------
void vpViewCore::getMousePosition(int* x, int* y)
{
  if (this->RenderWidget && this->RenderWindow)
    {
    QPoint pos = this->RenderWidget->mapFromGlobal(QCursor::pos());
    *x = pos.x();
    *y = this->RenderWindow->GetInteractor()->GetSize()[1] - pos.y() - 1;
    }
  else
    {
    *x = *y = 0;
    }
}

//-----------------------------------------------------------------------------
vpProject* vpViewCore::findProject(int projectId)
{
  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    if (this->Projects[i]->ProjectId == projectId)
      {
      return this->Projects[i];
      }
    }

  return 0;
}

//-----------------------------------------------------------------------------
void vpViewCore::setObjectExpirationTime(const vtkVgTimeStamp& time)
{
  if (time.GetFrameNumber() == this->ObjectExpirationTime.GetFrameNumber() &&
      time.GetTime() == this->ObjectExpirationTime.GetTime())
    {
    return;
    }

  bool timeIsValid = time.IsValid();
  this->ObjectExpirationTime = time;

  for (size_t i = 0, end = this->Projects.size(); i < end; ++i)
    {
    vpProject* project = this->Projects[i];
    if (!timeIsValid)
      {
      vtkVgTimeStamp ts;
      ts.SetFrameNumber(DefaultTrackExpiration);
      ts.SetTime(1e6 * (DefaultTrackExpiration / this->RequestedFPS));
      project->TrackModel->SetTrackExpirationOffset(ts);
      if (this->DisplayFullVolume)
        {
        project->TrackModel->ShowTracksBeforeStartOn();
        project->TrackModel->ShowTracksAfterExpirationOn();
        project->EventModel->ShowEventsBeforeStartOn();
        project->EventModel->ShowEventsAfterExpirationOn();
        }
      else
        {
        project->TrackModel->ShowTracksBeforeStartOff();
        project->TrackModel->ShowTracksAfterExpirationOff();
        project->EventModel->ShowEventsBeforeStartOff();
        this->updateEventDisplayEndFrame(project);
        }
      ts.SetFrameNumber(0);
      project->ActivityManager->SetActivityExpirationOffset(ts);
      project->ActivityManager->SetShowFullVolume(this->DisplayFullVolume);
      }
    else
      {
      project->TrackModel->SetTrackExpirationOffset(time);
      project->TrackModel->ShowTracksBeforeStartOff();
      project->TrackModel->ShowTracksAfterExpirationOff();

      project->EventModel->SetEventExpirationOffset(time);
      project->EventModel->ShowEventsBeforeStartOff();
      project->EventModel->ShowEventsAfterExpirationOff();

      project->ActivityManager->SetActivityExpirationOffset(time);
      project->ActivityManager->SetShowFullVolume(false);
      }
    }
}

//-----------------------------------------------------------------------------
double vpViewCore::getMinimumTime()
{
  if (this->UsingTimeBasedIndexing)
    {
    vpFrame frame;
    if (!this->FrameMap->first(frame))
      {
      return vgTimeStamp::InvalidTime();
      }
    return frame.Time.GetTime();
    }
  return 1e6 * (this->FrameNumberOffset / this->RequestedFPS);
}

//-----------------------------------------------------------------------------
double vpViewCore::getMaximumTime()
{
  if (this->UsingTimeBasedIndexing)
    {
    vpFrame frame;
    if (!this->FrameMap->last(frame))
      {
      return vgTimeStamp::InvalidTime();
      }
    return frame.Time.GetTime();
    }
  return 1e6 * ((this->FrameNumberOffset + this->NumberOfFrames - 1) /
                this->RequestedFPS);
}

//-----------------------------------------------------------------------------
double vpViewCore::getCurrentTime()
{
  if (this->UsingTimeBasedIndexing)
    {
    return this->CoreTimeStamp.GetTime();
    }

  return 1e6 * (this->CoreTimeStamp.GetFrameNumber() / this->RequestedFPS);
}

//-----------------------------------------------------------------------------
void vpViewCore::setCurrentTime(const vtkVgTimeStamp& timeStamp)
{
  if (!this->UsingTimeBasedIndexing)
    {
    vtkVgTimeStamp ts(1e6 * timeStamp.GetFrameNumber() / this->RequestedFPS);
    this->seekInternal(ts);
    return;
    }

  this->seekInternal(timeStamp);
}

//-----------------------------------------------------------------------------
double vpViewCore::getCurrentFrameTime()
{
  if (this->UsingTimeStampData)
    {
    return this->CoreTimeStamp.GetTime();
    }

  return 1e6 * (this->CoreTimeStamp.GetFrameNumber() / this->RequestedFPS);
}

//-----------------------------------------------------------------------------
void vpViewCore::startFrameMapRebuild()
{
  this->FrameMap->stop();
  this->FrameMap->startUpdate();
}

//-----------------------------------------------------------------------------
bool vpViewCore::waitForFrameMapRebuild()
{
  int numFiles = this->ImageDataSource->getFileCount();
  if (!this->FrameMap->isRunning() && this->FrameMap->progress() < numFiles)
    {
    // This shouldn't ever happen.
    qDebug() << "Trying to wait for frame map build thread to complete, but "
                "the thread has not been started. Restarting...";
    this->FrameMap->startUpdate();
    }

  QProgressDialog progress("Decoding image timestamps...", "Cancel",
                           0, numFiles);
  progress.setWindowModality(Qt::ApplicationModal);

  // Manually force the dialog to be shown. Otherwise, in release builds a
  // strange bug can occur where the dialog never appears, and allows the user
  // to continue interacting with the UI.
  progress.show();

  int curProgress;
  while ((curProgress = this->FrameMap->progress()) < numFiles)
    {
    progress.setValue(curProgress);

    QApplication::sendPostedEvents();
    QApplication::processEvents();

    if (progress.wasCanceled())
      {
      // Stop using time based indexing for now, but continue to update the
      // frame map in the background.
      emit this->timeBasedIndexingDisabled();
      this->setUseTimeBasedIndexing(false);
      return false;
      }
    }
  progress.setValue(numFiles);
  return true;
}

//-----------------------------------------------------------------------------
void vpViewCore::setCurrentFrame(unsigned int frameIndex, double currentTime)
{
  Q_ASSERT(!this->UsingTimeBasedIndexing);

  vtkVgTimeStamp ts;
  ts.SetFrameNumber(frameIndex + this->FrameNumberOffset);

  if (frameIndex != this->CurrentFrame || this->CoreTimeStamp != ts)
    {
    this->CoreTimeStamp = ts;
    this->CurrentFrame = frameIndex;
    this->updateScene();
    emit this->frameChanged();
    }

  if (currentTime == -1.0)
    {
    currentTime = this->getCurrentTime();
    }
  if (currentTime != this->CurrentTime)
    {
    this->CurrentTime = currentTime;
    emit this->timeChanged(currentTime);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::setCurrentFrame(const vpFrame& frame, double currentTime)
{
  if (this->CurrentFrame != frame.Index || this->CoreTimeStamp != frame.Time)
    {
    this->CoreTimeStamp = frame.Time;
    this->CurrentFrame = frame.Index;
    this->updateScene();
    emit this->frameChanged();
    }

  if (currentTime == -1.0)
    {
    currentTime = this->getCurrentTime();
    }
  else
    {
    currentTime = qBound(this->getMinimumTime(), currentTime,
                         this->getMaximumTime());
    }
  if (currentTime != this->CurrentTime)
    {
    this->CurrentTime = currentTime;
    emit this->timeChanged(currentTime);
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::setTrackTrailLength(vpProject* project,
                                     vtkVgTimeStamp duration)
{
  // Use only the appropriate part of the timestamp for the current mode.
  if (this->UsingTimeStampData)
    {
    duration.SetFrameNumber(vgTimeStamp::InvalidFrameNumber());
    }
  else
    {
    duration.SetTime(vgTimeStamp::InvalidTime());
    }
  project->TrackModel->SetMaximumDisplayDuration(duration);
}

//-----------------------------------------------------------------------------
void vpViewCore::setSceneElementLineWidth(vpProject* project,
                                          double lineWidth)
{
  project->SceneElementRepresentation->SetLineWidth(lineWidth);
  project->SelectedSceneElementRepresentation->SetLineWidth(lineWidth);
}

//-----------------------------------------------------------------------------
vtkVgTrack* vpViewCore::makeTrack(int id, int session)
{
  vtkVgTrackModel* trackModel = this->Projects[session]->TrackModel;
  vtkVgTrack* track = vtkVgTrack::New();
  track->InterpolateMissingPointsOnInsertOn();
  track->SetId(id);
  track->SetPoints(trackModel->GetPoints());
  track->SetUserCreated(true);

  double color[3];
  vpTrackIO::GetDefaultTrackColor(id, color);
  track->SetColor(color);

  trackModel->AddTrack(track);
  track->FastDelete();
  return track;
}

//-----------------------------------------------------------------------------
void vpViewCore::setFrameNumberOffset(int offset)
{
  this->FrameNumberOffset = offset;
  this->syncAnimationToCore();
}

//-----------------------------------------------------------------------------
void vpViewCore::syncAnimationToCore()
{
  this->VideoAnimation->setFrameRange(
    vtkVgTimeStamp(this->getMinimumTime()),
    vtkVgTimeStamp(this->getMaximumTime()));
  this->seekInternal(vtkVgTimeStamp(this->getCurrentTime()));
}

//-----------------------------------------------------------------------------
vpContour* vpViewCore::makeFilterContour()
{
  vpContour* contour = new vpContour(this->RenderWindow->GetInteractor());

  QSettings settings;

  vgColor lineColor =
    vgColor::read(settings, "FilterLineColor",
                  QColor::fromRgbF(1.0, 1.0, 1.0));

  vgColor finalColor =
    vgColor::read(settings, "FilterFinalColor",
                  QColor::fromRgbF(0.8, 0.8, 0.8));

  contour->SetLineColor(lineColor.data().color.red,
                        lineColor.data().color.green,
                        lineColor.data().color.blue);

  contour->SetFinalLineColor(finalColor.data().color.red,
                             finalColor.data().color.green,
                             finalColor.data().color.blue);

  settings.setValue("FilterLineColor", lineColor.toString(vgColor::NoAlpha));
  settings.setValue("FilterFinalColor", finalColor.toString(vgColor::NoAlpha));

  return contour;
}

//-----------------------------------------------------------------------------
void vpViewCore::setGraphRenderingEnabled(bool enable)
{
  this->GraphRenderingEnabled = enable;
  if (enable)
    {
    this->forceUpdate();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::updateColorofTracksOfType(int typeIndex, double *rgb)
{
  for (size_t i = 0, size = this->Projects.size(); i < size; ++i)
    {
    this->Projects[i]->TrackModel->UpdateColorOfTracksOfType(typeIndex, rgb);
    }
}

//-----------------------------------------------------------------------------
int findToken(const QStringList& fields, const QString& token,
              Qt::CaseSensitivity cs)
{
  QString trimmedToken = token.trimmed();
  QString trimmedField;
  for (int i = 0; i < fields.count(); ++i)
    {
    trimmedField = fields[i].trimmed();
    if (trimmedField.compare(trimmedToken, cs) == 0)
      {
      return i;
      }
    }

  return -1;
}

//-----------------------------------------------------------------------------
void vpViewCore::startExternalProcess(QString program, QStringList fields,
                                      QString ioPath)
{
  fields.replaceInStrings(QRegExp("^\\s*|\\s*$"),"");

  static int uniqueFilenameCounter = 0;
  bool startExternalProcessAfterFilterExport = false;

  if (this->Projects.size())
    {
    vpProject* currentProject =
      this->Projects[this->SessionView->GetCurrentSession()];
    QString processInput, processOutput, graphOutput;

    if ((findToken(fields, "${PI}", Qt::CaseInsensitive) != -1) ||
        (findToken(fields, "${PO}", Qt::CaseInsensitive) != -1) ||
        (findToken(fields, "${GF}", Qt::CaseInsensitive) != -1) ||
        (findToken(fields, "${EFO}", Qt::CaseInsensitive) != -1))
      {
      QDir ioDir(ioPath);
      if (!ioDir.exists())
        {
        if (!ioDir.mkpath(ioDir.absolutePath()))
          {
          QMessageBox::warning(0, "External Process Error!",
            "Unable to create ioDirectory; process aborted!");
          return;
          }
        }

      if (findToken(fields, "${PI}", Qt::CaseInsensitive) != -1)
        {
        QFileInfo inputFI(ioDir,
          QString("fseQuery_%1.json").arg(uniqueFilenameCounter));
        processInput = inputFI.absoluteFilePath();
        if (!currentProject->ModelIO->WriteFseTracks(qPrintable(processInput)))
          {
          QMessageBox::warning(0, "vpView", "Error writing scene elements file.");
          return;
          }
        }
      if (findToken(fields, "${PO}", Qt::CaseInsensitive) != -1)
        {
        QFileInfo outputFI(ioDir,
          QString("processOutput_%1.txt").arg(uniqueFilenameCounter++));
        processOutput = outputFI.absoluteFilePath();
        if (this->ExternalExecuteMode == 0 || this->ExternalExecuteMode == 2)
          {
          this->ExternalProcessOutputFile = processOutput;
          // if the file doesn't exist, create empty file so we have something
          // to watch for change
          if (!outputFI.exists())
            {
            QFile touchOutput(processOutput);
            touchOutput.open(QIODevice::WriteOnly);
            }
          }
        } // Process output file (PO)
      if (findToken(fields, "${GF}", Qt::CaseInsensitive) != -1)
        {
        QFileInfo outputFI(ioDir,
          QString("graphModel_%1.json").arg(uniqueFilenameCounter++));
        graphOutput = outputFI.absoluteFilePath();
        emit this->graphModelExportRequested(graphOutput);
        }
      }

    if ((findToken(fields, "${FF}", Qt::CaseInsensitive) != -1) &&
        this->ExternalExecuteMode == 1) // Scene Learning mode
      {
      if (currentProject->FiltersFile.empty())
        {
        QMessageBox::warning(0, QString(),
          "Aborting process execution; "
          "Input project FiltersFile is empty, but must be set.");
        return;
        }

      QFileInfo filterFileInfo(currentProject->FiltersFile.c_str());
      this->ExternalProcessOutputFile = filterFileInfo.absoluteFilePath();
      startExternalProcessAfterFilterExport = true;
      }

    // Mapping of tokens to their values
    QHash<QString, QString> externalProcessKeywords;
    externalProcessKeywords["${TF}"] =
      (QString("-trackfile ") + qtString(currentProject->TracksFile));
    externalProcessKeywords["${EF}"] =
      (QString("-eventfile ") + qtString(currentProject->EventsFile));
    externalProcessKeywords["${FF}"] =
      (QString("-filtersfile ") + qtString(currentProject->FiltersFile));
    externalProcessKeywords["${PI}"] =
      (QString("-inputfile ") + processInput);
    externalProcessKeywords["${PO}"] =
      (QString("-outputfile ") + processOutput);
    externalProcessKeywords["${GF}"] =
      (QString("-graphfile ") + graphOutput);
    externalProcessKeywords["${EM}"] =
      (QString("%1").arg(this->ExternalExecuteMode));

    QHash<QString, QString>::const_iterator kwItr;
    for (int i = 0; i < fields.count(); ++i)
      {
      kwItr = externalProcessKeywords.find(fields[i].toUpper());
      if (kwItr != externalProcessKeywords.end())
        {
        fields[i] = fields[i].replace(
                      fields[i], kwItr.value(), Qt::CaseInsensitive);
        }
      }
    }

  this->ExternalProcessProgram = program;
  this->ExternalProcessArguments = fields.join(" ").split(QRegExp("\\s+"));

  this->ExecutedExternalProcessMode = this->ExternalExecuteMode;
  if (startExternalProcessAfterFilterExport)
    {
    emit this->exportFilters(this->ExternalProcessOutputFile, true);
    }
  else
    {
    this->startExternalProcess();
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::startExternalProcess()
{
  this->FileSystemWatcher->addPath(this->ExternalProcessOutputFile);
  connect(this->FileSystemWatcher, SIGNAL(fileChanged(QString)), this,
          SLOT(reactToExternalProcessFileChanged(QString)));


  qDebug()  << "Running process with arguments "
            << this->ExternalProcessArguments;

  this->ExternalProcess->start(this->ExternalProcessProgram,
                               this->ExternalProcessArguments);

  QByteArray standardOutput = this->ExternalProcess->readAllStandardOutput();
  if (standardOutput.length() > 0)
    {
    qDebug() << standardOutput;
    }
}

//-----------------------------------------------------------------------------
bool vpViewCore::isExternalProcessRunning()
{
  return this->ExternalProcess->state() == QProcess::Running;
}

//-----------------------------------------------------------------------------
void vpViewCore::toWindowCoordinates(double &x, double &y)
{
  if (!this->Projects.empty())
    {
    double maxY = this->Projects[0]->AnalysisDimensions[1] - 1;
    y = maxY - y;
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::toWindowCoordinates(double (&xy)[2])
{
  this->toWindowCoordinates(xy[0], xy[1]);
}

//-----------------------------------------------------------------------------
void vpViewCore::toGraphicsCoordinates(double &x, double &y)
{
  if (!this->Projects.empty())
    {
    double maxY = this->Projects[0]->AnalysisDimensions[1] - 1;
    y = maxY - y;
    }
}

//-----------------------------------------------------------------------------
void vpViewCore::toGraphicsCoordinates(double (&xy)[2])
{
  this->toGraphicsCoordinates(xy[0], xy[1]);
}

//-----------------------------------------------------------------------------
void vpViewCore::reactToExternalProcessFileChanged(QString changedFile)
{
  QFile fileInfo(this->ExternalProcessOutputFile);
  if (!fileInfo.exists())
    {
    return;
    }

  disconnect(this->FileSystemWatcher, SIGNAL(fileChanged(QString)), this,
          SLOT(reactToExternalProcessFileChanged(QString)));
  this->FileSystemWatcher->removePath(changedFile);

# if 0
  // if filters file is set, wipe out all existing filters before loading
  if (!this->Projects[this->SessionView->GetCurrentSession()]->
                                                      FiltersFile.empty())
    {
    emit removeAllFilters();
    }
#endif

  // wait until file size hasn't changed for two seconds - can we skip this?
  qint64 oldFileSize, fileSize = fileInfo.size();
  do
    {
    oldFileSize = fileSize;
    QTime waitUntil = QTime::currentTime().addSecs(2);
    while(QTime::currentTime() < waitUntil) {}
    fileSize = fileInfo.size();
    } while(fileSize != oldFileSize);

  if (this->ExecutedExternalProcessMode == 1)  // Learning mode
    {
    this->loadFilters(this->ExternalProcessOutputFile.toStdString().c_str());
    return;
    }

  // Copy the current project and update the scene element file
  static int memoryProjectCounter = 0;
  QScopedPointer<vpProject> project(new vpProject(this->CurrentProjectId++));
  project->CopyConfig(*this->Projects[this->SessionView->GetCurrentSession()]);
  QString projectName(
    this->Projects[this->SessionView->GetCurrentSession()]->Name.c_str());
  projectName += QString(".%1").arg(memoryProjectCounter++);
  project->Name = projectName.toStdString();

  // For now, this is specific to FSE
  if (this->ExecutedExternalProcessMode == 0)
    {
    project->SceneElementsFile = this->ExternalProcessOutputFile.toStdString();
    project->SetIsValid(project->SceneElementsFile, vpProject::FILE_EXIST);
    }

  // For now this is specific to normalcy anamoly
  if (this->ExecutedExternalProcessMode == 2)
    {
    project->EventsFile = this->ExternalProcessOutputFile.toStdString();
    project->SetIsValid(project->EventsFile, vpProject::FILE_EXIST);
    }

  // For now this is specific to normalcy anamoly
  if (this->ExecutedExternalProcessMode == 3)
    {
    project->ActivitiesFile = this->ExternalProcessOutputFile.toStdString();
    project->SetIsValid(project->ActivitiesFile, vpProject::FILE_EXIST);
    }

  project->FrameNumberOffset = this->FrameNumberOffset;

  // If filters file is set, wipe out all existing filters before loading
  if (!project->FiltersFile.empty())
    {
    emit removeAllFilters();
    }

#ifdef VISGUI_USE_VIDTK
  QSharedPointer<vpVidtkFileIO> fileIO(new vpVidtkFileIO);
  fileIO->SetTracksFileName(project->TracksFile.c_str());
  fileIO->SetTrackTraitsFileName(project->TrackTraitsFile.c_str());
  fileIO->SetEventsFileName(project->EventsFile.c_str());
  fileIO->SetEventLinksFileName(project->EventLinksFile.c_str());
  fileIO->SetActivitiesFileName(project->ActivitiesFile.c_str());
  fileIO->SetFseTracksFileName(project->SceneElementsFile.c_str());
  project->ModelIO = fileIO;
  this->processProject(project);
#else
  QMessageBox::warning(0, QString(),
                       "Cannot open project files without VidTK support.");
#endif
}
