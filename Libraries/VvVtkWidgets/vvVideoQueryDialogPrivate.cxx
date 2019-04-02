/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvVideoQueryDialogPrivate.h"

#include <vgUnixTime.h>

Q_DECLARE_METATYPE(vgTimeStamp)

//-----------------------------------------------------------------------------
vvVideoQueryDialogPrivate::vvVideoQueryDialogPrivate(
  vvVideoQueryDialog* q, vvQueryVideoPlayer* player)
  : Player(player), q_ptr(q)
{
}

//-----------------------------------------------------------------------------
vvVideoQueryDialogPrivate::~vvVideoQueryDialogPrivate()
{
}

//-----------------------------------------------------------------------------
vvDescriptorInfoTree* vvVideoQueryDialogPrivate::currentAvailableTree() const
{
  switch (this->UI.selectionTabs->currentIndex())
    {
    case 1:
      return this->UI.regionDescriptors;
    default:
      return this->UI.trackDescriptors;
    }
}

//-----------------------------------------------------------------------------
QList<qint64> vvVideoQueryDialogPrivate::selectedDescriptorIds(
  vvDescriptorInfoTree* tree) const
{
  tree || (tree = this->currentAvailableTree());
  return tree->descriptorIds(tree->selectedItems(), true);
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* vvVideoQueryDialogPrivate::createKeyframeItem(
  vtkIdType id) const
{
  if (!this->Keyframes.contains(id))
    return 0;

  QTreeWidgetItem* item = new QTreeWidgetItem;
  vvDescriptorInfoTree::setItemId(item, id);
  vvDescriptorInfoTree::setItemType(item, KeyframeItem);

  this->updateKeyframeItem(item, id, this->Keyframes[id]);
  return item;
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialogPrivate::setEventSelectionState(
  vtkIdType id, bool state)
{
  vtkVgEvent* event = this->Player->event(id);
  if (event)
    {
    if (state)
      {
      event->SetDisplayFlags(SelectedFlag);
      event->SetCustomColor(0.0, 1.0, 0.0);
      event->UseCustomColorOn();
      }
    else
      {
      event->SetDisplayFlags(UnselectedFlag);
      event->UseCustomColorOff();
      }
    }
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialogPrivate::setDescriptorEnabled(qint64 id, bool state)
{
  foreach (auto* const item, this->UI.trackDescriptors->descriptorItems(id))
    vvVideoQueryDialogPrivate::setItemEnabled(item, state);
  foreach (auto* const item, this->UI.regionDescriptors->descriptorItems(id))
    vvVideoQueryDialogPrivate::setItemEnabled(item, state);
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialogPrivate::setEntityVisibility(
  bool visibility, int excludeType)
{
  QSet<vtkIdType> updatedEvents;

  // Block signals while we change check states, so we aren't updating style
  // group parents multiple times
  qtScopedBlockSignals bs(this->UI.trackDescriptors);

  // Loop over all tree items in both trees
  this->setEntityVisibility(this->UI.trackDescriptors, visibility,
                            excludeType, updatedEvents);
  this->setEntityVisibility(this->UI.regionDescriptors, visibility,
                            excludeType, updatedEvents);

  // Obviously, not all descriptors will be in the region tree, but track time
  // constraints mean they may not all be in the track tree either, so we must
  // now also update all descriptors that aren't in either tree
  foreach (qint64 did, this->Descriptors.keys())
    {
    vtkIdType id = static_cast<vtkIdType>(did);
    if (!updatedEvents.contains(id))
      this->Player->setEventVisibility(id, visibility);
    }
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialogPrivate::setEntityVisibility(
  QTreeWidget* tree, bool visibility, int excludeType,
  QSet<vtkIdType>& updatedEvents)
{
  const Qt::CheckState checkState =
    (visibility ? Qt::Checked : Qt::Unchecked);

  QTreeWidgetItemIterator iter(tree);
  while (*iter)
    {
    if ((*iter)->checkState(0) != checkState)
      {
      // Get item ID and type
      const vtkIdType id =
        static_cast<vtkIdType>(vvDescriptorInfoTree::itemId(*iter));
      const int type = vvDescriptorInfoTree::itemType(*iter);

      if (type != excludeType)
        {
        // Update entity visibility
        if (type == DescriptorItem)
          {
          // No need to do anything if we already updated this event
          if (!updatedEvents.contains(id))
            {
            this->Player->setEventVisibility(id, visibility);
            updatedEvents.insert(id);
            }
          }
        else if (type == TrackItem)
          {
          this->Player->setTrackVisibility(id, visibility);
          }

        // Set item check state
        (*iter)->setCheckState(0, checkState);
        (*iter)->setData(0, VisibilityRole, checkState);
        }
      }

    ++iter;
    }
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialogPrivate::scheduleParentVisibilityUpdate(
  QTreeWidgetItem* item, Qt::CheckState hint)
{
  // Queue an event to handle delayed updates, if we haven't already done so
  if (PendingVisibilityUpdates.isEmpty())
    {
    QTE_Q(vvVideoQueryDialog);
    QMetaObject::invokeMethod(q, "updateItemParentVisibilities",
                              Qt::QueuedConnection);
    }

  // Add update to pending updates list
  this->PendingVisibilityUpdates.insert(item, hint);
}

//-----------------------------------------------------------------------------
bool vvVideoQueryDialogPrivate::setTimeConstraint(int role)
{
  vgTimeStamp now = this->Player->currentTimeStamp().GetRawTimeStamp();
  return this->setTimeConstraint(role, now);
}

//-----------------------------------------------------------------------------
bool vvVideoQueryDialogPrivate::setTimeConstraint(
  int role, vgTimeStamp newValue)
{
  bool result = false;
  foreach (QTreeWidgetItem* item, this->UI.trackDescriptors->selectedItems())
    {
    if (vvDescriptorInfoTree::itemType(item) == TrackItem)
      {
      const vgTimeStamp oldValue =
        vvVideoQueryDialogPrivate::timeConstraint(item, role);
      if (oldValue != newValue)
        {
        result = true;
        vvVideoQueryDialogPrivate::setTimeConstraint(item, role, newValue);
        }
      }
    }

  return result;
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialogPrivate::createRegionWidget()
{
  QTE_Q(vvVideoQueryDialog);

  this->RegionWidget.reset(
    new vtkVgRegionWidget(this->Player->interactor(), q));

  q->connect(this->RegionWidget.data(), SIGNAL(completed()),
             q, SLOT(finalizeKeyframeEdit()));
  q->connect(this->RegionWidget.data(), SIGNAL(canceled()),
             q, SLOT(cancelKeyframeEdit()));
  q->connect(this->RegionWidget.data(),
             SIGNAL(statusMessageAvailable(qtStatusSource, QString)),
             &this->StatusManager,
             SLOT(setStatusText(qtStatusSource, QString)));
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialogPrivate::editKeyframe(vtkIdType id)
{
  if (this->Keyframes.contains(id))
    {
    const vgRegionKeyframe& kf = this->Keyframes[id];
    this->EditKeyframeId = id;

    this->createRegionWidget();
    this->Player->setKeyframeVisibility(false);
    this->Player->setPlaybackEnabled(false);

    // Convert to VTK coords
    QRect r = kf.Region;
    int vidHeight = this->Player->videoHeight();
    int bottom = r.bottom();
    r.setBottom(vidHeight - r.top() - 1);
    r.setTop(vidHeight - bottom - 1);

    this->RegionWidget->setRect(r);
    }
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialogPrivate::setItemEnabled(
  QTreeWidgetItem* item, bool state, QTreeWidget* tree)
{
  QVariant brush;
  if (!state)
    {
    tree || (tree = item->treeWidget());
    QPalette palette(tree->palette());
    brush = palette.brush(QPalette::Disabled, QPalette::WindowText);
    }

  for (int i = 0, k = item->columnCount(); i < k; ++i)
    item->setData(i, Qt::ForegroundRole, brush);
}

//-----------------------------------------------------------------------------
vgTimeStamp vvVideoQueryDialogPrivate::timeConstraint(
  QTreeWidgetItem* item, int role, vgTimeStamp defaultValue)
{
  const QVariant data = item->data(0, role);
  if (data.isValid() && data.canConvert<vgTimeStamp> ())
    return data.value<vgTimeStamp>();
  return defaultValue;
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialogPrivate::setTimeConstraint(
  QTreeWidgetItem* item, int role, vgTimeStamp value)
{
  int column =
    (role == StartTimeConstraintRole ? StartTimeColumn : EndTimeColumn);
  if (value.IsValid())
    {
    item->setText(column, vgUnixTime(value.Time).timeString());
    item->setData(0, role, QVariant::fromValue<vgTimeStamp>(value));
    }
  else
    {
    item->setText(column, QString());
    item->setData(0, role, QVariant());
    }
}

//-----------------------------------------------------------------------------
void vvVideoQueryDialogPrivate::updateKeyframeItem(
  QTreeWidgetItem* item, vtkIdType id, const vgRegionKeyframe& keyframe)
{
  static const auto name = QStringLiteral("%1,%2 %3\u00d7%4");
  QRect r = keyframe.Region;
  item->setText(NameColumn, QStringLiteral("keyframe-") + QString::number(id));
  item->setText(RegionColumn, name.arg(r.left()).arg(r.top())
                                  .arg(r.width()).arg(r.height()));
  vgUnixTime time(keyframe.Time.Time);
  item->setText(TimeColumn, time.timeString());
}

//-----------------------------------------------------------------------------
bool vvVideoQueryDialogPrivate::isMetadataDescriptor(
  const vvDescriptor& descriptor)
{
  const vvDescriptorStyle::Styles descriptorStyles =
    vvDescriptorStyle::styles(descriptor);

  // Check against descriptor style map, and also against name (in case style
  // map is missing/wrong)
  bool result =
    descriptorStyles.testFlag(vvDescriptorStyle::Metadata) ||
    descriptor.DescriptorName == "metadata";
  return result;
}
