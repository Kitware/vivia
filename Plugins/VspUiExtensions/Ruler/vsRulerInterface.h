// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsRulerInterface_h
#define __vsRulerInterface_h

#include <QObject>
#include <QSharedPointer>

#include <qtGlobal.h>

#include <vtkVgVideoFrameMetaData.h>

#include <vsContour.h>
#include <vsDataSource.h>
#include <vsEventInfo.h>

class QAction;

class vsCore;
class vsMainWindow;
class vsScene;
class vsVideoSource;

class vsRulerInterfacePrivate;

class vsRulerInterface : public QObject
{
  Q_OBJECT

public:
  vsRulerInterface(vsMainWindow*, vsScene*, vsCore*);
  virtual ~vsRulerInterface();

signals:
  void statusMessageAvailable(QString message);

protected slots:
  void setVideoSourceStatus(vsDataSource::Status);
  void setGsdFromMetadata(vtkVgVideoFrameMetaData);

  void toggleRuler(bool);
  void hideRuler();

  void rulerLengthChanged();

protected:
  QTE_DECLARE_PRIVATE_SPTR(vsRulerInterface)

private:
  QTE_DECLARE_PRIVATE(vsRulerInterface)
  Q_DISABLE_COPY(vsRulerInterface)
};

#endif
