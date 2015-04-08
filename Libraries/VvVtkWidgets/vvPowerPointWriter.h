/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvPowerPointWriter_h
#define __vvPowerPointWriter_h

#include <vtkVgTimeStamp.h>

#include <vgExport.h>

#include <qtGlobal.h>
#include <qtThread.h>

#include <QObject>
#include <QScopedPointer>
#include <QString>

#include <vector>

class vtkImageData;
class vtkMatrix4x4;
class vtkPropCollection;

class vtkVgTrack;
class vtkVgEvent;
class vtkVgEventTypeRegistry;

class vvPowerPointWriterPrivate;

class VV_VTKWIDGETS_EXPORT vvPowerPointWriter : public QObject
{
  Q_OBJECT

public:
  explicit vvPowerPointWriter(QString& file, QString& templateFile);
  virtual ~vvPowerPointWriter();

  bool startNewSlide(const char* slideTemplateName);
  int addImage(const char* itemTemplateName, vtkImageData* imageData,
               int imageId, bool lockAspectRatio);
  int addDateTime(const char* itemTemplateName, const vgTimeStamp& timeStamp,
                  const char* dateLabel = 0, const char* timeLabel = 0);
  int addDateTimePair(const char* itemTemplateName,
                      const vgTimeStamp& startTimeStamp,
                      const vgTimeStamp& endTimeStamp);
  int addText(const char* itemTemplateName, const char* text);
  int addPolyLine(const char* itemTemplateName, int imageId,
                  const QVector<float>& coordinates);
  int addVideo(const char* itemTemplateName, const char* videoFileName,
               bool lockAspectRatio);

  void writeVideoImageWithRep(vtkImageData* imageData,
                              vtkPropCollection* props,
                              int outputId, int frameNumber);

  QString createVideo(int outputId);

  virtual void cancel();

  virtual void writeTitleSlide(QString& title, QString& subTitle);

protected slots:
  void videoCreateError();
  void videoCreateFinished();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvPowerPointWriter);

private:
  QTE_DECLARE_PRIVATE(vvPowerPointWriter);
  Q_DISABLE_COPY(vvPowerPointWriter);
};

#endif
