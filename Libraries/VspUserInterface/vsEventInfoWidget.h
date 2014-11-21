/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
