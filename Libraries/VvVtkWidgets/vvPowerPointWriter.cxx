// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvPowerPointWriter.h"

#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QProcess>
#include <QVector>

#include <KWPowerPointWrapper.h>

#include <vtkCamera.h>
#include <vtkImageActor.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkPNMWriter.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkWindowToImageFilter.h>

#include <qtStlUtil.h>

#include <vgUnixTime.h>

#include <vtkVgRendererUtils.h>

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvPowerPointWriterPrivate

//-----------------------------------------------------------------------------
class vvPowerPointWriterPrivate
{
public:
  vvPowerPointWriterPrivate(const QString& file) :
    Canceled(false),
    OutputFile(file)
    {
    }

  void reportError(QString message);

public:
  vtkNew<vtkPNMWriter> PnmWriter;
  vtkNew<vtkPNGWriter> PngWriter;

  int Instance;
  bool Canceled;

  QString OutputFile;

  QEventLoop EventLoop;
  QProcess EncodeProcess;
};

QTE_IMPLEMENT_D_FUNC(vvPowerPointWriter)

//-----------------------------------------------------------------------------
void vvPowerPointWriterPrivate::reportError(QString message)
{
  char buffer[256];
  kwpptGetLastErrorMessage(this->Instance, buffer,
                           sizeof(buffer) / sizeof(char));
  qWarning().nospace() << qPrintable(message) << ": " << buffer;
}

//END vvPowerPointWriterPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvPowerPointWriter

//-----------------------------------------------------------------------------
bool vvPowerPointWriter::startNewSlide(const char* slideTemplateName)
{
  QTE_D(vvPowerPointWriter);

  if (kwpptStartNewSlide(d->Instance, slideTemplateName) < 0)
    {
    d->reportError(QString("startNewSlide(%1)").arg(slideTemplateName));
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
int vvPowerPointWriter::addImage(
  const char* itemTemplateName, vtkImageData* imageData, int imageId,
  bool lockAspectRatio)
{
  QTE_D(vvPowerPointWriter);

  const QString filePath =
    QString("%2/%3-%1.png").arg(imageId)
                           .arg(d->OutputFile, itemTemplateName);

  d->PngWriter->SetFileName(qPrintable(filePath));
  d->PngWriter->SetInputData(imageData);
  d->PngWriter->Write();

  const int result = kwpptAddImage(d->Instance, itemTemplateName,
                                   qPrintable(filePath),
                                   (lockAspectRatio ? 1 : 0));
  if (result < 0)
    {
    d->reportError(QString("addImage(%1)").arg(itemTemplateName));
    }
  return result;
}

//-----------------------------------------------------------------------------
int vvPowerPointWriter::addDateTime(
  const char* itemTemplateName, const vgTimeStamp& timeStamp,
  const char* dateLabel, const char* timeLabel)
{
  QTE_D(vvPowerPointWriter);

  const vgUnixTime unixTime(timeStamp.Time);
  const QString dateTimeString =
    QString("%1%2\n%3%4").arg(dateLabel, unixTime.dateString(),
                              timeLabel, unixTime.timeString());

  const int result = kwpptAddText(d->Instance, itemTemplateName,
                                  qPrintable(dateTimeString));
  if (result < 0)
    {
    d->reportError(QString("addDateTime(%1)").arg(itemTemplateName));
    }
  return result;
}

//-----------------------------------------------------------------------------
int vvPowerPointWriter::addDateTimePair(
  const char* itemTemplateName,
  const vgTimeStamp& startTimeStamp, const vgTimeStamp& endTimeStamp)
{
  QTE_D(vvPowerPointWriter);

  const vgUnixTime startTime(startTimeStamp.Time);
  const vgUnixTime endTime(endTimeStamp.Time);
  const QString dateTimeString =
    QString("%1 %2\n%3% 4").arg(startTime.dateString(),
                                startTime.timeString(),
                                endTime.dateString(),
                                endTime.timeString());

  const int result = kwpptAddText(d->Instance, itemTemplateName,
                                  qPrintable(dateTimeString));
  if (result < 0)
    {
    d->reportError(QString("addDateTimePair(%1)").arg(itemTemplateName));
    }
  return result;
}

//-----------------------------------------------------------------------------
int vvPowerPointWriter::addText(
  const char* itemTemplateName, const char* text)
{
  QTE_D(vvPowerPointWriter);

  const int result = kwpptAddText(d->Instance, itemTemplateName, text);
  if (result < 0)
    {
    d->reportError(QString("addText(%1)").arg(itemTemplateName));
    }
  return result;
}

//-----------------------------------------------------------------------------
int vvPowerPointWriter::addPolyLine(
  const char* itemTemplateName, int imageId, const QVector<float>& coordinates)
{
  QTE_D(vvPowerPointWriter);

  const int result = kwpptAddLineInPixels(d->Instance, itemTemplateName, imageId,
                                          coordinates.constData(),
                                          coordinates.count());
  if (result < 0)
    {
    d->reportError(QString("addPolyLine(%1)").arg(itemTemplateName));
    }
  return result;
}

//-----------------------------------------------------------------------------
int vvPowerPointWriter::addVideo(const char* itemTemplateName,
                                 const char* videoFileName,
                                 bool lockAspectRatio)
{
  QTE_D(vvPowerPointWriter);

  const int result = kwpptAddVideo(d->Instance, itemTemplateName,
                                   videoFileName, (lockAspectRatio ? 1 : 0));
  if (result < 0)
    {
    d->reportError(QString("addVideo(%1)").arg(itemTemplateName));
    }
  return result;
}

//-----------------------------------------------------------------------------
void vvPowerPointWriter::writeVideoImageWithRep(vtkImageData* imageData,
                                                vtkPropCollection* props,
                                                int outputId, int frameNumber)
{
  QTE_D(vvPowerPointWriter);

  Q_ASSERT(imageData);

  int dim[3];
  imageData->GetDimensions(dim);

  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;

  renderWindow->SetSize(dim[0], dim[1]);
  renderWindow->SetOffScreenRendering(1);
  renderWindow->AddRenderer(renderer.GetPointer());
  renderer->GetActiveCamera()->ParallelProjectionOn();

  vtkNew<vtkImageActor> imageProp;

  // Add image actor
  imageProp->SetInputData(imageData);
  renderer->AddViewProp(imageProp.GetPointer());

  double bounds[6];
  imageProp->GetBounds(bounds);
  vtkVgRendererUtils::ZoomToExtents2D(renderer.GetPointer(), bounds);

  props->InitTraversal();
  while (vtkProp* prop = props->GetNextProp())
    {
    renderer->AddViewProp(prop);
    }

  // Render to image
  renderWindow->Render();

  vtkNew<vtkWindowToImageFilter> windowToImageFilter;
  windowToImageFilter->SetInput(renderWindow.GetPointer());
  windowToImageFilter->Update();

  const QString path =
    QString("%2/%1").arg(outputId).arg(d->OutputFile);
  QDir dir(path);
  if (!dir.exists())
    {
    dir.mkpath(path);
    }

  // Write as PNM/PPM file as it's much faster than generating a PNG
  const QString filepath =
    QString("%2/%1.pnm").arg(frameNumber, 6, 10, QChar('0')).arg(path);
  d->PnmWriter->SetFileName(qPrintable(filepath));
  d->PnmWriter->SetInputConnection(windowToImageFilter->GetOutputPort());
  d->PnmWriter->Write();
}

//-----------------------------------------------------------------------------
QString vvPowerPointWriter::createVideo(int outputId)
{
  QTE_D(vvPowerPointWriter);

  const QString imagePath =
    QString("%2/%1").arg(outputId).arg(d->OutputFile);

  const QString filePath = imagePath + ".wmv";
  QStringList args;
  args << "-y"; // overwrite existing
  args << "-r";
  args << "10"; // assuming 10 fps
  args << "-i";
  args << QString("%1/%06d.pnm").arg(imagePath);
  args << "-b";
  args << "5M";
  args << filePath;

  qDebug() << "starting ffmpeg";
  d->EncodeProcess.start("ffmpeg", args);

  if (!d->EncodeProcess.waitForStarted())
    {
    qWarning() << "ffmpeg failed to start:"
               << d->EncodeProcess.errorString();
    return QString();
    }

  if (d->EventLoop.exec() != 0)
    {
    qWarning() << "ffmpeg failed:" << d->EncodeProcess.errorString();
    return QString();
    }

  int exitCode = d->EncodeProcess.exitCode();
  qDebug() << "ffmpeg returned with exit code:" << exitCode;
  if (exitCode != EXIT_SUCCESS)
    {
    qWarning() << "ffmpeg failed:";
    qWarning() << d->EncodeProcess.readAllStandardError();
    return QString();
    }

  return filePath;
}

//-----------------------------------------------------------------------------
vvPowerPointWriter::vvPowerPointWriter(QString& file, QString& templateFile) :
  d_ptr(new vvPowerPointWriterPrivate(file))
{
  QTE_D(vvPowerPointWriter);

  d->Instance = kwpptInitialize();
  if (kwpptStartNewPresentation(d->Instance, qPrintable(templateFile)) < 0)
    {
    d->reportError(QString("vvPowerPointWriter(%1)").arg(templateFile));
    }

  connect(&d->EncodeProcess, SIGNAL(error(QProcess::ProcessError)),
          this, SLOT(videoCreateError()));

  connect(&d->EncodeProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
          this, SLOT(videoCreateFinished()));
}

//-----------------------------------------------------------------------------
vvPowerPointWriter::~vvPowerPointWriter()
{
  QTE_D(vvPowerPointWriter);
  if (!d->Canceled)
    {
    std::string outputPowerPoint = stdString(d->OutputFile + "/report.ppt");
    kwpptSave(d->Instance, outputPowerPoint.c_str(), 1);
    kwpptTerminate(d->Instance);
    }
}

//-----------------------------------------------------------------------------
void vvPowerPointWriter::cancel()
{
  QTE_D(vvPowerPointWriter);
  kwpptDiscard(d->Instance);
  d->Canceled = true;
}

//-----------------------------------------------------------------------------
void vvPowerPointWriter::writeTitleSlide(QString& title, QString& subTitle)
{
  QTE_D(vvPowerPointWriter);

  if (this->startNewSlide("title"))
    {
    kwpptAddText(d->Instance, "title", qPrintable(title));
    kwpptAddText(d->Instance, "subTitle", qPrintable(subTitle));
    }
}

//-----------------------------------------------------------------------------
void vvPowerPointWriter::videoCreateError()
{
  QTE_D(vvPowerPointWriter);

  d->EventLoop.exit(1);
}

//-----------------------------------------------------------------------------
void vvPowerPointWriter::videoCreateFinished()
{
  QTE_D(vvPowerPointWriter);

  d->EventLoop.exit(0);
}

//END vvPowerPointWriter
