// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsContourWidget_h
#define __vsContourWidget_h

#include <QObject>

#include <qtGlobal.h>

#include <vsContour.h>

class vtkContourWidget;
class vtkMatrix4x4;
class vtkPoints;
class vtkRenderWindowInteractor;

class vsContourWidgetPrivate;

class vsContourWidget : public QObject
{
  Q_OBJECT

public:
  enum State
    {
    Begin,
    Manipulate,
    Complete
    };

  vsContourWidget(int id, vtkRenderWindowInteractor*);
  vsContourWidget(const vsContour&, vtkRenderWindowInteractor*);
  ~vsContourWidget();

  void setMatrix(vtkMatrix4x4* matrix);
  void setPoints(const QPolygonF&);

  int state() const;

  bool isClosed();

  int id() const;
  QString name() const;
  vsContour::Type type() const;

  vtkContourWidget* widget();
  vtkPoints* points();

  vsContour toContour();

signals:
  void interactionComplete();
  void manipulationComplete();

public slots:
  void begin();
  void close();
  void finalize();

  void setName(QString);
  void setType(vsContour::Type);

  void setVisible(bool);

protected slots:
  void onRightClick();
  void onInteractionComplete();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsContourWidget)

private:
  QTE_DECLARE_PRIVATE(vsContourWidget)
  Q_DISABLE_COPY(vsContourWidget)
};

#endif
