// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgRegionWidget_h
#define __vtkVgRegionWidget_h

#include <QColor>
#include <QObject>
#include <QRect>

#include <qtGlobal.h>
#include <qtStatusSource.h>

#include <vgExport.h>

class vtkObject;
class vtkRenderWindowInteractor;

class vtkVgRegionWidgetPrivate;

class VTKVGQT_WIDGETS_EXPORT vtkVgRegionWidget : public QObject
{
  Q_OBJECT

public:
  vtkVgRegionWidget(vtkRenderWindowInteractor*, QObject* parent = 0);
  ~vtkVgRegionWidget();

  QRect rect();
  void setRect(const QRect&);

  QSize displaySize();

  void setStyle(QColor color, qreal lineWidth);

  void beginQuick(const QSize&);

signals:
  void dropDrawStarted();
  void dropDrawCanceled();
  void dropDrawCompleted();
  void beginningManipulation();
  void completed();
  void canceled();
  void statusMessageAvailable(QString = QString());
  void statusMessageAvailable(qtStatusSource, QString = QString());

public slots:
  void begin();

  void setColor(QColor);
  void setLineWidth(qreal);

protected slots:
  void viewKeyEvent(vtkObject* sender, unsigned long event);
  void viewMouseEvent(vtkObject* sender, unsigned long event);
  void beginSecondPick();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vtkVgRegionWidget)

private:
  QTE_DECLARE_PRIVATE(vtkVgRegionWidget)
  Q_DISABLE_COPY(vtkVgRegionWidget)
};

#endif
