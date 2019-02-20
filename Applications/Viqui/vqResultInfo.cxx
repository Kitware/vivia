/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqResultInfo.h"
#include "ui_resultInfo.h"

#include "vqUtil.h"

#include <vgUnixTime.h>
#include <vvQueryResult.h>

#include <vgGeoUtil.h>

#include <qtMath.h>
#include <qtStlUtil.h>
#include <qtUtil.h>

#include <QStyle>

namespace // anonymous
{

//-----------------------------------------------------------------------------
void setText(QLineEdit* widget, const QString& text, bool isValid)
{
  QPalette p = widget->palette();
  QPalette::ColorRole scr = widget->foregroundRole();
  QPalette::ColorRole dcr = QPalette::Text;

  // Copy desired color from parent's palette to widget's palette... to both
  // the foregroundRole() and QPalette::Text, because some styles do not
  // respect foregroundRole() :-(
  if (isValid)
    {
    QWidget* parent = widget->parentWidget();
    QPalette sp = (parent ? parent->palette() : QPalette());
    p.setBrush(QPalette::Active,   scr, sp.brush(QPalette::Active,   scr));
    p.setBrush(QPalette::Active,   dcr, sp.brush(QPalette::Active,   scr));
    p.setBrush(QPalette::Inactive, scr, sp.brush(QPalette::Inactive, scr));
    p.setBrush(QPalette::Inactive, dcr, sp.brush(QPalette::Inactive, scr));
    }
  else
    {
    p.setBrush(QPalette::Active,   scr, p.brush(QPalette::Disabled, scr));
    p.setBrush(QPalette::Active,   dcr, p.brush(QPalette::Disabled, scr));
    p.setBrush(QPalette::Inactive, scr, p.brush(QPalette::Disabled, scr));
    p.setBrush(QPalette::Inactive, dcr, p.brush(QPalette::Disabled, scr));
    }

  widget->setText(text);
  widget->setPalette(p);
}

//-----------------------------------------------------------------------------
void setText(
  QLineEdit* widget, const QString& text, const QString& emptyText = "(none)")
{
  (text.isEmpty()
   ? setText(widget, emptyText, false)
   : setText(widget, text, true));
}

//-----------------------------------------------------------------------------
void setDateTime(QLineEdit* widget, long long ts)
{
  QString format("%1T%2Z");   // ISO 8601
  vgUnixTime t(ts);
  widget->setText(format.arg(t.dateString()) .arg(t.timeString()));
}

//-----------------------------------------------------------------------------
void setText(QLineEdit* widget, const vgGeocodedCoordinate& location)
{
  bool isValid;
  QString text = vgGeodesy::coordString(location, &isValid);
  setText(widget, text, isValid);
}

//-----------------------------------------------------------------------------
void makeTransparent(QLineEdit* widget)
{
  widget->setForegroundRole(QPalette::WindowText);
  qtUtil::makeTransparent(widget);
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vqResultInfo::vqResultInfo(QWidget* parent) : QWidget(parent)
{
  this->UI = new Ui::resultInfo;
  this->UI->setupUi(this);

  connect(this->UI->showDetails, SIGNAL(toggled(bool)),
          this, SLOT(setDetailsVisible(bool)));
  this->setDetailsVisible(this->UI->showDetails->isChecked());

  // Note: The 2 is QLineEditPrivate::horizontalMargin
  this->UI->descriptorInfo->setIndent(
    2 + this->style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

  makeTransparent(this->UI->id);
  makeTransparent(this->UI->relevancy);
  makeTransparent(this->UI->classification);

  makeTransparent(this->UI->missionId);
  makeTransparent(this->UI->streamId);

  makeTransparent(this->UI->startTime);
  makeTransparent(this->UI->endTime);
  makeTransparent(this->UI->duration);

  makeTransparent(this->UI->location);

  this->clear();
}

//-----------------------------------------------------------------------------
vqResultInfo::~vqResultInfo()
{
  delete this->UI;
}

//-----------------------------------------------------------------------------
void vqResultInfo::setResults(QList<const vvQueryResult*> results)
{
  if (results.count() != 1)
    {
    this->clear();
    if (results.count())
      {
      this->UI->notice->setText("Multiple results selected");
      }
    return;
    }

  const vvQueryResult& result = *results.first();

  this->UI->stackedWidget->setCurrentWidget(this->UI->scrollArea);

  this->UI->id->setText(QString::number(result.InstanceId));
  this->UI->relevancy->setText(QString::number(result.RelevancyScore));
  this->UI->classification->setText(
    vqUtil::uiIqrClassificationString(result.UserScore));
  this->UI->descriptorInfo->setResult(result);

  const std::string noteData = result.UserData.Notes;
  const QString note =
    (noteData.empty() ? "(no note set)" : qtString(noteData));
  this->UI->note->setText(note, qtSqueezedLabel::SetFullText |
                          qtSqueezedLabel::SimplifyText);
  this->UI->note->setToolTip(note, qtSqueezedLabel::PlainText |
                             qtSqueezedLabel::AutoWrap);

  setText(this->UI->missionId, qtString(result.MissionId));
  setText(this->UI->streamId, qtString(result.StreamId));

  setDateTime(this->UI->startTime, result.StartTime);
  setDateTime(this->UI->endTime,   result.EndTime);
  vgUnixTime duration(result.EndTime - result.StartTime);
  this->UI->duration->setText(duration.timeString());

  setText(this->UI->location, result.Location);
}

//-----------------------------------------------------------------------------
void vqResultInfo::clear()
{
  this->UI->notice->setText("No result selected");
  this->UI->stackedWidget->setCurrentWidget(this->UI->helpWidget);

  this->UI->id->clear();
  this->UI->relevancy->clear();
  this->UI->classification->clear();
  this->UI->descriptorInfo->clearData();

  this->UI->missionId->clear();
  this->UI->streamId->clear();

  this->UI->startTime->clear();
  this->UI->endTime->clear();
  this->UI->duration->clear();

  this->UI->location->clear();
}

//-----------------------------------------------------------------------------
void vqResultInfo::setDetailsVisible(bool visible)
{
  this->UI->missionIdLabel->setVisible(visible);
  this->UI->missionId->setVisible(visible);
  this->UI->streamIdLabel->setVisible(visible);
  this->UI->streamId->setVisible(visible);

  this->UI->startTimeLabel->setVisible(visible);
  this->UI->startTime->setVisible(visible);
  this->UI->endTimelabel->setVisible(visible);
  this->UI->endTime->setVisible(visible);
  this->UI->durationLabel->setVisible(visible);
  this->UI->duration->setVisible(visible);

  this->UI->locationLabel->setVisible(visible);
  this->UI->location->setVisible(visible);
}
