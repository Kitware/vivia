/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsKmlWriter_h
#define __vsKmlWriter_h

#include <QSharedPointer>

#include <qtGlobal.h>

#include <vgVtkVideoFrame.h>

template <typename T> class QList;

class vsEventUserInfo;
class vsVideoSource;

class vsKmlWriterPrivate;

class vtkVgEventTypeRegistry;
class vtkVgTrack;

class vsKmlWriter
{
public:
  vsKmlWriter();
  vsKmlWriter(QList<vsEventUserInfo> events,
              vtkVgEventTypeRegistry* registry,
              vsVideoSource* videoSource);
  virtual ~vsKmlWriter();

  void setOutputPath(const QString& path);

  void addTrack(vtkIdType eventId, const vtkVgTimeStamp& startTime,
                const vtkVgTimeStamp& endTime, vtkVgTrack* track,
                const QString& note, vtkMatrix4x4* refToImage,
                vtkMatrix4x4* imageToLatLon);

  void write();

protected:
  void process();

  QTE_DECLARE_PRIVATE_SPTR(vsKmlWriter)

private:
  QTE_DECLARE_PRIVATE(vsKmlWriter)
  Q_DISABLE_COPY(vsKmlWriter)
};

#endif
