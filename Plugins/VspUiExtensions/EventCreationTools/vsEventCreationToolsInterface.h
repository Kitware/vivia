/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsEventCreationToolsInterface_h
#define __vsEventCreationToolsInterface_h

#include <QObject>
#include <QSharedPointer>

#include <qtGlobal.h>

#include <vsContour.h>
#include <vsDataSource.h>
#include <vsEventInfo.h>

class QAction;

class vsCore;
class vsMainWindow;
class vsScene;
class vsVideoSource;

class vsEventCreationToolsInterfacePrivate;

class vsEventCreationToolsInterface : public QObject
{
  Q_OBJECT

public:
  vsEventCreationToolsInterface(vsMainWindow*, vsScene*, vsCore*);
  virtual ~vsEventCreationToolsInterface();

protected slots:
  void setVideoSourceStatus(vsDataSource::Status);

  void toggleEventDrawing(bool);
  void toggleEventBoxing(bool);
  void toggleEventQuickCreation(bool);
  void createEventFullFrame();

  void endEventCreation();

  void setActiveTool(QAction*);
  void setCurrentEventType(int);

  void pauseVideoPlayback();
  void resumeVideoPlayback();

  void cancelVideoPlaybackModeRestoration();
  void popBoxCursor();

  void convertContour(vsContour);
  void createEventFromRegion();

  void repopulateEventTypes(QList<vsEventInfo>);

protected:
  QTE_DECLARE_PRIVATE_SPTR(vsEventCreationToolsInterface)

private:
  QTE_DECLARE_PRIVATE(vsEventCreationToolsInterface)
  Q_DISABLE_COPY(vsEventCreationToolsInterface)
};

#endif
