/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvTrackInfoDialog.h"
#include "ui_vvTrackInfoDialog.h"

#include <qtScopedValueChange.h>
#include <qtStlUtil.h>
#include <qtUtil.h>

#include <vgCheckArg.h>

QTE_IMPLEMENT_D_FUNC(vvTrackInfoDialog)

namespace // anonymous
{

//-----------------------------------------------------------------------------
QTreeWidgetItem* createTrackItem(const vvTrack& track)
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText(0, QString::number(track.Id.Source));
  item->setText(1, QString::number(track.Id.SerialNumber));

  QString bestClassification;
  double bestScore = -1.0;
  foreach_iter (vvTrackObjectClassification::const_iterator,
                iter, track.Classification)
    {
    if (iter->second > bestScore)
      {
      bestClassification = qtString(iter->first);
      bestScore = iter->second;
      }
    else if (iter->second == bestScore)
      {
      bestClassification += " / " + qtString(iter->first);
      }
    }
  item->setText(2, bestClassification);

  return item;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vvTrackInfoDialogPrivate
{
public:
  Ui::vvTrackInfoDialog UI;
  QHash<qint64, vvTrack> tracks;
};

//-----------------------------------------------------------------------------
vvTrackInfoDialog::vvTrackInfoDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f), d_ptr(new vvTrackInfoDialogPrivate)
{
  QTE_D(vvTrackInfoDialog);
  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);

  connect(d->UI.toggleDetails, SIGNAL(toggled(bool)),
          this, SLOT(toggleDetails(bool)));
  connect(d->UI.trackTree, SIGNAL(itemSelectionChanged()),
          this, SLOT(showDetails()));

  d->UI.detailsPane->setVisible(d->UI.toggleDetails->isChecked());
  this->showDetails();
}

//-----------------------------------------------------------------------------
vvTrackInfoDialog::~vvTrackInfoDialog()
{
}

//-----------------------------------------------------------------------------
QList<vvTrack> vvTrackInfoDialog::tracks() const
{
  QTE_D_CONST(vvTrackInfoDialog);
  return d->tracks.values();
}

//-----------------------------------------------------------------------------
void vvTrackInfoDialog::setTracks(QList<vvTrack> newTracks)
{
  QTE_D(vvTrackInfoDialog);

  d->tracks.clear();
  d->UI.trackTree->clear();
  qtDelayTreeSorting dts(d->UI.trackTree);

  QList<qint64> ids;
  qint64 n = 0;
  foreach (const vvTrack& track, newTracks)
    {
    QTreeWidgetItem* item = createTrackItem(track);
    item->setData(0, Qt::UserRole, n);
    d->UI.trackTree->addTopLevelItem(item);

    d->tracks.insert(n, track);
    ids.append(n++);
    }

  qtUtil::resizeColumnsToContents(d->UI.trackTree);
}

//-----------------------------------------------------------------------------
void vvTrackInfoDialog::setTracks(std::vector<vvTrack> v)
{
  this->setTracks(qtList(v));
}

//-----------------------------------------------------------------------------
void vvTrackInfoDialog::clearTracks()
{
  this->setTracks(QList<vvTrack>());
}

//-----------------------------------------------------------------------------
void vvTrackInfoDialog::showDetails()
{
  QTE_D(vvTrackInfoDialog);

  QList<vvTrack> tracks;
  foreach (QTreeWidgetItem* item, d->UI.trackTree->selectedItems())
    {
    qint64 id = item->data(0, Qt::UserRole).toLongLong();
    if (d->tracks.contains(id))
      {
      tracks.append(d->tracks[id]);
      }
    }

  if (tracks.isEmpty())
    {
    d->UI.detailsPane->setCurrentWidget(d->UI.detailsMessage);
    d->UI.detailsSummary->setText("No track selected");
    }
  else if (tracks.count() > 1)
    {
    d->UI.detailsPane->setCurrentWidget(d->UI.detailsMessage);
    d->UI.detailsSummary->setText("Multiple tracks selected");
    }
  else
    {
    d->UI.detailsPane->setCurrentWidget(d->UI.trackDetails);
    d->UI.trackDetails->setTrack(tracks.first());
    }
}

//-----------------------------------------------------------------------------
void vvTrackInfoDialog::toggleDetails(bool detailsVisible)
{
  QTE_D(vvTrackInfoDialog);
  CHECK_ARG(d->UI.detailsPane->isVisible() != detailsVisible);

  QSize newSize = this->size();
  QSize detailsSize = d->UI.trackDetails->size();
  if (detailsVisible)
    {
    d->UI.detailsPane->setVisible(true);
    newSize.setHeight(newSize.height() + detailsSize.height());
    }
  else
    {
    newSize.setHeight(newSize.height() - detailsSize.height());
    d->UI.detailsPane->setVisible(false);
    }
  this->resize(newSize);
}
