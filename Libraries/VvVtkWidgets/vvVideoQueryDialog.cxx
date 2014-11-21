/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvVideoQueryDialog.h"
#include "vvVideoQueryDialogPrivate.h"
#include "ui_videoQuery.h"

#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QSettings>

// Qt Extensions includes
#include <qtScopedValueChange.h>
#include <qtStatusManager.h>
#include <qtUtil.h>

// visgui includes
#include <vgFileDialog.h>
#include <vgUnixTime.h>

// VTK Extensions includes
#include <vtkVgEvent.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgRegionWidget.h>

#include "vvQueryVideoPlayer.h"
#include "vvTrackInfo.h"

QTE_IMPLEMENT_D_FUNC(vvVideoQueryDialog)

///////////////////////////////////////////////////////////////////////////////

//BEGIN vtkVgQtVideoQueryDialogItemFactory

//-----------------------------------------------------------------------------
class vtkVgQtVideoQueryDialogItemFactory : public vvDescriptorInfoTreeItemFactory
{
protected:
  QTE_DECLARE_PRIVATE_PTR(vvVideoQueryDialog)
  friend class vvVideoQueryDialog;

  vtkVgQtVideoQueryDialogItemFactory(vvVideoQueryDialogPrivate* d,
                                     vvDescriptorInfoTree* tree);

  virtual QTreeWidgetItem* createItem(qint64 id, const vvDescriptor&);
  virtual QTreeWidgetItem* createGroupItem(int style);

private:
  QTE_DECLARE_PRIVATE(vvVideoQueryDialog)
  QTE_DECLARE_PUBLIC(vvDescriptorInfoTree)
};

QTE_IMPLEMENT_ALIASED_D_FUNC(vtkVgQtVideoQueryDialogItemFactory,
                             vvVideoQueryDialogPrivate)

//-----------------------------------------------------------------------------
vtkVgQtVideoQueryDialogItemFactory::vtkVgQtVideoQueryDialogItemFactory(
  vvVideoQueryDialogPrivate* d, vvDescriptorInfoTree* tree)
  : vvDescriptorInfoTreeItemFactory(tree), d_ptr(d)
{
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* vtkVgQtVideoQueryDialogItemFactory::createItem(
  qint64 id, const vvDescriptor& descriptor)
{
  QTE_D(vvVideoQueryDialog);
  QTE_Q(vvDescriptorInfoTree);

  // Start with item created by base implementation
  QTreeWidgetItem* item =
    vvDescriptorInfoTreeItemFactory::createItem(id, descriptor);

  // Add ID to name
  item->setText(0, QString("%2 (D%1)").arg(id).arg(item->text(0)));

  // Make checkable, if in an 'available' tree
  if (d)
    {
    const bool visible =
      d->Player->eventVisibility(static_cast<vtkIdType>(id));
    const Qt::CheckState checkState = (visible ? Qt::Checked : Qt::Unchecked);
    item->setCheckState(0, checkState);
    item->setData(0, VisibilityRole, checkState);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

    // Queue an event (so it runs after the item is added to the tree) to check
    // if we must update the parent check state
    d->scheduleParentVisibilityUpdate(item, checkState);
    }

  // Set availability state
  if (d && d->SelectedDescriptors.contains(id))
    vvVideoQueryDialogPrivate::setItemEnabled(item, false, q);

  // Done
  return item;
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* vtkVgQtVideoQueryDialogItemFactory::createGroupItem(int style)
{
  QTE_D(vvVideoQueryDialog);

  // Start with item created by base implementation
  QTreeWidgetItem* item =
    vvDescriptorInfoTreeItemFactory::createGroupItem(style);

  // Make checkable, if in an 'available' tree
  if (d)
    {
    item->setCheckState(0, Qt::Checked);
    item->setData(0, VisibilityRole, Qt::Checked);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    }

  // Done
  return item;
}

//END vtkVgQtVideoQueryDialogItemFactory

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvVideoQueryDialog public interface


//-----------------------------------------------------------------------------
vvVideoQueryDialog::vvVideoQueryDialog(
  vvVideoQueryDialogPrivate* d, bool useAdvancedUi,
  QWidget* parent, Qt::WindowFlags flags)
  : vvAbstractSimilarityQueryDialog(parent, flags),
    d_ptr(d)
{
  d->NewKeyframeId = 0;
  d->UseAdvancedUi = useAdvancedUi;

  // Set up UI
  d->UI.setupUi(this);

  // Add the video player to the dialog (must be created by a derived class)
  QVBoxLayout* layout = new QVBoxLayout(d->UI.videoPlayerWidget);
  layout->addWidget(d->Player);
  layout->setMargin(0);

  d->UI.trackDescriptors->setColumns(vvDescriptorInfoTree::All);
  d->UI.regionDescriptors->setColumns(vvDescriptorInfoTree::Name, false);

  // TODO: Figure out why we need to do this to get correct column headers
  d->UI.selectedDescriptors->setColumns(vvDescriptorInfoTree::All, false);
  d->UI.selectedDescriptors->setColumns(vvDescriptorInfoTree::Name |
                                        vvDescriptorInfoTree::Source);

  d->UI.trackDescriptors->header()->setSortIndicator(
    NameColumn, Qt::AscendingOrder);
  d->UI.regionDescriptors->header()->setSortIndicator(
    TimeColumn, Qt::AscendingOrder);

  d->UI.trackDescriptors->setItemFactory(
    new vtkVgQtVideoQueryDialogItemFactory(d, d->UI.trackDescriptors));
  d->UI.regionDescriptors->setItemFactory(
    new vtkVgQtVideoQueryDialogItemFactory(d, d->UI.regionDescriptors));
  d->UI.selectedDescriptors->setItemFactory(
    new vtkVgQtVideoQueryDialogItemFactory(0, d->UI.selectedDescriptors));

  qtUtil::setStandardIcons(d->UI.buttonBox);

  // Set up status bar
  d->StatusLabel = new QLabel;
  d->StatusProgress = new QProgressBar;
  d->StatusProgress->setMinimumWidth(170);
  d->UI.statusbar->addWidget(d->StatusLabel, 1);
  d->UI.statusbar->addWidget(d->StatusProgress);
  d->StatusManager.addStatusLabel(d->StatusLabel);
  d->StatusManager.addProgressBar(d->StatusProgress);

  QSettings settings;

  // Restore user options
  if (useAdvancedUi)
    {
    QVariant group =
      settings.value("GroupDescriptors", d->UI.groupByStyle->isChecked());
    d->UI.groupByStyle->setChecked(group.toBool());
    }
  else
    {
    d->UI.groupByStyle->setVisible(false);
    d->UI.useMetadata->setVisible(false);
    d->UI.trackDescriptors->setHideGroupedItems(true);
    d->UI.regionDescriptors->setHideGroupedItems(true);
    d->UI.selectedDescriptors->setHideGroupedItems(true);
    }

  // Restore geometry
  settings.beginGroup("Window/VideoQueryDialog");
  this->restoreGeometry(settings.value("geometry").toByteArray());
  d->UI.splitter->restoreState(settings.value("state").toByteArray());

  // Connect query video selection controls
  connect(d->UI.chooseVideo, SIGNAL(clicked()),
          this, SLOT(chooseQueryVideo()));
  connect(d->UI.reprocessVideo, SIGNAL(clicked()),
          this, SLOT(reprocessQueryVideo()));

  // Connect descriptor selection controls
  connect(d->UI.moveToSelected, SIGNAL(clicked()),
          this, SLOT(moveToSelected()));
  connect(d->UI.moveToAvailable, SIGNAL(clicked()),
          this, SLOT(moveToAvailable()));
  connect(d->UI.clearSelected, SIGNAL(clicked()),
          this, SLOT(clearSelected()));
  connect(d->UI.selectedDescriptors, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateSelectionControlsState()));
  connect(d->UI.selectionTabs, SIGNAL(currentChanged(int)),
          this, SLOT(updateSelectionControlsState()));

  // Connect entity visibility controls
  connect(d->UI.showAll, SIGNAL(clicked()), this, SLOT(showAll()));
  connect(d->UI.hideAll, SIGNAL(clicked()), this, SLOT(hideAll()));
  connect(d->UI.hideAllDescriptors, SIGNAL(clicked()),
          this, SLOT(hideAllDescriptors()));
  connect(d->UI.trackDescriptors,
          SIGNAL(itemChanged(QTreeWidgetItem*, int)),
          this, SLOT(itemVisibilityChanged(QTreeWidgetItem*, int)));
  connect(d->UI.regionDescriptors,
          SIGNAL(itemChanged(QTreeWidgetItem*, int)),
          this, SLOT(itemVisibilityChanged(QTreeWidgetItem*, int)));

  // Connect track interval tab controls
  connect(d->UI.trackDescriptors, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateSelectionControlsState()));
  connect(d->UI.trackDescriptors, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateTrackConstraintControlsState()));
  connect(d->UI.setStartTimeToCurrentFrame, SIGNAL(clicked()),
          this, SLOT(setStartTimeConstraintToCurrentFrame()));
  connect(d->UI.setEndTimeToCurrentFrame, SIGNAL(clicked()),
          this, SLOT(setEndTimeConstraintToCurrentFrame()));
  connect(d->UI.clearTimeConstraints, SIGNAL(clicked()),
          this, SLOT(clearTimeConstraints()));

  // Connect video region tab controls
  connect(d->UI.regionDescriptors, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateSelectionControlsState()));
  connect(d->UI.regionDescriptors, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateKeyframeControlsState()));
  connect(d->UI.addKeyframe, SIGNAL(clicked()),
          this, SLOT(addKeyframe()));
  connect(d->UI.removeKeyframes, SIGNAL(clicked()),
          this, SLOT(removeSelectedKeyframes()));
  connect(d->UI.loadKeyframes, SIGNAL(clicked()),
          this, SLOT(loadKeyframes()));
  d->UI.saveKeyframes->setEnabled(false); // \TODO not implemented yet

  // Connect to video player pick signals
  connect(d->Player, SIGNAL(pickedEvent(vtkIdType)),
          this, SLOT(selectDescriptor(vtkIdType)));
  connect(d->Player, SIGNAL(pickedTrack(vtkIdType)),
          this, SLOT(selectTrack(vtkIdType)));
  connect(d->Player, SIGNAL(pickedKeyframe(vtkIdType)),
          this, SLOT(selectKeyframe(vtkIdType)));

  // Connect to descriptor tree pick (item double-clicked) signals
  connect(d->UI.trackDescriptors,
          SIGNAL(itemActivated(QTreeWidgetItem*, int)),
          this, SLOT(selectItem(QTreeWidgetItem*, int)));
  connect(d->UI.regionDescriptors,
          SIGNAL(itemActivated(QTreeWidgetItem*, int)),
          this, SLOT(selectItem(QTreeWidgetItem*, int)));
  connect(d->UI.selectedDescriptors,
          SIGNAL(itemActivated(QTreeWidgetItem*, int)),
          this, SLOT(selectItem(QTreeWidgetItem*, int)));

  // Disable controls until initialized
  this->setSelectionControlsEnabled(false);

  // Disallow accept with nothing selected
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

//-----------------------------------------------------------------------------
vvVideoQueryDialog::~vvVideoQueryDialog()
{
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::initialize(vgTimeStamp initialTime)
{
  QTE_D(vvVideoQueryDialog);

  foreach (vvTrackInfo ti, vvTrackInfo::trackTypes())
    {
    d->Player->setTrackTypeColor(ti.Type, ti.PenColor.toQColor());
    }

  d->Player->initialize(true);
  d->InitialTime = initialTime;
}

//-----------------------------------------------------------------------------
std::vector<vvDescriptor> vvVideoQueryDialog::selectedDescriptors() const
{
  QTE_D_CONST(vvVideoQueryDialog);

  std::vector<vvDescriptor> selected;
  QSet<vvTrackId> selectedTracks;

  // Add selected descriptors and build list of tracks for which descriptors
  // are selected
  foreach (qint64 id, d->SelectedDescriptors)
    {
    const vvDescriptor& descriptor = d->Descriptors[id];
    selected.push_back(descriptor);
    size_t k = descriptor.TrackIds.size();
    while (k--)
      {
      selectedTracks.insert(descriptor.TrackIds[k]);
      }
    }

  if (d->UI.useMetadata->isChecked())
    {
    // Add matching metadata descriptors
    foreach (const vvDescriptor& mdd, d->MetadataDescriptors)
      {
      if (mdd.TrackIds.empty())
        {
        // Trackless metadata descriptors are always considered matching
        selected.push_back(mdd);
        // Continue with next metadata descriptor
        continue;
        }
      size_t k = mdd.TrackIds.size();
      while (k--)
        {
        if (selectedTracks.contains(mdd.TrackIds[k]))
          {
          // Descriptor matches at least one track for which other descriptors
          // were selected
          selected.push_back(mdd);
          // Continue with next metadata descriptor
          break;
          }
        }
      }
    }

  return selected;
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::setSelectedDescriptors(
  const std::vector<vvDescriptor>& descriptors)
{
  QTE_D(vvVideoQueryDialog);

  d->DeferredSelectedDescriptors = descriptors;
}

//-----------------------------------------------------------------------------
std::vector<vvTrack> vvVideoQueryDialog::selectedTracks() const
{
  QTE_D_CONST(vvVideoQueryDialog);

  std::vector<vvTrack> relevantTracks;
  if (d->Tracks.empty())
    {
    return relevantTracks;
    }

  // build list of tracks for which descriptors are selected
  QSet<vvTrackId> selectedTracks;
  foreach (qint64 id, d->SelectedDescriptors)
    {
    const vvDescriptor& descriptor = d->Descriptors[id];
    size_t k = descriptor.TrackIds.size();
    while (k--)
      {
      selectedTracks.insert(descriptor.TrackIds[k]);
      }
    }

  foreach (const vvTrack& track, d->Tracks)
    {
    if (selectedTracks.contains(track.Id))
      {
      relevantTracks.push_back(track);
      }
    }

  return relevantTracks;
}

//-----------------------------------------------------------------------------
int vvVideoQueryDialog::exec()
{
  QTE_D(vvVideoQueryDialog);

  // Wait for user to accept or cancel
  int result = vvAbstractSimilarityQueryDialog::exec();

  // Save user options
  QSettings settings;

  if (d->UseAdvancedUi)
    {
    settings.setValue("GroupDescriptors", d->UI.groupByStyle->isChecked());
    }

  // Save geometry
  settings.beginGroup("Window/VideoQueryDialog");
  settings.setValue("geometry", this->saveGeometry());
  settings.setValue("state", d->UI.splitter->saveState());

  // Done
  return result;
}

//END vvVideoQueryDialog public interface

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvVideoQueryDialog query formulation handling

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::setQueryTracksAndDescriptors(
  QList<vvDescriptor> descriptors, QList<vvTrack> tracks)
{
  QTE_D(vvVideoQueryDialog);

  // Extract metadata descriptors
  d->MetadataDescriptors.clear();
  QList<vvDescriptor>::iterator iter = descriptors.begin();
  while (iter != descriptors.end())
    {
    if (vvVideoQueryDialogPrivate::isMetadataDescriptor(*iter))
      {
      d->MetadataDescriptors.append(*iter);
      iter = descriptors.erase(iter);
      continue;
      }
    ++iter;
    }

  d->Tracks = tracks;

  // Update video player (only set tracks if non-empty)
  if (!tracks.isEmpty())
    {
    d->Player->setTracks(tracks);
    }
  d->Player->setDescriptors(descriptors, !tracks.isEmpty());

  // Create representation for selected events
  d->SelectedEventRepresentation =
    vtkSmartPointer<vtkVgEventRegionRepresentation>::New();

  d->SelectedEventRepresentation->SetRegionZOffset(1.5);
  d->SelectedEventRepresentation->SetDisplayMask(SelectedFlag);

  d->Player->addEventRepresentation(d->SelectedEventRepresentation);

  // Update internal descriptor map and assign to trees
  // NOTE: For translating between these and the events in the video player,
  //       we are basically relying on the video player to assign the events
  //       ID's equal to their list index
  d->Descriptors.clear();
  for (int i = 0, k = descriptors.count(); i < k; ++i)
    d->Descriptors.insert(i, descriptors[i]);
  d->UI.trackDescriptors->setDescriptors(d->Descriptors);
  d->UI.regionDescriptors->setDescriptors(d->Descriptors);
  d->UI.selectedDescriptors->setDescriptors(d->Descriptors);

  // Fill 'available' descriptor trees
  this->fillTrackDescriptors();
  this->fillRegionDescriptors();

  // If we have deferred-selected descriptors, try to select them now
  QList<qint64> selectIds;
  size_t n = d->DeferredSelectedDescriptors.size();
  while (n--)
    {
    const vvDescriptor& dd = d->DeferredSelectedDescriptors[n];
    const vtkIdType id = d->Player->descriptorEventId(dd);
    if (d->Descriptors.contains(id))
      selectIds.append(id);
    }
  this->moveToSelected(selectIds);
  d->DeferredSelectedDescriptors.clear();

  // Now that the player's video has loaded (which doesn't happen until we
  // provide it descriptors, which we did above), apply an initial seek, if one
  // was requested
  if (d->InitialTime.IsValid())
    {
    d->Player->seekTo(d->InitialTime.Time);

    // Reset so we don't seek again if descriptors change
    d->InitialTime = vgTimeStamp();
    }

  // Done; we now have descriptors, so let the user select them
  this->setSelectionControlsEnabled(true);
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::clearQueryDescriptors()
{
  QTE_D(vvVideoQueryDialog);

  d->Tracks.clear();
  d->Player->setDescriptors(QList<vvDescriptor>(), false);
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  this->setSelectionControlsEnabled(false);
}


//END vvVideoQueryDialog query formulation handling

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvVideoQueryDialog entity visibility

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::showAll()
{
  QTE_D(vvVideoQueryDialog);
  d->setEntityVisibility(true);
  d->Player->update();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::hideAll()
{
  QTE_D(vvVideoQueryDialog);
  d->setEntityVisibility(false);
  d->Player->update();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::hideAllDescriptors()
{
  QTE_D(vvVideoQueryDialog);
  d->setEntityVisibility(false, TrackItem);
  d->Player->update();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::updateItemParentVisibilities()
{
  QTE_D(vvVideoQueryDialog);

  typedef QHash<QTreeWidgetItem*, Qt::CheckState>::const_iterator Iterator;

  QHash<QTreeWidgetItem*, Qt::CheckState> groupUpdates;

  // Identify descriptor group items that need to be updates
  foreach_iter (Iterator, iter, d->PendingVisibilityUpdates)
    {
    QTreeWidgetItem* parent = iter.key()->parent();
    Q_ASSERT(parent);
    if (vvDescriptorInfoTree::itemType(parent) == DescriptorStyleGroup)
      groupUpdates.insert(parent, iter.value());
    }
  d->PendingVisibilityUpdates.clear();

  // Update groups
  foreach_iter (Iterator, iter, groupUpdates)
    {
    QTreeWidgetItem* group = iter.key();
    qtScopedBlockSignals bs(group->treeWidget());

    // Identify correct new check state
    Qt::CheckState newCheckState = iter.value();
    foreach_child (QTreeWidgetItem* child, group)
      {
      const Qt::CheckState ccs = child->checkState(0);
      if (ccs != newCheckState)
        {
        newCheckState = Qt::PartiallyChecked;
        break;
        }
      }

    // Apply new check state
    group->setCheckState(0, newCheckState);
    group->setData(0, VisibilityRole, newCheckState);
    }
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::itemVisibilityChanged(
  QTreeWidgetItem* item, int column)
{
  if (column != 0)
    return;

  // Check if the check state has changed, and bail otherwise
  const Qt::CheckState checkState = item->checkState(0);
  const QVariant visibilityData = item->data(0, VisibilityRole);
  if (visibilityData.isValid() && visibilityData.toInt() == checkState)
    return;

  // Get ID, type and new visibility state
  const vtkIdType id =
    static_cast<vtkIdType>(vvDescriptorInfoTree::itemId(item));
  const int type = vvDescriptorInfoTree::itemType(item);
  const bool visibility = (checkState == Qt::Checked);

  // Ignore keyframe items (they are not checkable)
  if (type == KeyframeItem)
    return;

  QTE_D(vvVideoQueryDialog);

  // If we got this far, the check state has been changed; update the
  // visibility data...
  item->setData(0, VisibilityRole, checkState);

  // ...and entity visibility
  if (type == TrackItem)
    {
    // Update track visibility
    d->Player->setTrackVisibility(id, visibility);
    }
  else if (type == DescriptorItem)
    {
    // Update descriptor visibility
    d->Player->setEventVisibility(id, visibility);

    // Update parent check state (if parent is a style group)
    d->scheduleParentVisibilityUpdate(item, checkState);

    // Find other instances of this descriptor item and update them
    qtScopedBlockSignals bst(d->UI.trackDescriptors);
    qtScopedBlockSignals bsr(d->UI.regionDescriptors);
    QList<QTreeWidgetItem*> peers;
    peers.append(d->UI.trackDescriptors->descriptorItems(id));
    peers.append(d->UI.regionDescriptors->descriptorItems(id));
    foreach (QTreeWidgetItem* peer, peers)
      {
      peer->setCheckState(0, checkState);
      peer->setData(0, VisibilityRole, checkState);
      d->scheduleParentVisibilityUpdate(peer, checkState);
      }
    }

  // Update check state of children
  foreach_child (QTreeWidgetItem* child, item)
    child->setCheckState(0, checkState);

  d->Player->update();
}

//END vvVideoQueryDialog entity visibility

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvVideoQueryDialog user interface

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::setSelectionControlsEnabled(bool state)
{
  QTE_D(vvVideoQueryDialog);

  d->Player->setEnabled(state);
  d->UI.selectionTabs->setEnabled(state);
  d->UI.selectedGroup->setEnabled(state);
  d->UI.chooseVideo->setEnabled(state);
  d->UI.reprocessVideo->setEnabled(state);
  this->updateSelectionControlsState();
  this->updateTrackConstraintControlsState();
  this->updateKeyframeControlsState();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::moveToSelected(QList<qint64> ids)
{
  QTE_D(vvVideoQueryDialog);

  if (ids.isEmpty())
    return;

  QTreeWidgetItem* item;
  QList<qint64> newSelectedIds;
  foreach (qint64 id, ids)
    {
    if (d->SelectedDescriptors.contains(id))
      continue;

    // Add newly selected descriptor
    d->SelectedDescriptors.insert(id);
    newSelectedIds.append(id);

    // Show as selected in video player
    d->setEventSelectionState(static_cast<vtkIdType>(id), true);

    // Mark items in 'available' trees as selected
    d->setDescriptorEnabled(id, false);
    }

  if (!newSelectedIds.isEmpty())
    {
    // Add newly selected descriptors to selected tree
    d->UI.selectedDescriptors->addDescriptorItems(newSelectedIds);

    // Resize columns to new contents
    qtUtil::resizeColumnsToContents(d->UI.selectedDescriptors);
    }

  // Generate list of the newly added items
  QList<QTreeWidgetItem*> newSelectedItems;
  foreach (qint64 id, ids)
    {
    newSelectedItems.append(
      d->UI.selectedDescriptors->descriptorItems(id));
    }

  // Update the item selection states so exactly the new items are selected
  d->UI.selectedDescriptors->clearSelection();
  foreach (item, newSelectedItems)
    item->setSelected(true);

  // Something is selected; allow accepting now
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::moveToSelected()
{
  QTE_D(vvVideoQueryDialog);
  this->moveToSelected(d->selectedDescriptorIds());
  d->Player->update();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::moveToAvailable()
{
  QTE_D(vvVideoQueryDialog);

  QList<qint64> selectedIds =
    d->selectedDescriptorIds(d->UI.selectedDescriptors);

  if (selectedIds.isEmpty())
    return;

  foreach (qint64 id, selectedIds)
    {
    // Remove from internal set and reset display in video player
    d->SelectedDescriptors.remove(id);
    d->setEventSelectionState(static_cast<vtkIdType>(id), false);

    // Mark items as available for selection
    d->setDescriptorEnabled(id, true);
    }

  // Update 'selected' tree
  d->UI.selectedDescriptors->setDescriptorItems(
    d->SelectedDescriptors.toList());

  this->updateSelectionControlsState();

  // Disallow selected unless something still selected
  bool allowAccept = !d->SelectedDescriptors.isEmpty();
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(allowAccept);

  d->Player->update();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::clearSelected()
{
  QTE_D(vvVideoQueryDialog);

  foreach (qint64 id, d->SelectedDescriptors)
    {
    // Reset display in video player and mark items as available for selection
    d->setEventSelectionState(static_cast<vtkIdType>(id), false);
    d->setDescriptorEnabled(id, true);
    }

  // Clear internal set and 'selected' tree
  d->SelectedDescriptors.clear();
  d->UI.selectedDescriptors->clear();

  this->updateSelectionControlsState();

  // Disallow accept with nothing selected
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

  d->Player->update();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::updateSelectionControlsState()
{
  QTE_D(vvVideoQueryDialog);

  QList<qint64> idsToSelect = d->selectedDescriptorIds();
  QList<qint64>::iterator iter = idsToSelect.begin();
  while (iter != idsToSelect.end())
    {
    if (d->SelectedDescriptors.contains(*iter))
      iter = idsToSelect.erase(iter);
    else
      ++iter;
    }

  bool enableMoveToSelected = !idsToSelect.isEmpty();
  bool enableMoveToAvailable =
    !d->selectedDescriptorIds(d->UI.selectedDescriptors).isEmpty();
  bool enableClearSelected = !d->SelectedDescriptors.isEmpty();

  d->UI.moveToSelected->setEnabled(enableMoveToSelected);
  d->UI.moveToAvailable->setEnabled(enableMoveToAvailable);
  d->UI.clearSelected->setEnabled(enableClearSelected);
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::updateTrackConstraintControlsState()
{
  QTE_D(vvVideoQueryDialog);

  QList<QTreeWidgetItem*> selectedItems =
    d->UI.trackDescriptors->selectedItems();

  bool enable = !selectedItems.isEmpty();
  foreach (QTreeWidgetItem* item, selectedItems)
    enable = enable && (vvDescriptorInfoTree::itemType(item) == TrackItem);

  d->UI.setStartTimeToCurrentFrame->setEnabled(enable);
  d->UI.setEndTimeToCurrentFrame->setEnabled(enable);
  d->UI.clearTimeConstraints->setEnabled(enable);
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::updateKeyframeControlsState()
{
  QTE_D(vvVideoQueryDialog);

  QList<QTreeWidgetItem*> selectedItems =
    d->UI.regionDescriptors->selectedItems();

  bool enableRemove = false;
  foreach (QTreeWidgetItem* item, selectedItems)
    {
    const int type = vvDescriptorInfoTree::itemType(item);
    enableRemove = enableRemove || (type == KeyframeItem);
    }

  d->UI.removeKeyframes->setEnabled(enableRemove);
  // \TODO not implemented yet
  // d->UI.saveKeyframes->setEnabled(!d->Keyframes.isEmpty());
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::setStartTimeConstraintToCurrentFrame()
{
  QTE_D(vvVideoQueryDialog);

  if (d->setTimeConstraint(StartTimeConstraintRole))
    this->updateTrackMatchingDescriptors();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::setEndTimeConstraintToCurrentFrame()
{
  QTE_D(vvVideoQueryDialog);

  if (d->setTimeConstraint(EndTimeConstraintRole))
    this->updateTrackMatchingDescriptors();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::clearTimeConstraints()
{
  QTE_D(vvVideoQueryDialog);

  bool rsc = d->setTimeConstraint(StartTimeConstraintRole, vgTimeStamp());
  bool rec = d->setTimeConstraint(EndTimeConstraintRole, vgTimeStamp());
  if (rsc || rec)
    this->updateTrackMatchingDescriptors();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::addKeyframe()
{
  QTE_D(vvVideoQueryDialog);

  d->EditKeyframeId = d->NewKeyframeId;
  d->EditKeyframeTime = d->Player->currentTimeStamp().GetRawTimeStamp();

  // Check if we are replacing an existing keyframe
  typedef QHash<vtkIdType, vgRegionKeyframe>::const_iterator Iterator;
  foreach_iter (Iterator, iter, d->Keyframes)
    {
    // Compare time only; loaded keyframes might have frame number, but the
    // timestamp we get from the video player doesn't, so we can't rely on
    // vgTimeStamp::operator==
    if (qFuzzyCompare(iter.value().Time.Time, d->EditKeyframeTime.Time))
      {
      d->EditKeyframeId = iter.key();
      d->Player->setKeyframeVisibility(false);
      break;
      }
    }

  d->Player->setPlaybackEnabled(false);
  d->Player->setCursor(Qt::CrossCursor);
  d->createRegionWidget();
  d->RegionWidget->begin();
  connect(d->RegionWidget.data(), SIGNAL(beginningManipulation()),
          this, SLOT(unsetKeyframeEditCursor()));
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::unsetKeyframeEditCursor()
{
  QTE_D(vvVideoQueryDialog);
  d->Player->unsetCursor();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::finalizeKeyframeEdit()
{
  QTE_D(vvVideoQueryDialog);

  const vtkIdType kfid = d->EditKeyframeId;
  bool isNewKeyframe = !d->Keyframes.contains(kfid);

  // Add or update keyframe
  QTreeWidgetItem* item;
  vgRegionKeyframe& kf = d->Keyframes[kfid];
  kf.Region = d->RegionWidget->rect();

  // Convert from VTK coords
  int vidHeight = d->Player->videoHeight();
  int bottom = kf.Region.bottom();
  kf.Region.setBottom(vidHeight - kf.Region.top() - 1);
  kf.Region.setTop(vidHeight - bottom - 1);

  if (isNewKeyframe)
    {
    kf.Time = d->EditKeyframeTime;
    ++d->NewKeyframeId;
    item = d->createKeyframeItem(kfid);
    d->UI.regionDescriptors->addTopLevelItem(item);
    }
  else
    {
    item = d->UI.regionDescriptors->findItem(KeyframeItem, kfid);
    Q_ASSERT(item);
    vvVideoQueryDialogPrivate::updateKeyframeItem(item, kfid, kf);
    }

  // Update trees and video player, and recompute matches
  d->Player->setKeyframes(d->Keyframes);
  this->updateKeyframeMatchingDescriptors();
  this->updateKeyframeControlsState();

  // Turn off editing
  this->cancelKeyframeEdit();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::cancelKeyframeEdit()
{
  QTE_D(vvVideoQueryDialog);

  d->RegionWidget.reset();
  d->Player->unsetCursor();
  d->Player->setKeyframeVisibility(true);
  d->Player->setPlaybackEnabled(true);
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::removeSelectedKeyframes()
{
  QTE_D(vvVideoQueryDialog);

  bool mustUpdate = false;

  // Consider selected tree items
  foreach (QTreeWidgetItem* item, d->UI.regionDescriptors->selectedItems())
    {
    // Is it a keyframe?
    const int type = vvDescriptorInfoTree::itemType(item);
    if (type == KeyframeItem)
      {
      const vtkIdType id =
        static_cast<vtkIdType>(vvDescriptorInfoTree::itemId(item));
      if (d->Keyframes.contains(id))
        {
        // Remove from tree and internal map
        mustUpdate = true;
        d->Keyframes.remove(id);
        delete item;
        }
      }
    }

  // If removals were made, update video player and recompute matches
  if (mustUpdate)
    {
    d->Keyframes.isEmpty() && (d->NewKeyframeId = 0);
    d->Player->setKeyframes(d->Keyframes);
    this->updateKeyframeMatchingDescriptors();
    this->updateKeyframeControlsState();
    }
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::loadKeyframes()
{
  QTE_D(vvVideoQueryDialog);

  QString fileName = vgFileDialog::getOpenFileName(
                       this, "Load keyframes...", QString(),
                       "All Keyframe Files (*.vvk *.xml);;"
                       "VisGUI Video region Keyframes (*.vvk);;"
                       "Test Harness Query XML (*.xml);;"
                       "All Files (*)");
  if (fileName.isEmpty())
    return;

  QList<vgRegionKeyframe> newKeyframes =
    vgRegionKeyframe::readFromFile(fileName);

  // Resolve frame numbers to times, if we were given only frame numbers
  int matchErrors = 0;
  QList<vgRegionKeyframe>::iterator iter = newKeyframes.begin();
  while (iter != newKeyframes.end())
    {
    if (!iter->Time.HasTime())
      {
      if (!d->Player->timeFromFrameNumber(iter->Time))
        {
        qDebug() << "Failed to find time for frame number"
                 << iter->Time.FrameNumber;
        ++matchErrors;
        iter = newKeyframes.erase(iter);
        continue;
        }
      }
    ++iter;
    }

  if (matchErrors)
    {
    QString msg = (matchErrors > 1
                   ? "%1 keyframes could not be matched to a video time"
                     " and have been discarded."
                   : "%1 keyframe could not be matched to a video time"
                     " and has been discarded.");
    QMessageBox::warning(this, "Errors occurred", msg.arg(matchErrors));
    }

  // Did we read anything successfully? (Check after dropping any frame numbers
  // that could not be converted!)
  if (newKeyframes.isEmpty())
    return;

  // Update keyframes
  d->Keyframes.clear();
  for (int i = 0, k = newKeyframes.count(); i < k; ++i)
    d->Keyframes.insert(i, newKeyframes[i]);
  d->Player->setKeyframes(d->Keyframes);
  d->NewKeyframeId = d->Keyframes.count();

  // Update tree
  this->fillRegionDescriptors();
}

//END vvVideoQueryDialog user interface

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvVideoQueryDialog tree filling

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::fillTrackDescriptors()
{
  QTE_D(vvVideoQueryDialog);

  qtDelayTreeSorting ds(d->UI.trackDescriptors);

  // Clear tree
  d->UI.trackDescriptors->clear();

  // Populate new tracks
  QHash<vvTrackId, qint64> trackIdMap;
  QHash<vvTrackId, QTreeWidgetItem*> trackItems;
  foreach (vtkIdType tid, d->Player->trackIds())
    {
    const vvTrackId t = d->Player->trackId(tid);
    const QString name("Track %1:%2");

    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(NameColumn, name.arg(t.Source).arg(t.SerialNumber));
    item->setCheckState(0, Qt::Checked);
    item->setData(0, VisibilityRole, Qt::Checked);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    vvDescriptorInfoTree::setItemType(item, TrackItem);
    vvDescriptorInfoTree::setItemId(item, tid);

    trackItems.insert(t, item);
    trackIdMap.insert(t, tid);
    }
  d->UI.trackDescriptors->addTopLevelItems(trackItems.values());

  // Check if we have descriptors with no associated track(s)
  bool haveTracklessDescriptors = false;
  foreach (const vvDescriptor& descriptor, d->Descriptors)
    {
    if (descriptor.TrackIds.size() == 0)
      {
      haveTracklessDescriptors = true;
      break;
      }
    }

  // Create parent for descriptors with no associated track(s), if needed
  if (haveTracklessDescriptors)
    {
    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(NameColumn, "(no track)");
    vvDescriptorInfoTree::setItemType(item, TrackItem);
    vvDescriptorInfoTree::setItemId(item, TracklessId);
    d->UI.trackDescriptors->addTopLevelItem(item);
    }

  // Populate descriptors-by-track lists (don't apply time constraints yet)
  d->DescriptorsByTrackId.clear();
  typedef QHash<qint64, vvDescriptor>::const_iterator DescriptorIterator;
  foreach_iter (DescriptorIterator, iter, d->Descriptors)
    {
    const std::vector<vvTrackId>& tracks = iter.value().TrackIds;
    size_t n = tracks.size();
    if (n)
      {
      while (n--)
        {
        qint64 id = trackIdMap.value(tracks[n], -1);
        Q_ASSERT(id != -1);
        d->DescriptorsByTrackId[id].insert(iter.key());
        }
      }
    else
      {
      d->DescriptorsByTrackId[TracklessId].insert(iter.key());
      }
    }

  // Fill matching descriptors
  this->updateTrackMatchingDescriptors();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::fillRegionDescriptors()
{
  QTE_D(vvVideoQueryDialog);

  qtDelayTreeSorting ds(d->UI.regionDescriptors);

  // Clear tree
  d->UI.regionDescriptors->clear();

  // Populate new keyframes
  QList<QTreeWidgetItem*> newItems;
  foreach (vtkIdType id, d->Keyframes.keys())
    newItems.append(d->createKeyframeItem(id));
  d->UI.regionDescriptors->addTopLevelItems(newItems);

  // Fill matching descriptors
  this->updateKeyframeMatchingDescriptors();
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::updateTrackMatchingDescriptors()
{
  QTE_D(vvVideoQueryDialog);

  // Populate track items, taking into account time constraints if set
  typedef QHash<qint64, QSet<qint64> >::const_iterator Iterator;
  foreach_iter (Iterator, titer, d->DescriptorsByTrackId)
    {
    // Get track item
    QTreeWidgetItem* item =
      d->UI.trackDescriptors->findItem(TrackItem, titer.key());
    if (!item)
      continue;

    // Get associated descriptor list
    QList<qint64> descriptorsForTrack = titer.value().toList();

    // Get time constraints for track
    vgTimeStamp sc = vvVideoQueryDialogPrivate::timeConstraint(
                       item, StartTimeConstraintRole,
                       vtkVgTimeStamp(false).GetRawTimeStamp());
    vgTimeStamp ec = vvVideoQueryDialogPrivate::timeConstraint(
                       item, EndTimeConstraintRole,
                       vtkVgTimeStamp(true).GetRawTimeStamp());

    // Remove items outside of any applied time constraints
    QList<qint64>::iterator diter = descriptorsForTrack.begin();
    while (diter != descriptorsForTrack.end())
      {
      const vvDescriptor& descriptor = d->Descriptors[*diter];
      if (descriptor.Region.size())
        {
        const vgTimeStamp& st = descriptor.Region.begin()->TimeStamp;
        const vgTimeStamp& et = descriptor.Region.rbegin()->TimeStamp;
        if (et < sc || ec < st)
          {
          diter = descriptorsForTrack.erase(diter);
          continue;
          }
        }
      ++diter;
      }

    // Update descriptors for track
    d->UI.trackDescriptors->setDescriptorItems(descriptorsForTrack, item);
    }

  // Resize columns to new contents
  qtUtil::resizeColumnsToContents(d->UI.trackDescriptors);
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::updateKeyframeMatchingDescriptors()
{
  QTE_D(vvVideoQueryDialog);

  qtDelayTreeSorting ds(d->UI.regionDescriptors);
  QTreeWidgetItem* item;

  // Get matching descriptors
  typedef QHash<vtkIdType, QSet<vtkIdType> > MatchMap;
  MatchMap matchingDescriptorsByKeyframe = d->Player->regionEvents();

  // Update keyframes
  foreach (vtkIdType kfid, d->Keyframes.keys())
    {
    item = d->UI.regionDescriptors->findItem(KeyframeItem, kfid);

    if (item)
      {
      QList<qint64> kfmd;
      foreach (vtkIdType did, matchingDescriptorsByKeyframe.value(kfid))
        kfmd.append(did);

      d->UI.regionDescriptors->setDescriptorItems(kfmd, item);
      item->setExpanded(!kfmd.isEmpty());
      }
    }

  // Resize columns to new contents
  qtUtil::resizeColumnsToContents(d->UI.regionDescriptors);
}

//END vvVideoQueryDialog tree filling

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvVideoQueryDialog selection helpers

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::selectTrack(vtkIdType id)
{
  QTE_D(vvVideoQueryDialog);

  if (d->currentAvailableTree() != d->UI.trackDescriptors)
    return;

  this->selectItem(d->UI.trackDescriptors, id, TrackItem);
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::selectDescriptor(vtkIdType id)
{
  QTE_D(vvVideoQueryDialog);
  this->selectItem(d->currentAvailableTree(), id, DescriptorItem);
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::selectKeyframe(vtkIdType id)
{
  QTE_D(vvVideoQueryDialog);

  if (d->currentAvailableTree() != d->UI.regionDescriptors)
    return;

  this->selectItem(d->UI.regionDescriptors, id, KeyframeItem);
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::selectItem(
  vvDescriptorInfoTree* tree, vtkIdType id, int type)
{
  // Find the requested item
  QTreeWidgetItem* item = tree->findItem(type, id);

  // If we are in simplified UI mode, the item might be hidden, so...
  while (item && item->isHidden())
    {
    // ...find the first ancestor that is not hidden
    item = item->parent();
    }

  // Select the resulting item
  item && (tree->setCurrentItem(item), false);
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialog::selectItem(QTreeWidgetItem* item, int column)
{
  QTE_D(vvVideoQueryDialog);

  // Abort keyframe editing before jumping
  this->cancelKeyframeEdit();

  // Determine jump direction
  bool toEnd = (column == EndTimeColumn);
  vvQueryVideoPlayer::JumpDirection direction =
    (toEnd ? vvQueryVideoPlayer::JumpToEnd : vvQueryVideoPlayer::JumpToStart);

  // Get ID of the item to which we will jump
  const vtkIdType id =
    static_cast<vtkIdType>(vvDescriptorInfoTree::itemId(item));

  // Perform the jump, based on the item type
  switch (vvDescriptorInfoTree::itemType(item))
    {
    case KeyframeItem:
      d->Player->jumpToKeyframe(id);
      (column == 1) && (d->editKeyframe(id), false);
      break;
    case TrackItem:
      if (column == NameColumn || column == SourceColumn)
        {
        d->Player->jumpToTrack(id, direction);
        }
      else
        {
        int role = (toEnd ? EndTimeConstraintRole : StartTimeConstraintRole);
        vgTimeStamp tc =
          vvVideoQueryDialogPrivate::timeConstraint(item, role);
        if (tc.IsValid())
          d->Player->jumpToTrack(id, tc);
        else
          d->Player->jumpToTrack(id, direction);
        }
      break;
    default:
      d->Player->jumpToEvent(id, direction);
      break;
    }
}

//END vvVideoQueryDialog selection helpers
