/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvTrackInfoWidget.h"
#include "ui_vvTrackInfoWidget.h"

#include <vgStringLiteral.h>
#include <vgUnixTime.h>

#include <qtNumericTreeWidgetItem.h>
#include <qtScopedValueChange.h>
#include <qtStlUtil.h>
#include <qtUtil.h>

#include <QDateTime>

QTE_IMPLEMENT_D_FUNC(vvTrackInfoWidget)

namespace // anonymous
{

//-----------------------------------------------------------------------------
void makeTransparent(QLineEdit* widget)
{
  widget->setForegroundRole(QPalette::WindowText);
  qtUtil::makeTransparent(widget);
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vvTrackInfoWidgetPrivate
{
public:
  Ui::vvTrackInfoWidget UI;
};

//-----------------------------------------------------------------------------
vvTrackInfoWidget::vvTrackInfoWidget(QWidget* parent) :
  QTabWidget(parent),
  d_ptr(new vvTrackInfoWidgetPrivate)
{
  QTE_D(vvTrackInfoWidget);
  d->UI.setupUi(this);

  makeTransparent(d->UI.source);
  makeTransparent(d->UI.serial);
  makeTransparent(d->UI.tocCount);
  makeTransparent(d->UI.statesCount);
  makeTransparent(d->UI.timeStart);
  makeTransparent(d->UI.timeEnd);
  makeTransparent(d->UI.timeDuration);

  d->UI.tocEntries->sortByColumn(1, Qt::DescendingOrder);
  d->UI.trajectoryStates->sortByColumn(0, Qt::AscendingOrder);
}

//-----------------------------------------------------------------------------
vvTrackInfoWidget::~vvTrackInfoWidget()
{
}

//-----------------------------------------------------------------------------
void vvTrackInfoWidget::setTrack(vvTrack newTrack)
{
  QTE_D(vvTrackInfoWidget);

  // Summary
  d->UI.source->setText(QString::number(newTrack.Id.Source));
  d->UI.serial->setText(QString::number(newTrack.Id.SerialNumber));

  // Time
  if (newTrack.Trajectory.size())
    {
    vgUnixTime ts(newTrack.Trajectory.begin()->TimeStamp.Time);
    vgUnixTime te(newTrack.Trajectory.rbegin()->TimeStamp.Time);
    vgUnixTime td(te.toInt64() - ts.toInt64());
    d->UI.timeStart->setText(ts.timeString());
    d->UI.timeEnd->setText(te.timeString());
    d->UI.timeDuration->setText(td.timeString());
    }
  else
    {
    d->UI.timeStart->setText("(n/a)");
    d->UI.timeEnd->setText("(n/a)");
    d->UI.timeDuration->setText("(n/a)");
    }

  // Data overview
  d->UI.tocCount->setText(QString::number(newTrack.Classification.size()));
  d->UI.statesCount->setText(QString::number(newTrack.Trajectory.size()));

  // Classification
  qtDelayTreeSorting cds(d->UI.tocEntries);
  d->UI.tocEntries->clear();
  foreach_iter (vvTrackObjectClassification::const_iterator,
                iter, newTrack.Classification)
    {
    qtNumericTreeWidgetItem* item = new qtNumericTreeWidgetItem;
    item->setText(0, qtString(iter->first));
    item->setValue(1, iter->second);
    d->UI.tocEntries->addTopLevelItem(item);
    }
  qtUtil::resizeColumnsToContents(d->UI.tocEntries);

  // Trajectory states
  qtDelayTreeSorting tds(d->UI.trajectoryStates);
  d->UI.trajectoryStates->clear();
  static const auto rf = QStringLiteral("%1,%2 %3\u00d7%4");
  static const auto pf = QStringLiteral("%1, %2");
  static const auto wf = QStringLiteral("%1:%2E,%3N");
  foreach_iter (vvTrackTrajectory::const_iterator, iter, newTrack.Trajectory)
    {
    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(0, vgUnixTime(iter->TimeStamp.Time).timeString());

    const vvImagePointF& ip = iter->ImagePoint;
    const vvImagePoint& tl = iter->ImageBox.TopLeft;
    const vvImagePoint& br = iter->ImageBox.BottomRight;
    item->setText(1, pf.arg(ip.X).arg(ip.Y));
    item->setText(2, rf.arg(tl.X).arg(tl.Y)
                       .arg(1 + br.X - tl.X).arg(1 + br.Y - tl.Y));

    // Add image object
    for (size_t n = 0, k = iter->ImageObject.size(); n < k; ++n)
      {
      QTreeWidgetItem* pitem = new QTreeWidgetItem;
      pitem->setText(0, QString("Image Object Point %1").arg(n));

      const vvImagePointF& op = iter->ImageObject[n];
      pitem->setText(1, pf.arg(op.X).arg(op.Y));

      item->addChild(pitem);
      }

    // Add world location
    if (iter->WorldLocation.GCS != -1)
      {
      QTreeWidgetItem* pitem = new QTreeWidgetItem;
      pitem->setText(0, "World Location");

      const vgGeocodedCoordinate& w = iter->WorldLocation;
      pitem->setText(1, wf.arg(w.GCS).arg(w.Easting).arg(w.Northing));

      item->addChild(pitem);
      }

    d->UI.trajectoryStates->addTopLevelItem(item);
    }
  qtUtil::resizeColumnsToContents(d->UI.trajectoryStates);
}
