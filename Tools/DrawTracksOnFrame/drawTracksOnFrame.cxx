/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <vdfDataSource.h>
#include <vdfSourceService.h>
#include <vdfTrackReader.h>

#if defined(VISGUI_USE_GDAL)
  #include <vtkVgGDALReader.h>
#endif

#include <vtkVgTrackModel.h>
#include <vtkVgTrackRepresentation.h>

#include <vtkVgImageSource.h>
#include <vtkVgJPEGReader.h>
#include <vtkVgMultiResJpgImageReader2.h>
#include <vtkVgPNGReader.h>
#include <vtkVgRendererUtils.h>

#include <vgAbstractFactory.h>

#include <qtCliArgs.h>
#include <qtStlUtil.h>

#include <QVTKWidget.h>

#include <vtkCamera.h>
#include <vtkExtractVOI.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkJPEGWriter.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWindowToImageFilter.h>

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QUrl>

typedef QHash<vdfTrackId, vdfTrackReader::Track> TrackMap;

//-----------------------------------------------------------------------------
class ImageSourceFactory :
  public vgAbstractFactory<vtkVgBaseImageSource*, std::string>
{
public:
  virtual vtkVgBaseImageSource* Create(std::string source) QTE_OVERRIDE;
} imageSourceFactory;

//-----------------------------------------------------------------------------
vtkVgBaseImageSource* ImageSourceFactory::Create(std::string source)
{
  for (size_t i = 0; i < this->Creators.size(); ++i)
    {
    vtkVgBaseImageSource* imageSourceInstance = this->Creators[i]();
    if (imageSourceInstance->CanRead(source))
      {
      return imageSourceInstance;
      }
    else
      {
      imageSourceInstance->Delete();
      }
    }

  return 0;
}

//-----------------------------------------------------------------------------
vtkImageData* getImageData(
  vtkSmartPointer<vtkVgBaseImageSource> imageSource, int downsample)
{
  imageSource->UpdateInformation();

  int imageDimensions[2];
  imageSource->GetDimensions(imageDimensions);

  int extents[6] =
    { 0, imageDimensions[0] - 1, 0, imageDimensions[1] - 1, 0, 0 };
  imageSource->SetReadExtents(extents);
  imageSource->SetOutputResolution(imageDimensions[0], imageDimensions[1]);
  imageSource->SetLevel(0);
  imageSource->Update();

  // crop to the area of interest / downsample to output dimensions
  vtkNew<vtkExtractVOI> voi;
  voi->SetInputConnection(imageSource->GetOutputPort());
  voi->SetVOI(extents);
  voi->SetSampleRate(1 << downsample, 1 << downsample, 1);
  voi->UpdateWholeExtent();

  vtkImageData* const imageData = voi->GetOutput();
  if (imageData)
    {
    imageData->Register(imageSource);
    return imageData;
    }

  return 0;
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  // Set application information
  QCoreApplication::setApplicationName("VisGUI track conversion utility");
  QCoreApplication::setOrganizationName("Kitware");
  QCoreApplication::setOrganizationDomain("kitware.com");

  // Set up command line options
  qtCliArgs args(argc, argv);

  qtCliOptions options;
  options.add("output <file>", "Write result to specified image file")
         .add("o", qtCliOption::Short);
  options.add("gui", "Display result on screen")
         .add("g", qtCliOption::Short);
  options.add("downsample <num>",
              "Downsample imagery by specified factor (power of two); "
              "may be required for larger images", "0")
         .add("s", qtCliOption::Short);
  options.add("world", "Use world coordinates for track position")
         .add("w", qtCliOption::Short);
  args.addOptions(options);

  qtCliOptions namedArgs;
  namedArgs.add("image", "Path to input image file", qtCliOption::Required);
  namedArgs.add("tracks", "Path to input tracks file", qtCliOption::Required);
  args.addNamedArguments(namedArgs);

  // Parse arguments
  args.parseOrDie();

  bool okay = true;
  const int downsample = args.value("downsample").toInt(&okay);
  if (!okay || downsample < 0)
    {
    qCritical() << "ERROR: Downsample must be a non-negative integer";
    return EXIT_FAILURE;
    }

  const QString outputName = args.value("output");

  // Create application (needed for vdfTrackReader event loop)
  QScopedPointer<QCoreApplication> app;
  if (args.isSet("gui"))
    {
    app.reset(new QApplication(args.qtArgc(), args.qtArgv()));
    }
  else if (!outputName.isEmpty())
    {
    app.reset(new QCoreApplication(args.qtArgc(), args.qtArgv()));
    }
  else
    {
    qWarning() << "WARNING: Neither --gui nor --output was given;"
                  " nothing to do!";
    return EXIT_SUCCESS;
    }

  // Determine output format, if applicable
  vtkSmartPointer<vtkImageWriter> imageWriter;
  if (!outputName.isEmpty())
    {
    const QString ext = QFileInfo(outputName).suffix().toLower();
    if (ext == "jpg" || ext == "jpeg")
      {
      imageWriter = vtkSmartPointer<vtkJPEGWriter>::New();
      }
    else if (ext == "png")
      {
      imageWriter = vtkSmartPointer<vtkPNGWriter>::New();
      }
    else
      {
      qCritical() << "ERROR: Output extension" << ext
                  << "does not match any supported output format";
      qCritical() << "ERROR: Supported formats: PNG, JPG";
      return EXIT_FAILURE;
      }
    }

  // Register imagery sources
  imageSourceFactory.Register(&vtkVgPNGReader::Create);
  imageSourceFactory.Register(&vtkVgMultiResJpgImageReader2::Create);
  imageSourceFactory.Register(&vtkVgImageSource::Create);
  imageSourceFactory.Register(&vtkVgJPEGReader::Create);
#if defined(VISGUI_USE_GDAL)
  imageSourceFactory.Register(&vtkVgGDALReader::Create);
#endif

  // Create image source
  qDebug() << "Reading imagery from" << args.value("image");
  const std::string imageFile = stdString(args.value("image"));

  vtkSmartPointer<vtkVgBaseImageSource> imageSource;
  imageSource.TakeReference(imageSourceFactory.Create(imageFile));

  if (!imageSource)
    {
    qCritical() << "ERROR: Failed to create image source";
    return EXIT_FAILURE;
    }

  imageSource->SetFileName(imageFile.c_str());
  vtkSmartPointer<vtkImageData> imageData =
    vtkSmartPointer<vtkImageData>::Take(getImageData(imageSource, downsample));
  if (!imageData)
    {
    qCritical() << "ERROR: Failed to read image data";
    return EXIT_FAILURE;
    }

  // Set up reader
  vdfTrackReader reader;

  // Read input tracks
  const QUrl trackUri = QUrl::fromLocalFile(args.value("tracks"));
  QScopedPointer<vdfDataSource> trackSource(
    vdfSourceService::createArchiveSource(trackUri));

  if (!trackSource)
    {
    qCritical() << "ERROR: Failed to create track source";
    return EXIT_FAILURE;
    }

  // Read tracks
  qDebug() << "Reading tracks from" << trackUri;
  reader.setSource(trackSource.data());
  if (!reader.exec() || !reader.hasData())
    {
    qCritical() << "ERROR: Failed to read tracks (or no tracks present)";
    return EXIT_FAILURE;
    }

  // Create and populate track model
  vtkNew<vtkVgTrackModel> trackModel;
  trackModel->SetDisplayAllTracks(true);
  trackModel->Initialize();
  trackModel->SetShowTracksBeforeStart(true);
  trackModel->SetShowTracksAfterExpiration(true);

  const bool useWorld = args.isSet("world");
  vtkIdType nextId = 0;
  foreach (const vdfTrackReader::Track& track, reader.tracks())
    {
    vtkNew<vtkVgTrack> t;
    t->SetPoints(trackModel->GetPoints());
    t->SetId(++nextId);

    foreach (const vvTrackState& s, track.Trajectory)
      {
      if (useWorld)
        {
        double pos[2] = { s.WorldLocation.Easting, s.WorldLocation.Northing };
        t->InsertNextPoint(s.TimeStamp, pos, s.WorldLocation);
        }
      else
        {
        double pos[2] = { s.ImagePoint.X, s.ImagePoint.Y };
        t->InsertNextPoint(s.TimeStamp, pos, s.WorldLocation);
        }
      }

    trackModel->AddTrack(t.GetPointer());
    }

  // Set up scene
  vtkNew<vtkImageActor> imageActor;
  imageActor->SetInputData(imageData);
  imageActor->Update();

  vtkNew<vtkRenderer> sceneRenderer;
  sceneRenderer->GetActiveCamera()->ParallelProjectionOn();
  sceneRenderer->AddActor(imageActor.GetPointer());

  vtkVgRendererUtils::ZoomToImageExtents2D(
    sceneRenderer.GetPointer(), imageData);

  vtkNew<vtkVgTrackRepresentation> trackRepresentation;
  trackRepresentation->SetTrackModel(trackModel.GetPointer());
  trackRepresentation->SetVisible(true);
  trackRepresentation->SetZOffset(0.1);

  if (useWorld)
    {
    vtkSmartPointer<vtkMatrix4x4> xf = imageSource->GetImageToWorldMatrix();
    if (!xf)
      {
      qCritical() << "ERROR: Failed to get image to world matrix";
      return EXIT_FAILURE;
      }

    xf->Invert();
    trackRepresentation->SetRepresentationMatrix(xf);
    }

  trackRepresentation->Update();

  vtkPropCollection* props = trackRepresentation->GetNewRenderObjects();
  vtkProp* prop;
  props->InitTraversal();
  while ((prop = props->GetNextProp()))
    {
    sceneRenderer->AddViewProp(prop);
    }

  // Show GUI, if requested
  if (args.isSet("gui"))
    {
    QVTKWidget w;
    w.setMinimumSize(150, 150);

    vtkRenderWindow* renderWindow = w.GetRenderWindow();
    renderWindow->AddRenderer(sceneRenderer.GetPointer());
    renderWindow->Render();

    w.show();
    app->exec();
    }

  // Write output image, if requested
  if (imageWriter) // only set if --output used
    {
    int* dim = imageData->GetDimensions();

    qDebug().nospace() << "Rendering output image ("
                       << dim[0] << 'x' << dim[1] << ")";

    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow->SetSize(dim[0], dim[1]);
    renderWindow->SetOffScreenRendering(1);
    renderWindow->AddRenderer(sceneRenderer.GetPointer());
    renderWindow->Render();

    vtkNew<vtkWindowToImageFilter> windowToImageFilter;
    windowToImageFilter->SetInput(renderWindow.GetPointer());
    windowToImageFilter->Update();

    qDebug() << "Writing output image to" << outputName;
    imageWriter->SetFileName(qPrintable(outputName));
    imageWriter->SetInputConnection(windowToImageFilter->GetOutputPort());
    imageWriter->Write();
    }

  // That's all, folks
  return EXIT_SUCCESS;
}
