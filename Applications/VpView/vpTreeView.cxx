// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpTreeView.h"

#include "vtkVgActivityManager.h"
#include "vtkVgEventFilter.h"
#include "vtkVgEventModel.h"
#include "vtkVgEventTypeRegistry.h"
#include "vtkVgTrackModel.h"
#include "vtkVgActivity.h"
#include "vtkVgEvent.h"
#include "vtkVgTrack.h"
#include "vtkVgTrackFilter.h"
#include "vtkVgTrackTypeRegistry.h"

#include "vgEventType.h"
#include "vgTrackType.h"

#include <QApplication>
#include <QTreeWidget>
#include <QMenu>
#include <QContextMenuEvent>

//-----------------------------------------------------------------------------
class SignalBlocker
{
  vpTreeView* tree;

public:
  SignalBlocker(vpTreeView* t) : tree(t)
    {
    tree->blockSignals(true);
    tree->model()->blockSignals(true);
    }

  ~SignalBlocker()
    {
    tree->model()->blockSignals(false);
    tree->blockSignals(false);

    // Make sure an update occurs when event processing is done. This would
    // normally be triggered by the dataChanged() signal emitted from the model.
    tree->scheduleDelayedItemsLayout();
    }
};

//-----------------------------------------------------------------------------
class vpTreeView::TreeWidgetItem : public QTreeWidgetItem
{
  vpTreeView* Tree;

public:
  TreeWidgetItem(vpTreeView* parent, const QStringList& strings)
    : QTreeWidgetItem(strings), Tree(parent)
    {}

private:
  virtual bool operator<(const QTreeWidgetItem& other) const
    {
    int sortData = vpTreeView::IDR_ItemSortValsStart + Tree->GetSortType();
    switch (Tree->GetSortType())
      {
      case ST_Id:
        return this->data(0, sortData).toInt() <
               other.data(0, sortData).toInt();

      case ST_Name:
        return this->data(0, sortData).toString() <
               other.data(0, sortData).toString();

      case ST_Normalcy:
      case ST_Saliency:
      case ST_Probability:
      case ST_Length:
        return this->data(0, sortData).toReal() <
               other.data(0, sortData).toReal();
      }

    return QTreeWidgetItem::operator<(other);
    }

  virtual QVariant data(int column, int role) const
    {
    switch (role)
      {
      case Qt::ToolTipRole:
        {
        // Show the rank of top level items in the tree.
        if (!this->parent())
          {
          return QString("%1 of %2")
                 .arg(this->Tree->indexFromItem(
                        const_cast<TreeWidgetItem*>(this)).row() + 1)
                 .arg(this->Tree->topLevelItemCount());
          }
        }
      }
    return QTreeWidgetItem::data(column, role);
    }
};

//-----------------------------------------------------------------------------
vpTreeView::vpTreeView(QWidget* p)
  : QTreeWidget(p), SortType(ST_Id),
    ShowExcludedItems(false), ShowUncheckedItems(true)
{
  this->setColumnCount(1);

  this->setHeaderHidden(true);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);

  // this is needed to get hover events
  this->setMouseTracking(true);

  connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
          this, SLOT(OnItemChanged(QTreeWidgetItem*)));
}

//-----------------------------------------------------------------------------
vpTreeView::~vpTreeView()
{
}

//-----------------------------------------------------------------------------
void vpTreeView::Initialize(vtkVgActivityManager* activityManager,
                            vtkVgEventModel* eventModel,
                            vtkVgTrackModel* trackModel,
                            vtkVgEventFilter* eventFilter,
                            vtkVgTrackFilter* trackFilter,
                            vtkVgEventTypeRegistry* eventTypes,
                            vtkVgTrackTypeRegistry* trackTypes)
{
  this->ActivityManager = activityManager;
  this->EventModel = eventModel;
  this->TrackModel = trackModel;

  this->TrackFilter = trackFilter;
  this->EventFilter = eventFilter;
  this->TrackTypeRegistry = trackTypes;
  this->EventTypeRegistry = eventTypes;

  // Clear the tree.
  this->clear();
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* vpTreeView::CreateItem(ItemTypeEnum type, const char* name,
                                        int id)
{
  int displayId = type == ItemType::SceneElement
                    ? this->TrackModel->GetSceneElementIdForTrack(id)
                    : id;

  QStringList strings(QString("%1-%2").arg(name).arg(displayId));

  TreeWidgetItem* item = new TreeWidgetItem(this, strings);

  item->setData(0, IDR_ItemType, type);
  item->setData(0, IDR_ItemId, id);
  item->setData(0, IDR_ItemSortValsStart + ST_Id, id);
  item->setData(0, IDR_ItemSortValsStart + ST_Name, name);

  return item;
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* vpTreeView::CreateItem(vtkVgTrack* track, bool isFseTrack)
{
  int type = track->GetType();

  QTreeWidgetItem* item =
    this->CreateItem(isFseTrack ? ItemType::SceneElement : ItemType::Track,
                     type == -1
                       ? "track"
                       : this->TrackTypeRegistry->GetType(type).GetName(),
                     track->GetId());

  vtkVgTimeStamp startFrame = track->GetStartFrame();
  vtkVgTimeStamp endFrame = track->GetEndFrame();

  double length;
  if (startFrame.HasTime() && endFrame.HasTime())
    {
    length = endFrame.GetTime() - startFrame.GetTime();
    }
  else
    {
    length = static_cast<double>(endFrame.GetFrameNumber() -
                                 startFrame.GetFrameNumber());
    }

  // Store the track length for sorting purposes
  // TODO: Update this if the track changes
  item->setData(0, IDR_ItemSortValsStart + ST_Length, length);
  return item;
}

//-----------------------------------------------------------------------------
bool vpTreeView::RefreshItem(QTreeWidgetItem* item, bool isTopLevel)
{
  bool on = false;
  bool shown = false;

  int id = item->data(0, IDR_ItemId).toInt();
  bool sticky = item->data(0, IDR_ItemSticky).toBool();

  switch (item->data(0, IDR_ItemType).toInt())
    {
    case ItemType::Activity:
      {
      on = this->ActivityManager->GetActivityDisplayState(id);
      vtkVgActivity* a = this->ActivityManager->GetActivity(id);
      shown = !this->ActivityManager->ActivityIsFiltered(a) &&
              this->ActivityManager->GetActivityFilteredDisplayState(id);
      break;
      }

    case ItemType::Event:
      {
      vtkVgEventInfo info = this->EventModel->GetEventInfo(id);
      on = info.GetDisplayEvent();
      shown = this->EventFilter->GetBestClassifier(info.GetEvent()) >= 0 &&
              info.GetPassesFilters();
      break;
      }

    case ItemType::Track:
      {
      vtkVgTrackInfo info = this->TrackModel->GetTrackInfo(id);
      on = info.GetDisplayTrack();
      shown = this->TrackFilter->GetBestClassifier(info.GetTrack()) >= 0 &&
              info.GetPassesFilters();
      break;
      }

    case ItemType::SceneElement:
      {
      vtkVgTrackInfo info = this->TrackModel->GetTrackInfo(id);
      on = info.GetDisplayTrack();
      shown = info.GetPassesFilters();
      break;
      }
    }

  if (isTopLevel && !sticky &&
      ((!on && !this->ShowUncheckedItems) ||
       (!shown && !this->ShowExcludedItems)))
    {
    // Note: If the item has not yet been added to the tree, this won't
    // actually cause the item to be hidden. Make sure to refresh after
    // adding the item.
    item->setHidden(true);
    return false;
    }

  item->setHidden(false);

  // gray out excluded (filtered) items
  QPalette palette;
  if (!shown)
    palette.setCurrentColorGroup(QPalette::Disabled);

  // update color
  item->setForeground(0, palette.windowText());

  // update checkbox
  item->setCheckState(0, on ? Qt::Checked : Qt::Unchecked);

  // update "sticky" icon
  item->setIcon(0, sticky ? QIcon(":/icons/16x16/pin") : QIcon());

  this->UpdateItemStatus(item);

  return true;
}

//-----------------------------------------------------------------------------
void vpTreeView::Clear()
{
  this->clear();
}

//-----------------------------------------------------------------------------
void vpTreeView::RefreshRecursive(QTreeWidgetItem* root, int depth)
{
  int numChildren = root->childCount();
  for (int i = 0; i < numChildren; ++i)
    {
    QTreeWidgetItem* item = root->child(i);

    // Hide top level items if the option has been set, but continue to show
    // children, even if they are filtered, in order to avoid confusion.
    this->RefreshItem(item, depth == 0);
    this->RefreshRecursive(item, depth + 1);
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::Refresh()
{
  // update the items we already have in the tree - do not modify tree structure
  SignalBlocker sb(this);
  this->RefreshRecursive(this->invisibleRootItem(), 0);
}

//-----------------------------------------------------------------------------
void vpTreeView::AddEventTracks(vtkVgEvent* event, QTreeWidgetItem* eventItem)
{
  int numTracks = event->GetNumberOfTracks();
  for (int k = 0; k < numTracks; ++k)
    {
    QTreeWidgetItem* item = this->CreateItem(event->GetTrack(k));

    // store event track index in addition to track id
    item->setData(0, IDR_ItemIndex, k);

    this->RefreshItem(item);
    eventItem->addChild(item);
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::AddAllActivities()
{
  vtkVgActivityManager* am = this->ActivityManager;
  int numActivities = am->GetNumberOfActivities();
  for (int i = 0; i < numActivities; ++i)
    {
    vtkVgActivity* a = am->GetActivity(i);

    QTreeWidgetItem* activityItem =
      this->CreateItem(ItemType::Activity, a->GetName(), a->GetId());

    activityItem->setData(0,
                          IDR_ItemSortValsStart + ST_Saliency,
                          a->GetSaliency());

    activityItem->setData(0,
                          IDR_ItemSortValsStart + ST_Probability,
                          a->GetProbability());

    // store index rather than id since future lookups will be index-based
    activityItem->setData(0, IDR_ItemId, i);

    // add all events in this activity
    int numEvents = a->GetNumberOfEvents();
    for (int j = 0; j < numEvents; ++j)
      {
      vtkVgEvent* e = a->GetEvent(j);

      int id = e->GetId();
      const char* type = this->EventTypeRegistry->GetTypeById(
                           e->GetActiveClassifierType()).GetName();

      QTreeWidgetItem* eventItem = this->CreateItem(ItemType::Event, type, id);

      eventItem->setData(0, IDR_ItemIndex, j);
      this->UpdateItemStatus(eventItem);

      // add all tracks in this event
      this->AddEventTracks(e, eventItem);

      this->RefreshItem(eventItem);
      activityItem->addChild(eventItem);
      }

    bool visible = this->RefreshItem(activityItem, true);
    this->addTopLevelItem(activityItem);

    if (!visible)
      {
      activityItem->setHidden(true);
      }
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::AddAllEvents()
{
  vtkVgEventModel* em = this->EventModel;

  em->InitEventTraversal();
  while (vtkVgEvent* event = em->GetNextEvent().GetEvent())
    {
    const char* type = this->EventTypeRegistry->GetTypeById(
                         event->GetActiveClassifierType()).GetName();

    int id = event->GetId();

    QTreeWidgetItem* eventItem =
      this->CreateItem(ItemType::Event, type, id);

    eventItem->setData(0,
                       IDR_ItemSortValsStart + ST_Normalcy,
                       event->GetActiveClassifierNormalcy());

    this->UpdateItemStatus(eventItem);

    // add all tracks in this event
    this->AddEventTracks(event, eventItem);

    bool visible = this->RefreshItem(eventItem, true);
    this->addTopLevelItem(eventItem);

    if (!visible)
      {
      eventItem->setHidden(true);
      }
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::AddAllTracks()
{
  vtkVgTrackModel* tm = this->TrackModel;

  tm->InitTrackTraversal();
  while (vtkVgTrack* track = tm->GetNextTrack().GetTrack())
    {
    if (track->GetDisplayFlags() & vtkVgTrack::DF_SceneElement)
      {
      continue;
      }

    QTreeWidgetItem* trackItem = this->CreateItem(track);

    trackItem->setData(0,
                       IDR_ItemSortValsStart + ST_Normalcy,
                       track->GetNormalcy());

    bool visible = this->RefreshItem(trackItem, true);
    this->addTopLevelItem(trackItem);

    if (!visible)
      {
      trackItem->setHidden(true);
      }
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::AddAllSceneElements()
{
  vtkVgTrackModel* tm = this->TrackModel;

  tm->InitTrackTraversal();
  while (vtkVgTrack* track = tm->GetNextTrack().GetTrack())
    {
    if (!(track->GetDisplayFlags() & vtkVgTrack::DF_SceneElement))
      {
      continue;
      }

    QTreeWidgetItem* trackItem = this->CreateItem(track, true);

    trackItem->setData(0,
                       IDR_ItemSortValsStart + ST_Probability,
                       track->GetNormalcy());

    bool visible = this->RefreshItem(trackItem, true);
    this->addTopLevelItem(trackItem);

    if (!visible)
      {
      trackItem->setHidden(true);
      }
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::AddAndSelectTrack(vtkVgTrack* track)
{
  int id = track->GetId();
  QTreeWidgetItem* trackItem;

  if ((trackItem = this->FindTopLevelItem(ItemType::Track, id)) == 0)
    {
    trackItem = this->CreateItem(track);

    this->UpdateItemStatus(trackItem);

    bool visible = this->RefreshItem(trackItem, true);
    this->addTopLevelItem(trackItem);

    if (!visible)
      {
      trackItem->setHidden(true);
      }
    }

  this->setCurrentItem(trackItem);
}

//-----------------------------------------------------------------------------
void vpTreeView::AddAndSelectEvent(vtkVgEvent* event)
{
  int id = event->GetId();
  QTreeWidgetItem* eventItem;

  if ((eventItem = this->FindTopLevelItem(ItemType::Event, id)) == 0)
    {
    const char* type = this->EventTypeRegistry->GetTypeById(
                         event->GetActiveClassifierType()).GetName();

    eventItem = this->CreateItem(ItemType::Event, type, id);

    eventItem->setData(0, IDR_ItemSortValsStart + ST_Saliency,
                       1.0 - event->GetActiveClassifierNormalcy());

    this->UpdateItemStatus(eventItem);

    // add all tracks in this event
    this->AddEventTracks(event, eventItem);

    bool visible = this->RefreshItem(eventItem, true);
    this->addTopLevelItem(eventItem);

    if (!visible)
      {
      eventItem->setHidden(true);
      }
    }

  this->setCurrentItem(eventItem);
}

//-----------------------------------------------------------------------------
void vpTreeView::AddAndSelectSceneElement(vtkVgTrack* track)
{
  int id = track->GetId();
  QTreeWidgetItem* trackItem;

  if ((trackItem = this->FindTopLevelItem(ItemType::SceneElement, id)) == 0)
    {
    trackItem = this->CreateItem(track, true);

    this->UpdateItemStatus(trackItem);

    bool visible = this->RefreshItem(trackItem, true);
    this->addTopLevelItem(trackItem);

    if (!visible)
      {
      trackItem->setHidden(true);
      }
    }

  this->setCurrentItem(trackItem);
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* vpTreeView::FindItem(QTreeWidgetItem* root, int type, int id)
{
  // perform a pre-order traversal
  int numChildren = root->childCount();
  for (int i = 0; i < numChildren; ++i)
    {
    // look at this child
    QTreeWidgetItem* item = root->child(i);
    if (item->data(0, IDR_ItemType).toInt() == type &&
        item->data(0, IDR_ItemId).toInt() == id)
      {
      return item;
      }
    // look in this child's subtree
    if (QTreeWidgetItem* child = this->FindItem(item, type, id))
      {
      return child;
      }
    }
  return 0; // not found
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* vpTreeView::FindTopLevelItem(int type, int id)
{
  for (int i = 0, end = this->topLevelItemCount(); i < end; ++i)
    {
    QTreeWidgetItem* item = this->topLevelItem(i);
    if (item->data(0, IDR_ItemType).toInt() == type &&
        item->data(0, IDR_ItemId).toInt() == id)
      {
      return item;
      }
    }
  return 0;
}

//-----------------------------------------------------------------------------
bool vpTreeView::SelectChildItem(int parentType, int parentId, int index)
{
  // look up the parent item, then select from its children
  if (QTreeWidgetItem* foundItem =
      this->FindItem(this->invisibleRootItem(), parentType, parentId))
    {
    this->setCurrentItem(foundItem->child(index));
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vpTreeView::SelectItem(int type, int id)
{
  if (QTreeWidgetItem* foundItem =
      this->FindItem(this->invisibleRootItem(), type, id))
    {
    this->setCurrentItem(foundItem);
    this->scrollTo(this->indexFromItem(foundItem));
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
void vpTreeView::GetItemInfo(QTreeWidgetItem* item,
                             int& type, int& id,
                             int& parentId, int& index)
{
  type = item->data(0, IDR_ItemType).toInt();
  id = item->data(0, IDR_ItemId).toInt();

  QTreeWidgetItem* parent = item->parent();
  if (parent && parent->data(0, IDR_ItemType).isValid())
    {
    parentId = parent->data(0, IDR_ItemId).toInt();
    index = item->data(0, IDR_ItemIndex).toInt();
    }
  else
    {
    parentId = -1;
    index = -1;
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::UpdateItemStatus(QTreeWidgetItem* item)
{
  int status = vgObjectStatus::None;

  int id = item->data(0, IDR_ItemId).toInt();
  int type = item->data(0, IDR_ItemType).toInt();

  switch (type)
    {
    case ItemType::Activity:
      {
      status = this->ActivityManager->GetActivity(id)->GetStatus();
      break;
      }

    case ItemType::Event:
      {
      vtkVgEvent* event = this->EventModel->GetEvent(id);
      status = event->GetStatus();

      // show modifiable events in bold type
      QFont f = item->font(0);
      f.setBold(event->IsModifiable());
      item->setFont(0, f);
      break;
      }

    case ItemType::Track:
    case ItemType::SceneElement:
      {
      vtkVgTrack* track = this->TrackModel->GetTrack(id);
      status = track->GetStatus();
      QFont f = item->font(0);
      f.setBold(track->IsModifiable());
      f.setItalic(track->IsUserCreated());
      item->setFont(0, f);
      break;
      }

    default:
      // nothing to be done for this item type
      return;
    }

  // update background color based on item status
  switch (status)
    {
    case vgObjectStatus::None:
      // normal background color
      item->setBackground(0, QBrush());
      break;

    case vgObjectStatus::Positive:
      {
      QColor lightGreen(159, 202, 166);
      item->setBackgroundColor(0, lightGreen);
      break;
      }

    case vgObjectStatus::Negative:
      {
      QColor lightRed(255, 106, 106);
      item->setBackgroundColor(0, lightRed);
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::UpdateActivityItem(QTreeWidgetItem* item)
{
  int id = item->data(0, IDR_ItemId).toInt();
  vtkVgActivity* activity = this->ActivityManager->GetActivity(id);

  // NOTE: We assume here that the child event tree items appear in the tree
  // in order of their index in the parent activity. This may not be a safe
  // assumption in the future if we allow different sort orders.
  int eventIndex = 0;
  int childIndex = 0;
  int numEvents = activity->GetNumberOfEvents();
  for (; eventIndex < numEvents && childIndex < item->childCount();)
    {
    vtkVgEvent* event = activity->GetEvent(eventIndex);
    QTreeWidgetItem* child = item->child(childIndex);

    if (event->GetId() ==  child->data(0, IDR_ItemId).toInt())
      {
      // update the index of the tree item
      child->setData(0, IDR_ItemIndex, eventIndex);
      ++childIndex;
      ++eventIndex;
      }
    else
      {
      // This event has a different id, so the event referred to in the tree
      // must have been deleted.  Remove the tree item.
      QTreeWidgetItem* c =  item->takeChild(childIndex);
      delete c;
      }
    }

  // remove any remaining dangling children
  while (childIndex < item->childCount())
    {
    QTreeWidgetItem* c =  item->takeChild(childIndex);
    delete c;
    }

  // Add new events to the tree. We assume new events are always added to the
  // end of the activity event list.
  while (eventIndex < numEvents)
    {
    vtkVgEvent* e = activity->GetEvent(eventIndex);
    int id = e->GetId();
    const char* type = this->EventTypeRegistry->GetTypeById(
                         e->GetActiveClassifierType()).GetName();

    QTreeWidgetItem* eventItem = this->CreateItem(ItemType::Event, type, id);

    eventItem->setData(0, IDR_ItemIndex, eventIndex);
    this->UpdateItemStatus(eventItem);

    // add all tracks in this event
    this->AddEventTracks(e, eventItem);

    item->addChild(eventItem);
    this->RefreshItem(eventItem);
    ++eventIndex;
    }
}

//-----------------------------------------------------------------------------
QSize vpTreeView::sizeHint() const
{
  QSize size = QTreeWidget::sizeHint();
  size.setWidth(150);
  return size;
}

//-----------------------------------------------------------------------------
void vpTreeView::contextMenuEvent(QContextMenuEvent* event)
{
  QMenu menu(this);

  if (this->selectedItems().size() == 1)
    {
    menu.addAction("Focus Alone", this, SIGNAL(FocusItemAlone()));
    menu.addAction("Focus", this, SIGNAL(FocusItem()));
    menu.addSeparator();
    menu.addAction("Go To Start", this, SIGNAL(GoToStartFrame()));
    menu.addAction("Go To End", this, SIGNAL(GoToEndFrame()));

    QTreeWidgetItem* item = this->selectedItems()[0];

    if (item->data(0, IDR_ItemType).toInt() == ItemType::Track)
      {
      menu.addAction("Follow", this, SIGNAL(FollowTrack()));
      menu.addSeparator();
      menu.addAction("Show Related Events", this,
                     SIGNAL(ShowLinkedEvents()));
      menu.addAction("Hide Related Events", this,
                     SIGNAL(HideLinkedEvents()));
      menu.addAction("Show Related Activities", this,
                     SIGNAL(ShowLinkedActivities()));
      menu.addAction("Hide Related Activities", this,
                     SIGNAL(HideLinkedActivities()));
      menu.addSeparator();
      menu.addAction("Add Related Active Events To Graph Model", this,
                     SIGNAL(AddTrackEventsToGraphModel()));
      }

    menu.addSeparator();
    }

  bool enable = this->selectedItems().size() > 0;
  menu.addAction("Show", this, SLOT(ShowItems()))->setEnabled(enable);
  menu.addAction("Hide", this, SLOT(HideItems()))->setEnabled(enable);
  menu.addAction("Hide All Except", this, SLOT(HideItemsExcept()))->setEnabled(enable);

  foreach (QTreeWidgetItem* item, this->selectedItems())
    {
    if (item->data(0, IDR_ItemType).toInt() == ItemType::Event)
      {
      menu.addSeparator();
      menu.addAction("Add Event(s) to Graph Model", this,
                     SIGNAL(AddEventsToGraphModel()));
      break;
      }
    }

  if (enable)
    {
    bool allChildren = true;
    bool allSticky = true;
    foreach (QTreeWidgetItem* item, this->selectedItems())
      {
      if (!item->parent())
        {
        allChildren = false;
        if (allSticky && !item->data(0, IDR_ItemSticky).toBool())
          {
          allSticky = false;
          }
        }
      if (!(allChildren || allSticky))
        {
        break;
        }
      }

    if (!allChildren)
      {
      menu.addSeparator();
      QAction* a = menu.addAction("Sticky", this, SLOT(ToggleSticky()));
      a->setCheckable(true);
      a->setChecked(allSticky);
      }
    }

  // allow someone else to add items to the menu
  emit this->ContextMenuOpened(menu);

  menu.exec(event->globalPos());
}

//-----------------------------------------------------------------------------
void vpTreeView::leaveEvent(QEvent* event)
{
  QTreeWidget::leaveEvent(event);
  emit this->MouseLeft();
}

//-----------------------------------------------------------------------------
void vpTreeView::SetStateRecursive(QTreeWidgetItem* root, Qt::CheckState state,
                                   QList<QTreeWidgetItem*>& changed)
{
  for (int i = 0; i < root->childCount(); ++i)
    {
    QTreeWidgetItem* child = root->child(i);
    if (child->checkState(0) != state)
      {
      child->setCheckState(0, state);
      changed.append(child);
      }
    this->SetStateRecursive(child, state, changed);
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::SetStateRecursive(QTreeWidgetItem* root, Qt::CheckState state,
                                   QList<QTreeWidgetItem*>& changed, int type)
{
  for (int i = 0; i < root->childCount(); ++i)
    {
    QTreeWidgetItem* child = root->child(i);
    int childType = child->data(0, IDR_ItemType).toInt();
    if (childType == type)
      {
      if (child->checkState(0) != state)
        {
        child->setCheckState(0, state);
        changed.append(child);
        }
      this->SetStateRecursive(child, state, changed);
      continue;
      }
    this->SetStateRecursive(child, state, changed, type);
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::SetEventStateRecursive(QTreeWidgetItem* root,
                                        Qt::CheckState state,
                                        QList<QTreeWidgetItem*>& changed,
                                        int eventType)
{
  for (int i = 0; i < root->childCount(); ++i)
    {
    QTreeWidgetItem* child = root->child(i);
    if (child->data(0, IDR_ItemType).toInt() == ItemType::Event)
      {
      int id = child->data(0, IDR_ItemId).toInt();
      if (this->EventModel->GetEvent(id)->GetActiveClassifierType() == eventType)
        {
        if (child->checkState(0) != state)
          {
          child->setCheckState(0, state);
          changed.append(child);
          }
        this->SetStateRecursive(child, state, changed);
        continue;
        }
      }
    this->SetEventStateRecursive(child, state, changed, eventType);
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::SetActivityStateRecursive(QTreeWidgetItem* root,
                                           Qt::CheckState state,
                                           QList<QTreeWidgetItem*>& changed,
                                           int activityType)
{
  for (int i = 0; i < root->childCount(); ++i)
    {
    QTreeWidgetItem* child = root->child(i);
    if (child->data(0, IDR_ItemType).toInt() == ItemType::Activity)
      {
      int id = child->data(0, IDR_ItemId).toInt();
      if (this->ActivityManager->GetActivity(id)->GetType() == activityType)
        {
        if (child->checkState(0) != state)
          {
          child->setCheckState(0, state);
          changed.append(child);
          }
        this->SetStateRecursive(child, state, changed);
        continue;
        }
      }
    this->SetActivityStateRecursive(child, state, changed, activityType);
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::SetAll(Qt::CheckState state)
{
  QList<QTreeWidgetItem*> changed;
    {
    SignalBlocker sb(this);
    this->SetStateRecursive(this->invisibleRootItem(), state, changed);
    }
  emit this->ItemsChanged(changed);
}

//-----------------------------------------------------------------------------
void vpTreeView::SetAll(Qt::CheckState state, int type)
{
  QList<QTreeWidgetItem*> changed;
    {
    SignalBlocker sb(this);
    this->SetStateRecursive(this->invisibleRootItem(), state, changed, type);
    }
  emit this->ItemsChanged(changed);
}

//-----------------------------------------------------------------------------
void vpTreeView::SetAllEvents(Qt::CheckState state, int eventType)
{
  QList<QTreeWidgetItem*> changed;
    {
    SignalBlocker sb(this);
    this->SetEventStateRecursive(this->invisibleRootItem(), state, changed, eventType);
    }
  emit this->ItemsChanged(changed);
}

//-----------------------------------------------------------------------------
void vpTreeView::SetAllActivities(Qt::CheckState state, int activityType)
{
  QList<QTreeWidgetItem*> changed;
    {
    SignalBlocker sb(this);
    this->SetActivityStateRecursive(this->invisibleRootItem(), state, changed, activityType);
    }
  emit this->ItemsChanged(changed);
}

//-----------------------------------------------------------------------------
void vpTreeView::ShowAll()
{
  this->SetAll(Qt::Checked);
}

//-----------------------------------------------------------------------------
void vpTreeView::HideAll()
{
  this->SetAll(Qt::Unchecked);
}

//-----------------------------------------------------------------------------
void vpTreeView::ShowAllTracks()
{
  this->SetAll(Qt::Checked, ItemType::Track);
}

//-----------------------------------------------------------------------------
void vpTreeView::HideAllTracks()
{
  this->SetAll(Qt::Unchecked, ItemType::Track);
}

//-----------------------------------------------------------------------------
void vpTreeView::ShowEventType(int type)
{
  this->SetAllEvents(Qt::Checked, type);
}

//-----------------------------------------------------------------------------
void vpTreeView::ShowAllEvents()
{
  this->SetAll(Qt::Checked, ItemType::Event);
}

//-----------------------------------------------------------------------------
void vpTreeView::HideAllEvents()
{
  this->SetAll(Qt::Unchecked, ItemType::Event);
}

//-----------------------------------------------------------------------------
void vpTreeView::ShowActivityType(int type)
{
  this->SetAllActivities(Qt::Checked, type);
}

//-----------------------------------------------------------------------------
void vpTreeView::ShowAllActivities()
{
  this->SetAll(Qt::Checked, ItemType::Activity);
}

//-----------------------------------------------------------------------------
void vpTreeView::HideAllActivities()
{
  this->SetAll(Qt::Unchecked, ItemType::Activity);
}

//-----------------------------------------------------------------------------
void vpTreeView::ShowAllSceneElements()
{
  this->SetAll(Qt::Checked, ItemType::SceneElement);
}

//-----------------------------------------------------------------------------
void vpTreeView::HideAllSceneElements()
{
  this->SetAll(Qt::Unchecked, ItemType::SceneElement);
}

//-----------------------------------------------------------------------------
void vpTreeView::ShowItems()
{
  // show all selected objects
  QList<QTreeWidgetItem*> selected = this->selectedItems();
  QList<QTreeWidgetItem*> changed;
    {
    SignalBlocker sb(this);
    foreach (QTreeWidgetItem* item, selected)
      {
      if (item->checkState(0) != Qt::Checked)
        {
        item->setCheckState(0, Qt::Checked);
        changed.append(item);
        }
      }
    }
  emit this->ItemsChanged(changed);
}

//-----------------------------------------------------------------------------
void vpTreeView::HideItems()
{
  // hide all selected objects
  QList<QTreeWidgetItem*> selected = this->selectedItems();
  QList<QTreeWidgetItem*> changed;
    {
    SignalBlocker sb(this);
    foreach (QTreeWidgetItem* item, selected)
      {
      if (item->checkState(0) != Qt::Unchecked)
        {
        item->setCheckState(0, Qt::Unchecked);
        changed.append(item);
        }
      }
    }
  emit this->ItemsChanged(changed);
}

//-----------------------------------------------------------------------------
void vpTreeView::HideUnselectedShowSelected(QTreeWidgetItem* root,
                                            QList<QTreeWidgetItem*>& changed)
{
  // recursively hide all the unselected objects and show the selected ones
  for (int i = 0; i < root->childCount(); ++i)
    {
    QTreeWidgetItem* child = root->child(i);
    if (child->isSelected() != (child->checkState(0) == Qt::Checked))
      {
      child->setCheckState(0, child->isSelected() ? Qt::Checked : Qt::Unchecked);
      changed.append(child);
      }
    this->HideUnselectedShowSelected(child, changed);
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::HideItemsExcept()
{
  QList<QTreeWidgetItem*> changed;
    {
    SignalBlocker sb(this);
    this->HideUnselectedShowSelected(this->invisibleRootItem(), changed);
    }
  emit this->ItemsChanged(changed);
}

//-----------------------------------------------------------------------------
void vpTreeView::ToggleSticky()
{
  SignalBlocker sb(this);
  bool sticky = static_cast<QAction*>(this->sender())->isChecked();

  foreach (QTreeWidgetItem* item, this->selectedItems())
    {
    if (!item->parent())
      {
      item->setData(0, IDR_ItemSticky, sticky);
      this->RefreshItem(item, true);
      }
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::OnItemChanged(QTreeWidgetItem* item)
{
  QList<QTreeWidgetItem*> items;
  items.append(item);
  // set child state as well unless holding Ctrl
  if ((QApplication::keyboardModifiers() & Qt::ControlModifier) == 0)
    {
    SignalBlocker sb(this);
    this->SetStateRecursive(item, item->checkState(0), items);
    }
  emit this->ItemsChanged(items, items.size() > 1);
}

//-----------------------------------------------------------------------------
void vpTreeView::SortBy(int sortType, Qt::SortOrder direction)
{
  this->SortType = sortType;
  this->invisibleRootItem()->sortChildren(0, direction);
}

//-----------------------------------------------------------------------------
void vpTreeView::SetShowExcludedItems(bool show)
{
  if (show != this->ShowExcludedItems)
    {
    this->ShowExcludedItems = show;
    this->Refresh();
    }
}

//-----------------------------------------------------------------------------
void vpTreeView::SetShowUncheckedItems(bool show)
{
  if (show != this->ShowUncheckedItems)
    {
    this->ShowUncheckedItems = show;
    this->Refresh();
    }
}

//-----------------------------------------------------------------------------
const char* vpTreeView::GetSortTypeString(int sortType)
{
  switch (sortType)
    {
    case ST_Id:          return "Id";
    case ST_Name:        return "Name";
    case ST_Normalcy:    return "Normalcy";
    case ST_Saliency:    return "Saliency";
    case ST_Probability: return "Probability";
    case ST_Length:      return "Length";
    }
  return 0;
}
