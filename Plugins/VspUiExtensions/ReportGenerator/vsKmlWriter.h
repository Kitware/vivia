// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
