/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsEventInfoWidget.h"
#include "ui_eventInfo.h"

#include <vtkVgEvent.h>
#include <vtkVgEventTypeRegistry.h>

#include <vgUnixTime.h>

#include <qtMath.h>
#include <qtUtil.h>

QTE_IMPLEMENT_D_FUNC(vsEventInfoWidget)

namespace // anonymous
{

//-----------------------------------------------------------------------------
void setDateTime(QLineEdit* widget, long long ts)
{
  QString format("%1T%2Z"); // ISO 8601
  vgUnixTime t(ts);
  widget->setText(format.arg(t.dateString()) .arg(t.timeString()));
}

//-----------------------------------------------------------------------------
void makeTransparent(QLineEdit* widget)
{
  widget->setForegroundRole(QPalette::WindowText);
  qtUtil::makeTransparent(widget);
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vsEventInfoWidgetPrivate
{
public:
  vsEventInfoWidgetPrivate() : EventTypeRegistry(0), Event(0) {}

  void updateEventInfo(vtkVgEvent*);

  Ui::vsEventInfoWidget UI;
  vtkVgEventTypeRegistry* EventTypeRegistry;

  vtkVgEvent* Event;
};

//-----------------------------------------------------------------------------
void vsEventInfoWidgetPrivate::updateEventInfo(vtkVgEvent* e)
{
  this->UI.stackedWidget->setCurrentWidget(this->UI.scrollArea);

  this->UI.id->setText(QString::number(e->GetId()));

  double startTime = e->GetStartFrame().GetTime();
  double endTime = e->GetEndFrame().GetTime();
  setDateTime(this->UI.startTime, startTime);
  setDateTime(this->UI.endTime, endTime);
  vgUnixTime duration(endTime - startTime);
  this->UI.duration->setText(duration.timeString());

  const char* noteData = e->GetNote();
  const QString note =
    (noteData && *noteData ? QString::fromLocal8Bit(noteData)
                           : "(no note set)");
  this->UI.note->setText(note, qtSqueezedLabel::SetFullText |
                               qtSqueezedLabel::SimplifyText);
  this->UI.note->setToolTip(note, qtSqueezedLabel::PlainText |
                                  qtSqueezedLabel::AutoWrap);

  this->UI.classifiers->clear();

  if (!this->EventTypeRegistry)
    {
    qDebug() << "Event type registry not set";
    return;
    }

  for (bool valid = e->InitClassifierTraversal(); valid;
       valid = e->NextClassifier())
    {
    int type = e->GetClassifierType();
    double prob = e->GetClassifierProbability();
    QStringList sl;
    sl << this->EventTypeRegistry->GetTypeById(type).GetName()
       << QString::number(prob, 'f', 4);
    QTreeWidgetItem* item = new QTreeWidgetItem(sl);
    this->UI.classifiers->addTopLevelItem(item);
    }
}

//-----------------------------------------------------------------------------
vsEventInfoWidget::vsEventInfoWidget(QWidget* parent)
  : QWidget(parent), d_ptr(new vsEventInfoWidgetPrivate)
{
  QTE_D(vsEventInfoWidget);

  d->UI.setupUi(this);

  connect(d->UI.showDetails, SIGNAL(toggled(bool)),
          this, SLOT(setDetailsVisible(bool)));
  this->setDetailsVisible(d->UI.showDetails->isChecked());

  makeTransparent(d->UI.id);

  makeTransparent(d->UI.startTime);
  makeTransparent(d->UI.endTime);
  makeTransparent(d->UI.duration);

  d->UI.classifiers->sortByColumn(1);

  this->clear();
}

//-----------------------------------------------------------------------------
vsEventInfoWidget::~vsEventInfoWidget()
{
}

//-----------------------------------------------------------------------------
void vsEventInfoWidget::setEventTypeRegistry(vtkVgEventTypeRegistry* registry)
{
  QTE_D(vsEventInfoWidget);
  d->EventTypeRegistry = registry;
}

//-----------------------------------------------------------------------------
void vsEventInfoWidget::clear()
{
  QTE_D(vsEventInfoWidget);

  d->Event = 0;

  d->UI.notice->setText("No event selected");
  d->UI.stackedWidget->setCurrentWidget(d->UI.helpWidget);

  d->UI.id->clear();

  d->UI.startTime->clear();
  d->UI.endTime->clear();
  d->UI.duration->clear();

  d->UI.classifiers->clear();
}

//-----------------------------------------------------------------------------
void vsEventInfoWidget::setDetailsVisible(bool visible)
{
  QTE_D(vsEventInfoWidget);
  d->UI.classifiers->setVisible(visible);
}

//-----------------------------------------------------------------------------
void vsEventInfoWidget::setEvents(QList<vtkVgEvent*> events)
{
  QTE_D(vsEventInfoWidget);

  if (events.count() == 1)
    {
    d->Event = events.first();
    d->updateEventInfo(d->Event);
    return;
    }

  this->clear();
  if (events.count())
    {
    d->UI.notice->setText("Multiple events selected");
    }
}

//-----------------------------------------------------------------------------
void vsEventInfoWidget::updateEvent(vtkVgEvent* e)
{
  QTE_D(vsEventInfoWidget);
  if (d->Event == e)
    {
    d->updateEventInfo(e);
    }
}
