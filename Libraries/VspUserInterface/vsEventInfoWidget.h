// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsEventInfoWidget_h
#define __vsEventInfoWidget_h

#include <QWidget>

#include <qtGlobal.h>

class vtkVgEvent;
class vtkVgEventTypeRegistry;

class vsEventInfoWidgetPrivate;

class vsEventInfoWidget : public QWidget
{
  Q_OBJECT

public:
  vsEventInfoWidget(QWidget* parent = 0);
  ~vsEventInfoWidget();

  void setEventTypeRegistry(vtkVgEventTypeRegistry* registry);

public slots:
  void clear();
  void setEvents(QList<vtkVgEvent*> events);
  void updateEvent(vtkVgEvent*);

protected slots:
  void setDetailsVisible(bool);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsEventInfoWidget)

private:
  QTE_DECLARE_PRIVATE(vsEventInfoWidget)
};

#endif
