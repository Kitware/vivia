/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvReportWriter_h
#define __vvReportWriter_h

#include <QObject>
#include <QScopedPointer>
#include <QString>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vtkVgTimeStamp.h>

class QFile;
class QString;

class vtkImageData;
class vtkMatrix4x4;

class vtkVgDataSourceBase;
class vtkVgEvent;
class vtkVgEventTypeRegistry;
class vtkVgTerrain;
class vtkVgTrack;

class vvReportWriterPrivate;

class VV_VTKWIDGETS_EXPORT vvReportWriter : public QObject
{
  Q_OBJECT

public:
  explicit vvReportWriter(QFile& file);
  ~vvReportWriter();

  void setContext(vtkVgTerrain* context,
                  vtkVgDataSourceBase* contextSource,
                  vtkMatrix4x4* latLonToWorld);

  void setEvent(vtkVgEvent* event,
                int outputId,
                const QString& eventNote,
                vtkVgEventTypeRegistry* typeRegistry = 0,
                int rank = -1,
                double relevancyScore = -1.0,
                const QString& missionId = QString(),
                const QString& source = QString());

  void setTrack(vtkVgTrack* track,
                vtkMatrix4x4* modelToImage = 0,
                const vtkVgTimeStamp& startFrame = vtkVgTimeStamp(),
                const vtkVgTimeStamp& endFrame = vtkVgTimeStamp());

  void setImageData(vtkImageData* imageData,
                    const vtkVgTimeStamp& imageTimeStamp,
                    vtkMatrix4x4* modelToImage = 0,
                    vtkMatrix4x4* imageToLatLon = 0);

  void writeOverview();
  void writeEventSummary();
  void writeEventVideoImage(int number);
  void writeEventVideo();

protected slots:
  void videoCreateError();
  void videoCreateFinished();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvReportWriter)

private:
  QTE_DECLARE_PRIVATE(vvReportWriter)
  Q_DISABLE_COPY(vvReportWriter)
};

#endif
