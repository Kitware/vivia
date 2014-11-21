/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvScoreGradientStopTree.h"

#include <QColorDialog>

#include <qtDoubleSpinBoxDelegate.h>
#include <qtScopedValueChange.h>

#include <vgCheckArg.h>

#include <vgSwatchCache.h>

QTE_IMPLEMENT_D_FUNC(vvScoreGradientStopTree)

//-----------------------------------------------------------------------------
class vvScoreGradientStopTreePrivate
{
public:
  void buildTree(vvScoreGradientStopTree* q) const;

  vgSwatchCache swatches;
  vvScoreGradient stops;
};

//-----------------------------------------------------------------------------
void vvScoreGradientStopTreePrivate::buildTree(
  vvScoreGradientStopTree* q) const
{
  qtScopedBlockSignals bs(q);
  qtDelayTreeSorting ds(q);

  q->clear();

  foreach (vvScoreGradient::Stop stop, this->stops.stops())
    {
    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(0, stop.text);
    item->setIcon(1, this->swatches.swatch(stop.color));
    item->setText(2, QString::number(stop.threshold, 'f', 2));
    item->setData(1, Qt::UserRole, stop.color);
    item->setData(2, Qt::UserRole, stop.threshold);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    q->addTopLevelItem(item);
    }
}

//-----------------------------------------------------------------------------
vvScoreGradientStopTree::vvScoreGradientStopTree(QWidget* parent) :
  QTreeWidget(parent),
  d_ptr(new vvScoreGradientStopTreePrivate)
{
  QTE_D(vvScoreGradientStopTree);

  QStringList headers;
  headers << "Text" << "Color" << "Threshold";
  this->setHeaderLabels(headers);
  this->sortByColumn(2, Qt::DescendingOrder);

  d->buildTree(this);

  qtDoubleSpinBoxDelegate* delegate = new qtDoubleSpinBoxDelegate(this);
  this->setItemDelegateForColumn(2, delegate);
  delegate->setRange(0.0, 1.0);
  delegate->setPrecision(2);
  delegate->setDataRole(Qt::UserRole);

  connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
          this, SLOT(recomputeStops()));
}

//-----------------------------------------------------------------------------
vvScoreGradientStopTree::~vvScoreGradientStopTree()
{
}

//-----------------------------------------------------------------------------
vvScoreGradient vvScoreGradientStopTree::stops() const
{
  QTE_D_CONST(vvScoreGradientStopTree);
  return d->stops;
}

//-----------------------------------------------------------------------------
void vvScoreGradientStopTree::setStops(vvScoreGradient newStops)
{
  QTE_D(vvScoreGradientStopTree);

  CHECK_ARG(newStops != d->stops);

  d->stops = newStops;
  d->buildTree(this);

  emit this->stopsChanged(d->stops);
}

//-----------------------------------------------------------------------------
void vvScoreGradientStopTree::addStop()
{
  // \TODO
}

//-----------------------------------------------------------------------------
void vvScoreGradientStopTree::removeSelectedStops()
{
  // \TODO
}

//-----------------------------------------------------------------------------
bool vvScoreGradientStopTree::edit(
  const QModelIndex& index, QAbstractItemView::EditTrigger trigger, QEvent* e)
{
  if (index.column() == 1 &&
      (trigger == QAbstractItemView::DoubleClicked ||
       trigger == QAbstractItemView::EditKeyPressed))
    {
    QColorDialog chooser(this);
    chooser.setCurrentColor(index.data(Qt::UserRole).value<QColor>());
    if (chooser.exec() == QDialog::Accepted)
      {
      QTE_D(vvScoreGradientStopTree);

      QTreeWidgetItem* item = this->itemFromIndex(index);
      const QColor color = chooser.currentColor();

      item->setIcon(1, d->swatches.swatch(color));
      item->setData(1, Qt::UserRole, color);

      this->recomputeStops();
      }
    return false;
    }

  return QTreeWidget::edit(index, trigger, e);
}

//-----------------------------------------------------------------------------
void vvScoreGradientStopTree::recomputeStops()
{
  QTE_D(vvScoreGradientStopTree);

  QList<vvScoreGradient::Stop> newStops;
  foreach_child (QTreeWidgetItem* item, this->invisibleRootItem())
    {
    vvScoreGradient::Stop stop;
    stop.text = item->text(0);
    stop.color = item->data(1, Qt::UserRole).value<QColor>();
    stop.threshold = item->data(2, Qt::UserRole).toReal();
    newStops.append(stop);
    }

  if (d->stops != newStops)
    {
    d->stops = newStops;
    emit this->stopsChanged(newStops);
    }
}
