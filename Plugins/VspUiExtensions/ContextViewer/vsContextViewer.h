/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsContextViewer_h
#define __vsContextViewer_h

// VisGUI includes.
#include <vsDescriptorInput.h>

#include <qtGlobal.h>

// VTK includes.
#include <QVTKWidget.h>

// QT includes.
#include <QBasicTimer>
#include <QSet>
#include <QUrl>

// Forward declarations
class vtkVgEvent;
class vtkVgInteractorStyleRubberBand2D;
class vtkVgTimeStamp;
class vtkVgTrack;

class vsCore;
class vsScene;

class vsContextViewerPlugin;

class vsContextViewerPrivate;

class vsContextViewer : public QVTKWidget
{
  Q_OBJECT

public:
  vsContextViewer(vsCore* core, vsScene* scene, QWidget* parent = 0,
                  Qt::WindowFlags flag = 0);
  virtual ~vsContextViewer();

  virtual QSize minimumSizeHint() const QTE_OVERRIDE;

  virtual void addRasterLayer(const QUrl& uri);

  virtual void setViewToExtents(double extents[4]);

  vtkVgInteractorStyleRubberBand2D* interactorStyle();

  void updateSources();

signals:
  void trackSelectionChanged(QSet<vtkIdType>);
  void eventSelectionChanged(QSet<vtkIdType>);

  void trackPicked(vtkIdType);
  void eventPicked(vtkIdType);

public slots:
  void loadContext();

  void resetView();

  void setSelectedTracks(QSet<vtkIdType> eventIds);
  void setSelectedEvents(QSet<vtkIdType> eventIds);

  virtual void update();

  virtual void updateTrackMarkers();
  virtual void updateEventMarkers();
  virtual void updateSceneGraph();

protected slots:
  virtual void render();

  void pickItemOnContext();
  void selectItemsOnContext();

  void updateTrackMarker(vtkVgTrack*);
  void updateEventMarker(vtkVgEvent*);
  void updateRegionlessEventMarker(vtkVgEvent*);

  void updateTrackInfo(vtkVgTrack*);
  void updateEventInfo(vtkVgEvent*);

  void showContextMenu();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsContextViewer)
  friend class vsContextViewerPlugin;

  virtual bool event(QEvent* e) QTE_OVERRIDE;
  virtual void timerEvent(QTimerEvent* e) QTE_OVERRIDE;

private:
  QTE_DECLARE_PRIVATE(vsContextViewer)
  QTE_DISABLE_COPY(vsContextViewer)
};

#endif // __vsContextViewer_h
