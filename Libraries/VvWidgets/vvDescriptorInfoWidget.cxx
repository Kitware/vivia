/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvDescriptorInfoWidget.h"
#include "ui_vvDescriptorInfoWidget.h"

#include "vvDescriptorStyle.h"

#include <vgStringLiteral.h>
#include <vgUnixTime.h>

#include <qtNumericTreeWidgetItem.h>
#include <qtScopedValueChange.h>
#include <qtStlUtil.h>
#include <qtUtil.h>

#include <QDateTime>

QTE_IMPLEMENT_D_FUNC(vvDescriptorInfoWidget)

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
class vvDescriptorInfoWidgetPrivate
{
public:
  Ui::vvDescriptorInfoWidget UI;
};

//-----------------------------------------------------------------------------
vvDescriptorInfoWidget::vvDescriptorInfoWidget(QWidget* parent)
  : QTabWidget(parent), d_ptr(new vvDescriptorInfoWidgetPrivate)
{
  QTE_D(vvDescriptorInfoWidget);
  d->UI.setupUi(this);

  makeTransparent(d->UI.name);
  makeTransparent(d->UI.name);
  makeTransparent(d->UI.style);
  makeTransparent(d->UI.source);
  makeTransparent(d->UI.timeStart);
  makeTransparent(d->UI.timeEnd);
  makeTransparent(d->UI.timeDuration);
  makeTransparent(d->UI.valueCount);
  makeTransparent(d->UI.regionCount);
  makeTransparent(d->UI.trackCount);

  d->UI.values->sortByColumn(0, Qt::AscendingOrder);
  d->UI.regions->sortByColumn(0, Qt::AscendingOrder);
  d->UI.tracks->sortByColumn(0, Qt::AscendingOrder);
}

//-----------------------------------------------------------------------------
vvDescriptorInfoWidget::~vvDescriptorInfoWidget()
{
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoWidget::setDescriptor(vvDescriptor newDescriptor)
{
  QTE_D(vvDescriptorInfoWidget);

  // Summary
  d->UI.name->setText(qtString(newDescriptor.DescriptorName));
  d->UI.style->setText(vvDescriptorStyle::styleString(newDescriptor));
  d->UI.source->setText(qtString(newDescriptor.ModuleName));

  // Time
  if (newDescriptor.Region.size())
    {
    vgUnixTime ts(newDescriptor.Region.begin()->TimeStamp.Time);
    vgUnixTime te(newDescriptor.Region.rbegin()->TimeStamp.Time);
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
  size_t valueCount = 0;
  for (size_t n = 0, k = newDescriptor.Values.size(); n < k; ++n)
    {
    valueCount += newDescriptor.Values[n].size();
    }
  d->UI.valueCount->setText(QString::number(valueCount));
  d->UI.regionCount->setText(QString::number(newDescriptor.Region.size()));
  d->UI.trackCount->setText(QString::number(newDescriptor.TrackIds.size()));

  // Values
  qtDelayTreeSorting vds(d->UI.values);
  d->UI.values->clear();
  for (size_t i = 0, ki = newDescriptor.Values.size(); i < ki; ++i)
    {
    const std::vector<float>& v = newDescriptor.Values[i];

    // Create first-level entry
    qtNumericTreeWidgetItem* iitem = new qtNumericTreeWidgetItem;
    iitem->setValue(0, i);
    iitem->setText(1, QString("(%1 values)").arg(v.size()));

    // Add sub-values
    for (size_t j = 0, kj = v.size(); j < kj; ++j)
      {
      // Create second-level entry
      qtNumericTreeWidgetItem* jitem = new qtNumericTreeWidgetItem;
      jitem->setValue(0, j);
      jitem->setValue(1, v[j]);
      iitem->addChild(jitem);
      }

    // Add first-level entry
    d->UI.values->addTopLevelItem(iitem);
    }
  qtUtil::resizeColumnsToContents(d->UI.values);

  // Regions
  qtDelayTreeSorting rds(d->UI.regions);
  d->UI.regions->clear();
  static const auto rf = QStringLiteral("%1,%2 %3\u00d7%4");
  foreach_iter (vvDescriptorRegionMap::const_iterator, iter,
                newDescriptor.Region)
    {
    const vvImagePoint& tl = iter->ImageRegion.TopLeft;
    const vvImagePoint& br = iter->ImageRegion.BottomRight;
    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(0, vgUnixTime(iter->TimeStamp.Time).timeString());
    item->setText(1, rf.arg(tl.X).arg(tl.Y)
                       .arg(1 + br.X - tl.X).arg(1 + br.Y - tl.Y));
    d->UI.regions->addTopLevelItem(item);
    }
  qtUtil::resizeColumnsToContents(d->UI.regions);

  // Tracks
  qtDelayTreeSorting tds(d->UI.tracks);
  d->UI.tracks->clear();
  for (size_t n = 0, k = newDescriptor.TrackIds.size(); n < k; ++n)
    {
    const vvTrackId& tid = newDescriptor.TrackIds[n];
    qtNumericTreeWidgetItem* item = new qtNumericTreeWidgetItem;
    item->setValue(0, tid.Source);
    item->setValue(1, tid.SerialNumber);
    d->UI.tracks->addTopLevelItem(item);
    }
  qtUtil::resizeColumnsToContents(d->UI.tracks);
}
