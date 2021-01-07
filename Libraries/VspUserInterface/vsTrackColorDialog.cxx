// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsTrackColorDialog.h"
#include "ui_trackColor.h"

#include <limits>

#include <qtUtil.h>
#include <qtMap.h>

#include <qtColorButtonItemWidget.h>
#include <qtSpinBoxDelegate.h>

#include <vtkVgTrack.h>
#include <vvTrackInfo.h>

#include "vsSettings.h"

namespace // anonymous
{

//-----------------------------------------------------------------------------
void setColorFromTocType(
  qtColorButton* penButton, qtColorButton* fgButton, qtColorButton* bgButton,
  QLabel* label, vtkVgTrack::enumTrackPVOType type)
{
  vvTrackInfo ti(type);
  penButton->setColor(ti.PenColor.toQColor());
  fgButton->setColor(ti.ForegroundColor.toQColor());
  bgButton->setColor(ti.BackgroundColor.toQColor());
  if (label)
    {
    label->setText(ti.Name + ':');
    }
}

//-----------------------------------------------------------------------------
void saveTocTypeColor(
  vtkVgTrack::enumTrackPVOType type, const QColor& penColor,
  const QColor& foreColor, const QColor& backColor)
{
  vvTrackInfo ti(type);
  ti.PenColor = penColor;
  ti.ForegroundColor = foreColor;
  ti.BackgroundColor = backColor;
  ti.write();
}

} // namespace <anonymous>

QTE_IMPLEMENT_D_FUNC(vsTrackColorDialog)

//-----------------------------------------------------------------------------
class vsTrackColorDialogPrivate
{
public:
  Ui::vsTrackColorDialog UI;

  void addTrackSource(QTreeWidgetItem* item);
};

//-----------------------------------------------------------------------------
void vsTrackColorDialogPrivate::addTrackSource(QTreeWidgetItem* item)
{
  // Set common item attributes
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  item->setTextAlignment(0, Qt::AlignRight | Qt::AlignVCenter);

  // Add item to tree and create item widgets for colors (which will parent
  // and assign themselves automatically)
  this->UI.sourceColors->addTopLevelItem(item);
  new qtColorButtonItemWidget(item, 1);
  new qtColorButtonItemWidget(item, 2);
  new qtColorButtonItemWidget(item, 3);
}

//-----------------------------------------------------------------------------
vsTrackColorDialog::vsTrackColorDialog(QWidget* parent)
  : QDialog(parent), d_ptr(new vsTrackColorDialogPrivate)
{
  QTE_D(vsTrackColorDialog);
  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);

  // Get TOC colors
#define SET_COLOR_FROM_TOC_TYPE(button, label, type) \
  setColorFromTocType(button, button##Fore, button##Back, label, type)

  SET_COLOR_FROM_TOC_TYPE(d->UI.defaultColor, 0, vtkVgTrack::Unclassified);
  SET_COLOR_FROM_TOC_TYPE(d->UI.personColor,
                          d->UI.personLabel, vtkVgTrack::Person);
  SET_COLOR_FROM_TOC_TYPE(d->UI.vehicleColor,
                          d->UI.vehicleLabel, vtkVgTrack::Vehicle);
  SET_COLOR_FROM_TOC_TYPE(d->UI.otherColor,
                          d->UI.otherLabel, vtkVgTrack::Other);

  vsSettings settings;
  d->UI.minimumColor->setColor(settings.dataMinColor());
  d->UI.maximumColor->setColor(settings.dataMaxColor());

  d->UI.colorBySource->setChecked(settings.colorTracksBySource());

  // Get per-source colors
  QList<vvTrackInfo> sourceInfo = vvTrackInfo::trackSources();
  foreach (const vvTrackInfo& ti, sourceInfo)
    {
    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setData(0, Qt::EditRole, ti.Source);
    item->setData(1, Qt::EditRole, ti.PenColor.toQColor());
    item->setData(2, Qt::EditRole, ti.ForegroundColor.toQColor());
    item->setData(3, Qt::EditRole, ti.BackgroundColor.toQColor());

    d->addTrackSource(item);
    }

  qtSpinBoxDelegate* const delegate = new qtSpinBoxDelegate(d->UI.sourceColors);
  delegate->setRange(0, std::numeric_limits<int>::max());
  delegate->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  d->UI.sourceColors->setItemDelegateForColumn(0, delegate);

  qtUtil::resizeColumnsToContents(d->UI.sourceColors);

  connect(d->UI.addSource, SIGNAL(clicked()), this, SLOT(addSource()));
  connect(d->UI.removeSource, SIGNAL(clicked()), this, SLOT(removeSources()));
  connect(d->UI.sourceColors, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateSourceButtonsState()));
}

//-----------------------------------------------------------------------------
vsTrackColorDialog::~vsTrackColorDialog()
{
}

//-----------------------------------------------------------------------------
vsTrackColorDialog::ColoringMode vsTrackColorDialog::mode() const
{
  QTE_D_CONST(vsTrackColorDialog);
  return (d->UI.colorByToc->isChecked()   ? ColorByClassification :
          d->UI.colorByData->isChecked()  ? ColorByDynamicData    :
                                            SingleColor);
}

//-----------------------------------------------------------------------------
void vsTrackColorDialog::setMode(vsTrackColorDialog::ColoringMode mode)
{
  QTE_D(vsTrackColorDialog);

  switch (mode)
    {
    case ColorByClassification:
      d->UI.colorByToc->setChecked(true);
      break;
    case ColorByDynamicData:
      d->UI.colorByData->setChecked(true);
      break;
    case SingleColor:
    default:
      d->UI.singleColor->setChecked(true);
    }
}

//-----------------------------------------------------------------------------
QString vsTrackColorDialog::dynamicDataSet()
{
  QTE_D_CONST(vsTrackColorDialog);
  return (d->UI.colorByData->isChecked()
          ? d->UI.dataSet->currentText() : QString());
}

//-----------------------------------------------------------------------------
void vsTrackColorDialog::setDynamicDataSet(const QString& set)
{
  QTE_D(vsTrackColorDialog);
  d->UI.dataSet->setCurrentIndex(d->UI.dataSet->findText(set));
}

//-----------------------------------------------------------------------------
void vsTrackColorDialog::setDynamicDataSets(const QStringList& sets)
{
  QTE_D(vsTrackColorDialog);

  d->UI.dataSet->clear();

  if (sets.isEmpty())
    {
    d->UI.colorByData->setEnabled(false);
    }
  else
    {
    d->UI.dataSet->insertItems(0, sets);
    d->UI.colorByData->setEnabled(true);
    }
}

//-----------------------------------------------------------------------------
void vsTrackColorDialog::accept()
{
  QTE_D(vsTrackColorDialog);

  vsSettings settings;

#define SAVE_TOC_TYPE_COLOR(type, button) \
  saveTocTypeColor(type, button->color(), \
                   button##Fore->color(), button##Back->color())

  SAVE_TOC_TYPE_COLOR(vtkVgTrack::Unclassified, d->UI.defaultColor);

  if (d->UI.colorByToc->isChecked())
    {
    SAVE_TOC_TYPE_COLOR(vtkVgTrack::Person,  d->UI.personColor);
    SAVE_TOC_TYPE_COLOR(vtkVgTrack::Vehicle, d->UI.vehicleColor);
    SAVE_TOC_TYPE_COLOR(vtkVgTrack::Other,   d->UI.otherColor);
    }
  else if (d->UI.colorByData->isChecked())
    {
    settings.setDataMinColor(d->UI.minimumColor->color());
    settings.setDataMaxColor(d->UI.maximumColor->color());
    }

  settings.setColorTracksBySource(d->UI.colorBySource->isChecked());
  if (d->UI.colorBySource->isChecked())
    {
    // Get collection of sources that were previously saved
    QSet<int> removedSources;
    foreach (const vvTrackInfo& ti, vvTrackInfo::trackSources())
      {
      removedSources.insert(ti.Source);
      }

    // Write new sources
    foreach_child (QTreeWidgetItem* const item,
                   d->UI.sourceColors->invisibleRootItem())
      {
      vvTrackInfo ti;
      ti.Source = item->data(0, Qt::EditRole).toInt();
      ti.PenColor = item->data(1, Qt::EditRole).value<QColor>();
      ti.ForegroundColor = item->data(2, Qt::EditRole).value<QColor>();
      ti.BackgroundColor = item->data(3, Qt::EditRole).value<QColor>();
      ti.write();
      removedSources.remove(ti.Source);
      }

    // Remove configuration for any sources that the user deleted
    qtUtil::map(removedSources, &vvTrackInfo::eraseTrackSource);
    }

  settings.commit();

  QDialog::accept();
}

//-----------------------------------------------------------------------------
void vsTrackColorDialog::addSource()
{
  QTE_D(vsTrackColorDialog);

  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setData(0, Qt::EditRole, 1);
  item->setData(1, Qt::EditRole, d->UI.defaultColor->color());
  item->setData(2, Qt::EditRole, d->UI.defaultColorFore->color());
  item->setData(3, Qt::EditRole, d->UI.defaultColorBack->color());

  d->addTrackSource(item);
  qtUtil::resizeColumnsToContents(d->UI.sourceColors);
}

//-----------------------------------------------------------------------------
void vsTrackColorDialog::removeSources()
{
  QTE_D(vsTrackColorDialog);
  qDeleteAll(d->UI.sourceColors->selectedItems());
}

//-----------------------------------------------------------------------------
void vsTrackColorDialog::updateSourceButtonsState()
{
  QTE_D(vsTrackColorDialog);

  const bool canRemove = d->UI.sourceColors->selectedItems().count();
  d->UI.removeSource->setEnabled(canRemove);
}
