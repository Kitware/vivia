// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvVideoPlayer_h
#define __vvVideoPlayer_h

#include <QWidget>

#include <qtGlobal.h>

#include <vtkSmartPointer.h>

#include <vgExport.h>

#include <vvQueryFormulation.h>

#include "vtkVgTimeStamp.h"

// Forward declarations.
class QTimer;
class QVTKWidget;

class vtkRenderer;
class vtkRenderWindowInteractor;

class vtkVgEventRepresentationBase;
class vtkVgNodeBase;
class vtkVgVideoModel0;
class vtkVgVideoNode;
class vtkVgVideoRepresentationBase0;
class vtkVgVideoProviderBase;

class vtkVgVideoViewer;

class vvVideoPlayerPrivate;

class VV_VTKWIDGETS_EXPORT vvVideoPlayer : public QWidget
{
  Q_OBJECT

public:
  explicit vvVideoPlayer(QWidget* parent = 0, Qt::WindowFlags f = 0);
  virtual ~vvVideoPlayer();

signals:
  void videoPlaying(vtkVgNodeBase& videoNode);
  void videoPaused(vtkVgNodeBase& videoNode);
  void videoStopped(vtkVgNodeBase& videoNode);

  void played(vtkVgVideoNode& videoNode);
  void paused(vtkVgVideoNode& videoNode);
  void stopped(vtkVgVideoNode& videoNode);

  void pickedEvent(vtkIdType id);
  void pickedTrack(vtkIdType id);

  void timeChanged(double value);

  void currentVideoChanged(vtkVgVideoNode* videoNode);

public slots:
  void onExternalPlay(vtkVgVideoNode& videoNode, bool usingSelection = false);
  void onExternalPlay(vtkVgNodeBase& node);
  void onExternalStop(vtkVgVideoNode& videoNode);
  void onExternalPause(vtkVgVideoNode& videoNode);
  void onExternalSelect(const QList<vtkVgNodeBase*>& nodes);

  void onFrameAvailable();

  void onActionPlay();
  void onActionPause();
  void onActionStop();
  void onActionNext();
  void onActionPrev();

  void onAreaOfInterest(double* center, double maxX, double maxY);

  void onVideoPlaying();
  void onVideoPaused();
  void onVideoStopped();

  void onLeftButtonClicked();

  void reset();

  void resetView();

  void seekTo(double value);

  void setEventRegionVisible(bool);
  void setEventFollowingEnabled(bool);
  void setPlaybackEnabled(bool);

  void doubleThePlaybackSpeed();
  void reducePlaybackSpeedByHalf();

public:
  void initialize(bool allowPicking = false);
  void update();

  void loadExternal(vtkVgVideoNode& videoNode);

  void moveCameraTo(double pos[2]);

  void autoCenterUpdate();

  vtkRenderWindowInteractor* interactor();

protected:
  virtual vtkVgVideoRepresentationBase0* buildVideoRepresentation(
    vtkVgVideoNode& videoNode);

  virtual void forceUpdateVideoRepresentation();
  void calculateExtentsScale(double* center, double maxX, double maxY,
                             double& scaleX, double& scaleY);

  void renderLoopOn();
  void renderLoopOff();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvVideoPlayer)

  explicit vvVideoPlayer(vvVideoPlayerPrivate*,
                         QWidget* parent = 0, Qt::WindowFlags f = 0);

  void construct();

  vtkVgVideoModel0* currentPickedVideoModel();

  bool doRubberBandZoom();

  void onUpdate();
  void updateCurrentTime();

  virtual void updateFrame();

  void setupFrameScrubber(vtkVgVideoProviderBase* vs);

  void updateState(vtkVgEventRepresentationBase* eventRep);

  void updatePlaybackSpeedDisplay();

private:
  QTE_DECLARE_PRIVATE(vvVideoPlayer)
};

#endif // __vvVideoPlayer_h
