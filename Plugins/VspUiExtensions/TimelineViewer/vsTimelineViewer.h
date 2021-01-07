// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsTimelineViewer_h
#define __vsTimelineViewer_h

#include <qtGlobal.h>
#include <vtkVgVideoFrameMetaData.h>

#include <QVTKWidget.h>

#include <QSet>

class vtkPlot;

class vtkVgEvent;
class vtkVgTrack;

class vsCore;
class vsScene;

class vsTimelineSelectionCallback;

class vsTimelineViewerPrivate;

class vsTimelineViewer : public QVTKWidget
{
  Q_OBJECT

public:
  vsTimelineViewer(vsCore* core, vsScene* scene, QWidget* parent = 0,
                   Qt::WindowFlags flag = 0);
  virtual ~vsTimelineViewer();

  virtual QSize minimumSizeHint() const QTE_OVERRIDE;

signals:
  void eventSelectionChanged(QSet<vtkIdType>);
  void eventPicked(vtkIdType);

  void trackSelectionChanged(QSet<vtkIdType>);
  void trackPicked(vtkIdType);

public slots:
  void updateTimeFromMetadata(const vtkVgVideoFrameMetaData&);

  void addTrack(vtkVgTrack* track);
  void addEvent(vtkVgEvent* event);

  void updateTracks();
  void updateEvents();

  void setSelectedTracks(QSet<vtkIdType> trackIds);
  void setSelectedEvents(QSet<vtkIdType> eventIds);

protected slots:
  virtual void update();
  virtual void render();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsTimelineViewer)
  friend class vsTimelineSelectionCallback;

  virtual void resizeEvent(QResizeEvent*);

  virtual void setSelectedEntity(vtkPlot* plot, vtkIdType row);
  virtual void activateEntity(vtkPlot* plot, vtkIdType row);

private:
  QTE_DECLARE_PRIVATE(vsTimelineViewer)
  QTE_DISABLE_COPY(vsTimelineViewer)
};

#endif
