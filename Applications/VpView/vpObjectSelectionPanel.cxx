/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "ui_vpObjectSelectionPanel.h"

#include "vpObjectSelectionPanel.h"
#include "vpTrackTypeDialog.h"

#include "vgEventType.h"
#include "vpViewCore.h"
#include "vtkVpTrackModel.h"

#include "vtkVgActivity.h"
#include "vtkVgActivityManager.h"
#include "vtkVgEvent.h"
#include "vtkVgEventModel.h"
#include "vtkVgEventTypeRegistry.h"
#include "vtkVgRendererUtils.h"
#include "vtkVgTrack.h"
#include "vtkVgTrackTypeRegistry.h"

#include <vtkIdList.h>

#include <QContextMenuEvent>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QSignalMapper>

// C++ includes.
#include <vector>

//-----------------------------------------------------------------------------
static void AddSortComboItem(const QString& fmt, QComboBox* cb, int type)
{
  cb->addItem(fmt.arg(vpTreeView::GetSortTypeString(type)), type);
}

//-----------------------------------------------------------------------------
static void UpdateTreeSort(vpTreeView* tree,
                           QComboBox* sortType,
                           QCheckBox* sortDescending)
{
  tree->SortBy(
    sortType->itemData(sortType->currentIndex()).toInt(),
    sortDescending->checkState() == Qt::Checked ? Qt::DescendingOrder
                                                : Qt::AscendingOrder);
}

//-----------------------------------------------------------------------------
vpObjectSelectionPanel::vpObjectSelectionPanel(QWidget* p)
  : QWidget(p)
{
  this->Ui = new Ui::vpObjectSelectionPanel;
  this->Ui->setupUi(this);

  QString fmt("Sort By %1");

  // Populate the various sort type combo boxes.
  AddSortComboItem(fmt, this->Ui->activitySortType, vpTreeView::ST_Id);
  AddSortComboItem(fmt, this->Ui->activitySortType, vpTreeView::ST_Name);
  AddSortComboItem(fmt, this->Ui->activitySortType, vpTreeView::ST_Saliency);
  AddSortComboItem(fmt, this->Ui->activitySortType, vpTreeView::ST_Probability);

  AddSortComboItem(fmt, this->Ui->eventSortType, vpTreeView::ST_Id);
  AddSortComboItem(fmt, this->Ui->eventSortType, vpTreeView::ST_Name);
  AddSortComboItem(fmt, this->Ui->eventSortType, vpTreeView::ST_Normalcy);

  AddSortComboItem(fmt, this->Ui->trackSortType, vpTreeView::ST_Id);
  AddSortComboItem(fmt, this->Ui->trackSortType, vpTreeView::ST_Name);
  AddSortComboItem(fmt, this->Ui->trackSortType, vpTreeView::ST_Normalcy);
  AddSortComboItem(fmt, this->Ui->trackSortType, vpTreeView::ST_Length);

  AddSortComboItem(fmt, this->Ui->fseSortType, vpTreeView::ST_Id);
  AddSortComboItem(fmt, this->Ui->fseSortType, vpTreeView::ST_Name);
  AddSortComboItem(fmt, this->Ui->fseSortType, vpTreeView::ST_Probability);

  connect(this->Ui->activitySortType,
          SIGNAL(currentIndexChanged(int)),
          SLOT(OnSortTypeChanged(int)));

  connect(this->Ui->eventSortType,
          SIGNAL(currentIndexChanged(int)),
          SLOT(OnSortTypeChanged(int)));

  connect(this->Ui->trackSortType,
          SIGNAL(currentIndexChanged(int)),
          SLOT(OnSortTypeChanged(int)));

  connect(this->Ui->fseSortType,
          SIGNAL(currentIndexChanged(int)),
          SLOT(OnSortTypeChanged(int)));

  connect(this->Ui->activitySortDescending,
          SIGNAL(stateChanged(int)),
          SLOT(OnSortDirectionChanged(int)));

  connect(this->Ui->eventSortDescending,
          SIGNAL(stateChanged(int)),
          SLOT(OnSortDirectionChanged(int)));

  connect(this->Ui->trackSortDescending,
          SIGNAL(stateChanged(int)),
          SLOT(OnSortDirectionChanged(int)));

  connect(this->Ui->fseSortDescending,
          SIGNAL(stateChanged(int)),
          SLOT(OnSortDirectionChanged(int)));

  this->CreateEventMapper = new QSignalMapper;
  this->SetEventStatusMapper = new QSignalMapper;
  this->SetActivityStatusMapper = new QSignalMapper;
  this->SetTrackStatusMapper = new QSignalMapper;

  this->ShowEventTypeMapper = new QSignalMapper;
  this->ShowActivityTypeMapper = new QSignalMapper;

  connect(this->CreateEventMapper,
          SIGNAL(mapped(int)),
          SLOT(CreateEvent(int)));

  connect(this->SetEventStatusMapper,
          SIGNAL(mapped(int)),
          SLOT(SetEventStatus(int)));

  connect(this->SetActivityStatusMapper,
          SIGNAL(mapped(int)),
          SLOT(SetActivityStatus(int)));

  connect(this->SetTrackStatusMapper,
          SIGNAL(mapped(int)),
          SLOT(SetTrackStatus(int)));

  connect(this->ShowEventTypeMapper,
          SIGNAL(mapped(int)),
          SLOT(ShowEventType(int)));

  connect(this->ShowActivityTypeMapper,
          SIGNAL(mapped(int)),
          SLOT(ShowActivityType(int)));

  this->SelectedItem.Type = -1;
  this->HoveredItem.Type = -1;

  this->Trees[0] = this->Ui->activityTree;
  this->Trees[1] = this->Ui->eventTree;
  this->Trees[2] = this->Ui->trackTree;
  this->Trees[3] = this->Ui->fseTree;

  this->Ui->tabWidget->setCurrentIndex(2);
  this->PrevTab = 0;

  this->SetupTab();

  connect(this->Ui->tabWidget,
          SIGNAL(currentChanged(int)),
          SLOT(OnObjectTypeChanged(int)));

  connect(this->Ui->showExcludedItems,
          SIGNAL(stateChanged(int)),
          SLOT(OnShownItemsChanged()));

  connect(this->Ui->showUncheckedItems,
          SIGNAL(stateChanged(int)),
          SLOT(OnShownItemsChanged()));

  connect(this->Ui->showAllButton,
          SIGNAL(clicked()),
          SLOT(OnShowAll()));

  connect(this->Ui->hideAllButton,
          SIGNAL(clicked()),
          SLOT(OnHideAll()));

  this->SessionId = 0;
  this->ViewCoreInstance = 0;
  this->ActivityManager = 0;
  this->EventModel = 0;
  this->TrackModel = 0;
  this->RefreshFlags = 0;
  this->RebuildFlags = 0;
}

//-----------------------------------------------------------------------------
vpObjectSelectionPanel::~vpObjectSelectionPanel()
{
  delete this->ShowActivityTypeMapper;
  delete this->ShowEventTypeMapper;
  delete this->SetActivityStatusMapper;
  delete this->SetEventStatusMapper;
  delete this->SetTrackStatusMapper;
  delete this->CreateEventMapper;
  delete this->Ui;
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::SetCurrentTab(int type)
{
  this->Ui->tabWidget->setCurrentIndex(type);
}

//-----------------------------------------------------------------------------
inline int vpObjectSelectionPanel::CurrentTab()
{
  return this->Ui->tabWidget->currentIndex();
}

//-----------------------------------------------------------------------------
inline vpTreeView* vpObjectSelectionPanel::CurrentTree()
{
  return this->Trees[this->CurrentTab()];
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::Initialize(vpViewCore* viewCore,
                                        vtkVgActivityManager* activityManager,
                                        vtkVgEventModel* eventModel,
                                        vtkVpTrackModel* trackModel,
                                        vtkVgEventFilter* eventFilter,
                                        vtkVgTrackFilter* trackFilter,
                                        vtkVgEventTypeRegistry* eventTypes,
                                        vtkVgTrackTypeRegistry* trackTypes,
                                        int sessionId,
                                        const QString& title)
{
  for (int i = 0; i < 4; ++i)
    {
    this->Trees[i]->Initialize(activityManager, eventModel , trackModel,
                               eventFilter, trackFilter,
                               eventTypes, trackTypes);
    }

  this->SessionId = sessionId;
  this->Title = title;
  this->ViewCoreInstance = viewCore;
  this->ActivityManager = activityManager;
  this->EventModel = eventModel;
  this->TrackModel = trackModel;

  this->EventTypeRegistry = eventTypes;
  this->TrackTypeRegistry = trackTypes;

  this->RebuildFlags = ~0;
  this->Ui->showAllButton->setEnabled(true);
  this->Ui->hideAllButton->setEnabled(true);

  connect(this->Ui->tabWidget,
          SIGNAL(tabBarContextMenu(QContextMenuEvent*, int)),
          SLOT(OnTabBarContextMenu(QContextMenuEvent*, int)));

  connect(viewCore,
          SIGNAL(stoppedEditingTrack(vpViewCore::enumAnnotationMode)),
          SLOT(StopEditingTrack()));
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::showEvent(QShowEvent* /*event*/)
{
  int tab = this->CurrentTab();
  if (this->NeedsRebuild(tab))
    {
    this->RebuildTreeView();
    }
  else if (this->NeedsRefresh(tab))
    {
    this->RefreshTreeView();
    }
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::Update(bool rebuild)
{
  // only update the visible tab - don't refresh the others until they are shown
  if (rebuild)
    {
    this->RebuildFlags = ~0;
    if (this->isVisible())
      {
      this->RebuildTreeView();
      }
    }
  else
    {
    this->RefreshFlags = ~0;
    if (this->isVisible())
      {
      this->RefreshTreeView();
      }
    }
}

//-----------------------------------------------------------------------------
bool vpObjectSelectionPanel::SelectItem(int type, int id)
{
  this->Ui->tabWidget->setCurrentIndex(type);
  return this->CurrentTree()->SelectItem(type, id);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::AddAndSelectItem(int type, int id)
{
  this->Ui->tabWidget->setCurrentIndex(type);

  switch (type)
    {
    case vgObjectTypeDefinitions::Track:
      // add new track (and begin editing)
      this->Ui->trackTree->AddAndSelectTrack(this->TrackModel->GetTrack(id));
      break;

    case vgObjectTypeDefinitions::Event:
      // add new event
      this->Ui->eventTree->AddAndSelectEvent(this->EventModel->GetEvent(id));
      break;

    case vgObjectTypeDefinitions::SceneElement:
      // add new scene element
      this->Ui->fseTree->AddAndSelectSceneElement(this->TrackModel->GetTrack(id));
      break;

    default:
      // not yet implemented
      return;
    }
}

//-----------------------------------------------------------------------------
bool vpObjectSelectionPanel::GetSelectedItemInfo(vgItemInfo& info)
{
  if (this->SelectedItem.Type < 0)
    {
    return false;
    }
  info = this->SelectedItem;
  return true;
}

//-----------------------------------------------------------------------------
bool vpObjectSelectionPanel::GetHoveredItemInfo(vgItemInfo& info)
{
  if (this->HoveredItem.Type < 0)
    {
    return false;
    }
  info = this->HoveredItem;
  return true;
}

//-----------------------------------------------------------------------------
QList<vtkIdType> vpObjectSelectionPanel::GetSelectedItems(int type)
{
  vpTreeView* tree = this->CurrentTree();

  vgItemInfo info;
  QList<vtkIdType> ids;
  foreach(QTreeWidgetItem* item, tree->selectedItems())
    {
    tree->GetItemInfo(item, info.Type, info.Id, info.ParentId, info.Index);
    if (info.Type == type)
      {
      ids << info.Id;
      }
    }

  return ids;
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnTreeHoverItemChanged(QTreeWidgetItem* item)
{
  this->CurrentTree()->GetItemInfo(item,
                                   this->HoveredItem.Type,
                                   this->HoveredItem.Id,
                                   this->HoveredItem.ParentId,
                                   this->HoveredItem.Index);

  emit HoverItemChanged(this->SessionId);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnTreeHoverStopped()
{
  this->HoveredItem.Type = -1;

  emit HoverItemChanged(this->SessionId);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnTreeSelectionChanged()
{
  vpTreeView* tree = this->CurrentTree();
  QList<QTreeWidgetItem*> items = tree->selectedItems();

  if (items.isEmpty())
    {
    this->SelectedItem.Type = -1;
    this->SelectedItem.Id = -1;
    this->SelectedItem.ParentId = -1;
    this->SelectedItem.Id = -1;
    }
  else
    {
    QTreeWidgetItem* item = items.first();
    tree->GetItemInfo(item, this->SelectedItem.Type, this->SelectedItem.Id,
                      this->SelectedItem.ParentId, this->SelectedItem.Index);
    }

  emit SelectionChanged(this->SessionId);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnTreeContextMenu(QMenu& menu)
{
  int numSelections = this->CurrentTree()->selectedItems().size();
  switch (this->CurrentTab())
    {
    case vgObjectTypeDefinitions::Activity:
      {
      // add activity context menu items
      if (this->SelectedItem.Type == vgObjectTypeDefinitions::Activity)
        {
        menu.addSeparator();
        menu.addAction("Add Event", this, SLOT(AddEventToActivity()));

        QMenu* submenu = menu.addMenu("Set Status");

        vtkVgActivity* a = this->ActivityManager->GetActivity(this->SelectedItem.Id);

        this->AddSetStatusActions(submenu, this->SetActivityStatusMapper,
                                  a->GetStatus());
        }
      // add activity event context menu items
      else if (this->SelectedItem.Type == vgObjectTypeDefinitions::Event)
        {
        menu.addSeparator();
        menu.addAction("Remove", this, SLOT(RemoveEventFromActivity()));
        }
      }
    break;

    case vgObjectTypeDefinitions::Track:
      {
      // add track context menu items
      menu.addSeparator();

      QMenu* submenu = menu.addMenu("Create Event");

      // are the right number of tracks selected to create an event?
      if (numSelections == 0)
        {
        submenu->setEnabled(false);
        return;
        }

      // create items in the submenu for each event type
      for (int i = 0; i < this->EventTypeRegistry->GetNumberOfTypes(); ++i)
        {
        const vgEventType& type = this->EventTypeRegistry->GetType(i);

        QAction* a = submenu->addAction(type.GetName(),
                                        this->CreateEventMapper, SLOT(map()));
        this->CreateEventMapper->setMapping(a, type.GetId());

        int minTracks = type.GetMinTracks();
        int maxTracks = type.GetMaxTracks();

        a->setEnabled(numSelections >= minTracks &&
                      (maxTracks == -1 || numSelections <= maxTracks));
        }
      } // fall through

    case vgObjectTypeDefinitions::SceneElement:
      {
      QMenu* submenu = menu.addMenu("Set Status");

      if (numSelections == 0)
        {
        submenu->setEnabled(false);
        return;
        }

      menu.addAction("Set Type", this, SLOT(SetTracksType()));

      vtkVgTrack* track = this->TrackModel->GetTrack(this->SelectedItem.Id);

      this->AddSetStatusActions(submenu, this->SetTrackStatusMapper,
                                track->GetStatus());

      menu.addSeparator();

      if (this->SelectedItem.Type == vgObjectTypeDefinitions::Track ||
          this->SelectedItem.Type == vgObjectTypeDefinitions::SceneElement)
        {
        vtkVgTrack* track = this->TrackModel->GetTrack(this->SelectedItem.Id);
        if (track->IsModifiable())
          {
          menu.addAction("Stop Editing Track", this, SLOT(StopEditingTrack()));
          }
        else
          {
          menu.addAction("Edit Track", this, SLOT(EditTrack()));
          }
        QAction* a = menu.addAction("Delete Track", this, SLOT(DeleteTrack()));
        if (numSelections != 1)
          {
          a->setEnabled(false);
          }

#ifdef VISGUI_USE_KWIVER
        a = menu.addAction("Improve Track", this, SLOT(ImproveTrack()));
#endif

        a = menu.addAction("Split Here...", this, SLOT(SplitTrack()));

        vtkVgTimeStamp splitTime = this->ViewCoreInstance->getCoreTimeStamp();

        const bool isKeyframe =
          this->TrackModel->GetIsKeyframe(this->SelectedItem.Id, splitTime);

        a->setEnabled(isKeyframe &&
                      track->GetStartFrame() < splitTime &&
                      track->GetEndFrame() > splitTime);
        }
      }
    break;

    case vgObjectTypeDefinitions::Event:
      {
      // make sure they clicked on an event
      if (this->SelectedItem.Type != vgObjectTypeDefinitions::Event)
        {
        return;
        }

      // add event context menu items
      menu.addSeparator();

      QMenu* submenu = menu.addMenu("Set Status");

      if (numSelections == 0)
        {
        submenu->setEnabled(false);
        return;
        }

      vtkVgEvent* event = this->EventModel->GetEvent(this->SelectedItem.Id);

      this->AddSetStatusActions(submenu, this->SetEventStatusMapper,
                                event->GetStatus());

      QAction* a = menu.addAction("Delete Event", this, SLOT(DeleteEvent()));
      if (numSelections != 1)
        {
        a->setEnabled(false);
        }
      }
    break;
    }
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::AddSetStatusActions(QMenu* menu,
                                                 QSignalMapper* mapper,
                                                 int curStatus)
{
  int numSelections = this->CurrentTree()->selectedItems().size();

  QAction* a;
  a = menu->addAction("None", mapper, SLOT(map()));
  a->setCheckable(true);
  a->setChecked(numSelections == 1 && curStatus == vgObjectStatus::None);
  mapper->setMapping(a, vgObjectStatus::None);

  a = menu->addAction("Positive", mapper, SLOT(map()));
  a->setCheckable(true);
  a->setChecked(numSelections == 1 && curStatus == vgObjectStatus::Adjudicated);
  mapper->setMapping(a, vgObjectStatus::Adjudicated);

  a = menu->addAction("Negative", mapper, SLOT(map()));
  a->setCheckable(true);
  a->setChecked(numSelections == 1 && curStatus == vgObjectStatus::Excluded);
  mapper->setMapping(a, vgObjectStatus::Excluded);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::CreateEvent(int type)
{
  QList<QTreeWidgetItem*> trackItems = this->CurrentTree()->selectedItems();

  vtkIdList* idl = vtkIdList::New();
  idl->SetNumberOfIds(trackItems.size());

  vgItemInfo info;
  vtkIdType i = 0;
  foreach (QTreeWidgetItem* item, trackItems)
    {
    this->CurrentTree()->GetItemInfo(item, info.Type, info.Id,
                                     info.ParentId, info.Index);
    idl->SetId(i++, info.Id);
    }

  emit this->CreateEvent(type, idl, this->SessionId);
  idl->Delete();
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::DeleteEvent()
{
  emit this->DeleteEvent(this->SelectedItem.Id, this->SessionId);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::EditTrack()
{
  emit this->EditTrack(this->SelectedItem.Id, this->SessionId);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::StopEditingTrack()
{
  emit this->StopEditingTrack(this->SessionId);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::DeleteTrack()
{
  emit this->DeleteTrack(this->SelectedItem.Id, this->SessionId);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::SplitTrack()
{
  emit this->SplitTrack(this->SelectedItem.Id, this->SessionId);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::ImproveTrack()
{
  emit this->ImproveTrack(this->SelectedItem.Id, this->SessionId);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::SetEventStatus(int status)
{
  vgItemInfo info;
  QList<QTreeWidgetItem*> items = this->CurrentTree()->selectedItems();

  // set the status on all selected events
  foreach (QTreeWidgetItem* item, items)
    {
    this->CurrentTree()->GetItemInfo(item, info.Type, info.Id,
                                     info.ParentId, info.Index);

    if (info.Type == vgObjectTypeDefinitions::Event)
      {
      this->EventModel->GetEvent(info.Id)->SetStatus(status);

      // update the tree display
      this->CurrentTree()->UpdateItemStatus(item);
      }
    }
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::SetActivityStatus(int status)
{
  vgItemInfo info;
  QList<QTreeWidgetItem*> items = this->CurrentTree()->selectedItems();

  // set the status on all selected events
  foreach (QTreeWidgetItem* item, items)
    {
    this->CurrentTree()->GetItemInfo(item, info.Type, info.Id,
                                     info.ParentId, info.Index);

    if (info.Type == vgObjectTypeDefinitions::Activity)
      {
      this->ActivityManager->GetActivity(info.Id)->SetStatus(status);

      // update the tree display
      this->CurrentTree()->UpdateItemStatus(item);
      }
    }
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::SetTrackStatus(int status)
{
  vgItemInfo info;
  QList<QTreeWidgetItem*> items = this->CurrentTree()->selectedItems();

  // set the status on all selected events
  foreach (QTreeWidgetItem* item, items)
    {
    this->CurrentTree()->GetItemInfo(item, info.Type, info.Id,
                                     info.ParentId, info.Index);

    if (info.Type == vgObjectTypeDefinitions::Track ||
        info.Type == vgObjectTypeDefinitions::SceneElement)
      {
      this->TrackModel->GetTrack(info.Id)->SetStatus(status);

      // update the tree display
      this->CurrentTree()->UpdateItemStatus(item);
      }
    }
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::AddEventToActivity()
{
  bool ok;
  int id = QInputDialog::getInt(this, tr("Add Event"),
                                tr("Event Id:"),
                                0, 0, 2147483647, 1, &ok);
  if (!ok)
    {
    return;
    }

  vtkVgEvent* event = this->EventModel->GetEvent(id);
  if (!event)
    {
    QMessageBox::warning(0, QString(), "Invalid event ID.");
    return;
    }

  vtkVgActivity* a = this->ActivityManager->GetActivity(this->SelectedItem.Id);
  a->AddEvent(event);

  this->CurrentTree()->UpdateActivityItem(this->CurrentTree()->currentItem());

  this->ActivityManager->Modified();
  emit this->ItemsChanged();
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::RemoveEventFromActivity()
{
  vtkVgActivity* a = this->ActivityManager->GetActivity(this->SelectedItem.ParentId);
  a->RemoveEvent(this->SelectedItem.Index);

  this->CurrentTree()->UpdateActivityItem(this->CurrentTree()->currentItem()->parent());

  this->ActivityManager->Modified();
  emit this->ItemsChanged();
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::SetTracksType()
{
  this->TrackTypeDialog = new vpTrackTypeDialog(this->CurrentTree(), 
                                                this->TrackModel,
                                                this->TrackTypeRegistry,
                                                this);
  connect(this->TrackTypeDialog, SIGNAL(accepted()),
          SLOT(typeUpdateAccepted()));

  this->TrackTypeDialog->show();
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::typeUpdateAccepted()
{
  emit this->ObjectTypeUpdated();
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnTreeItemsChanged(const QList<QTreeWidgetItem*>& items,
                                                bool updateStatus)
{
  bool changed = false;

  vpTreeView* tree = this->CurrentTree();

  int numShown = 0;
  int numHidden = 0;

  // update the display status for each changed item
  foreach (QTreeWidgetItem* item, items)
    {
    int itemType, itemId;
    int parentItemId, index;
    tree->GetItemInfo(item, itemType, itemId, parentItemId, index);

    bool displayObject = item->checkState(0) != Qt::Unchecked;
    displayObject ? ++numShown : ++numHidden;

    switch (itemType)
      {
      case vgObjectTypeDefinitions::Activity:
        // @TODO: this is linear in the number of activities
        this->ActivityManager->SetActivityState(
          this->ActivityManager->GetActivity(itemId), displayObject);
        changed = true;
        break;

      case vgObjectTypeDefinitions::Event:
        this->EventModel->SetEventDisplayState(itemId, displayObject);
        changed = true;
        break;

      case vgObjectTypeDefinitions::SceneElement:
      case vgObjectTypeDefinitions::Track:
        this->TrackModel->SetTrackDisplayState(itemId, displayObject);
        changed = true;
        break;
      }
    }

  // update the viewport
  if (changed)
    {
    emit this->ItemsChanged();
    }

  // show the number of items changed in the status bar
  if (updateStatus)
    {
    QString status;
    if (numShown == 0 && numHidden == 0)
      {
      status = "0 items changed";
      }
    else
      {
      if (numShown > 0)
        {
        status += numShown == 1 ? "1 item shown"
                                : QString("%1 items shown").arg(numShown);
        if (numHidden > 0)
          {
          status += ", ";
          }
        }
      if (numHidden > 0)
        {
        status += numHidden == 1 ? "1 item hidden"
                                 : QString("%1 items hidden").arg(numHidden);
        }
      }

    emit this->ShowStatusMessage(status, 3000, this->SessionId);
    }
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnObjectTypeChanged(int index)
{
  if (this->NeedsRebuild(index))
    {
    this->RebuildTreeView();
    }
  else if (this->NeedsRefresh(index))
    {
    this->RefreshTreeView();
    }

  this->UpdateItemVisibility();

  this->SetupTab();

  // if we are switching to a track or event tab and we already have an object
  // of that type (or one of its children) selected, try to maintain the
  // selection in the new tab
  if ((this->SelectedItem.Type == vgObjectTypeDefinitions::Event &&
       this->CurrentTab() == vgObjectTypeDefinitions::Event) ||
      (this->SelectedItem.Type == vgObjectTypeDefinitions::Track &&
       this->CurrentTab() == vgObjectTypeDefinitions::Track))
    {
    this->CurrentTree()->SelectItem(this->SelectedItem.Type,
                                    this->SelectedItem.Id);
    }
  else if (this->SelectedItem.Type == vgObjectTypeDefinitions::Track &&
           this->SelectedItem.ParentId >= 0 &&
           this->CurrentTab() == vgObjectTypeDefinitions::Event)
    {
    this->CurrentTree()->SelectChildItem(vgObjectTypeDefinitions::Event,
                                         this->SelectedItem.ParentId,
                                         this->SelectedItem.Index);
    }

  // update selection
  this->OnTreeSelectionChanged();
  this->OnTreeHoverStopped();
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnTabBarContextMenu(QContextMenuEvent* event,
                                                 int /*tab*/)
{
  QMenu menu(this);

  vpTreeView* tree = this->CurrentTree();

  switch (this->CurrentTab())
    {
    case vgObjectTypeDefinitions::Activity:
      {
      menu.addAction("No Activities", tree, SLOT(HideAllActivities()));
      menu.addAction("All Activities", tree, SLOT(ShowAllActivities()));
      QMenu* activityTypeMenu = menu.addMenu("All Activity Type");

      for (int i = 0, end = this->ActivityManager->GetNumberOfActivityTypes();
           i < end; ++i)
        {
        QAction* a =
          activityTypeMenu->addAction(this->ActivityManager->GetActivityName(i),
                                      this->ShowActivityTypeMapper,
                                      SLOT(map()));
        this->ShowActivityTypeMapper->setMapping(a, i);
        }

      menu.addSeparator();
      // fall through
      }

    case vgObjectTypeDefinitions::Event:
      {
      menu.addAction("No Events", tree, SLOT(HideAllEvents()));
      menu.addAction("All Events", tree, SLOT(ShowAllEvents()));
      QMenu* eventTypeMenu = menu.addMenu("All Event Type");

      for (int i = 0; i < this->EventTypeRegistry->GetNumberOfTypes(); ++i)
        {
        const vgEventType& type = this->EventTypeRegistry->GetType(i);

        if (type.GetIsUsed())
          {
          QAction* a = eventTypeMenu->addAction(type.GetName(),
                                                this->ShowEventTypeMapper,
                                                SLOT(map()));

          this->ShowEventTypeMapper->setMapping(a, type.GetId());
          }
        }

      menu.addSeparator();
      // fall through
      }

    case vgObjectTypeDefinitions::Track:
      {
      menu.addAction("No Tracks", tree, SLOT(HideAllTracks()));
      menu.addAction("All Tracks", tree, SLOT(ShowAllTracks()));
      break;
      }

    case vgObjectTypeDefinitions::SceneElement:
      {
      menu.addAction("No Scene Elements", tree, SLOT(HideAllSceneElements()));
      menu.addAction("All Scene Elements", tree, SLOT(ShowAllSceneElements()));
      break;
      }
    }

  menu.exec(event->globalPos());
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::ShowEventType(int type)
{
  this->CurrentTree()->ShowEventType(type);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::ShowActivityType(int type)
{
  this->CurrentTree()->ShowActivityType(type);
}

//-----------------------------------------------------------------------------
std::pair<vtkVgTimeStamp, vtkVgTimeStamp>
vpObjectSelectionPanel::GetSelectedItemFrameExtents()
{
  vtkVgTimeStamp startFrame, endFrame;
  switch (this->SelectedItem.Type)
    {
    case vgObjectTypeDefinitions::Activity:
      {
      vtkVgActivity* a = this->ActivityManager->GetActivity(this->SelectedItem.Id);
      a->GetActivityFrameExtents(startFrame, endFrame);
      break;
      }
    case vgObjectTypeDefinitions::Event:
      {
      vtkVgEvent* e = this->EventModel->GetEvent(this->SelectedItem.Id);
      startFrame = e->GetStartFrame();
      endFrame = e->GetEndFrame();
      break;
      }
    case vgObjectTypeDefinitions::Track:
    case vgObjectTypeDefinitions::SceneElement:
      {
      // use track extents relative to parent event if one exists
      if (this->SelectedItem.ParentId >= 0)
        {
        vtkVgTrack* track;
        vtkVgEvent* e = this->EventModel->GetEvent(this->SelectedItem.ParentId);
        e->GetTrack(this->SelectedItem.Index, track, startFrame, endFrame);
        break;
        }

      vtkVgTrack* t = this->TrackModel->GetTrack(this->SelectedItem.Id);
      startFrame = t->GetStartFrame();
      endFrame = t->GetEndFrame();
      break;
      }
    }
  return std::make_pair(startFrame, endFrame);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::FocusItemAlone()
{
  // turn everything off
  this->ActivityManager->TurnOffAllActivities();
  this->EventModel->TurnOffAllEvents();
  this->TrackModel->TurnOffAllTracks();

  // turn on the selected item
  QTreeWidgetItem* item = this->CurrentTree()->selectedItems().first();
  this->SetItemDisplayState(item, true);

  // make sure the item type is visible
  this->FocusItem(item);
  emit this->ItemsChanged();
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::FocusItem()
{
  // first, make sure the item is selected for display
  QTreeWidgetItem* item = this->CurrentTree()->selectedItems().first();
  item->setCheckState(0, Qt::Checked);
  this->FocusItem(item);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::FocusItem(QTreeWidgetItem* item)
{
  int itemType, itemId;
  int parentItemId, index;
  this->CurrentTree()->GetItemInfo(item, itemType, itemId, parentItemId, index);

  // make sure the item type is visible
  switch (itemType)
    {
    case vgObjectTypeDefinitions::Activity:
      emit this->DisplayActivities(true);
      this->ViewCoreInstance->focusActivity(this->SessionId, itemId);
      break;

    case vgObjectTypeDefinitions::Event:
      emit this->DisplayEvents(true);
      this->ViewCoreInstance->focusEvent(this->SessionId, itemId);
      break;

    case vgObjectTypeDefinitions::Track:
      {
      emit this->DisplayTracks(true);

      // use track extents relative to parent event if one exists
      vtkVgTimeStamp startFrame, endFrame;
      if (parentItemId >= 0)
        {
        vtkVgTrack* track;
        vtkVgEvent* e = this->EventModel->GetEvent(parentItemId);
        e->GetTrack(index, track, startFrame, endFrame);
        }

      this->ViewCoreInstance->focusTrack(this->SessionId, itemId,
                                         startFrame, endFrame);
      break;
      }

    case vgObjectTypeDefinitions::SceneElement:
      emit this->DisplaySceneElements(true);

      this->ViewCoreInstance->focusTrack(this->SessionId, itemId,
                                         vtkVgTimeStamp(), vtkVgTimeStamp());
      break;
    }
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnGoToStartFrame()
{
  this->ViewCoreInstance->setCurrentTime(
    this->GetSelectedItemFrameExtents().first);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnGoToEndFrame()
{
  this->ViewCoreInstance->setCurrentTime(
    this->GetSelectedItemFrameExtents().second);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::FollowTrack()
{
  QList<QTreeWidgetItem*> trackItem = this->CurrentTree()->selectedItems();

  vgItemInfo info;
  this->CurrentTree()->GetItemInfo(trackItem[0], info.Type, info.Id,
                                   info.ParentId, info.Index);

  this->ViewCoreInstance->setIdOfTrackToFollow(info.Id);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnShowLinkedEvents()
{
  this->ToggleLinkedEvents(true);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnHideLinkedEvents()
{
  this->ToggleLinkedEvents(false);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnShowLinkedActivities()
{
  this->ToggleLinkedActivities(true);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnHideLinkedActivities()
{
  this->ToggleLinkedActivities(false);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::ToggleLinkedEvents(bool state)
{
  QList<QTreeWidgetItem*> trackItems = this->CurrentTree()->selectedItems();

  foreach (auto trackItem, trackItems)
    {
    vgItemInfo info;
    this->CurrentTree()->GetItemInfo(trackItem, info.Type, info.Id,
                                     info.ParentId, info.Index);

    std::vector<vtkVgEvent*> linkedEvents;
    this->EventModel->GetEvents(info.Id, linkedEvents);


    std::vector<vtkVgEvent*>::iterator itr = linkedEvents.begin();
    for (; itr != linkedEvents.end(); ++itr)
      {
      this->EventModel->SetEventDisplayState((*itr)->GetId(), state);
      }
    }

  this->Update();
  emit this->ItemsChanged();
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::ToggleLinkedActivities(bool state)
{
  QList<QTreeWidgetItem*> trackItems = this->CurrentTree()->selectedItems();

  foreach (auto trackItem, trackItems)
    {
    vgItemInfo info;
    this->CurrentTree()->GetItemInfo(trackItem, info.Type, info.Id,
                                     info.ParentId, info.Index);

    std::vector<vtkVgActivity*> linkedActivites;

    // Get the track.
    vtkVgTrack* track = this->TrackModel->GetTrack(info.Id);

    // Now fetch all the linked activities.
    this->ActivityManager->GetActivities(track, linkedActivites);

    std::vector<vtkVgActivity*>::iterator itr = linkedActivites.begin();
    for (; itr != linkedActivites.end(); ++itr)
      {
      this->ActivityManager->SetActivityState(*(itr), state);
      }
    }

  emit this->ItemsChanged();
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::SetupTab()
{
  int currTab = this->CurrentTab();
  QWidget* prevTree = this->Trees[this->PrevTab];
  QWidget* currTree = this->Trees[currTab];

  // tear down signal connections from previous tab's tree
  prevTree->disconnect(this);

  // connect signals to current tab's tree
  connect(currTree, SIGNAL(itemSelectionChanged()),
          this, SLOT(OnTreeSelectionChanged()));

  connect(currTree, SIGNAL(itemEntered(QTreeWidgetItem*, int)),
          this, SLOT(OnTreeHoverItemChanged(QTreeWidgetItem*)));

  connect(currTree, SIGNAL(MouseLeft()), this, SLOT(OnTreeHoverStopped()));

  connect(currTree, SIGNAL(ItemsChanged(const QList<QTreeWidgetItem*>&, bool)),
          this, SLOT(OnTreeItemsChanged(const QList<QTreeWidgetItem*>&, bool)));

  connect(currTree, SIGNAL(FocusItemAlone()), this, SLOT(FocusItemAlone()));
  connect(currTree, SIGNAL(FocusItem()), this, SLOT(FocusItem()));
  connect(currTree, SIGNAL(GoToStartFrame()), this, SLOT(OnGoToStartFrame()));
  connect(currTree, SIGNAL(GoToEndFrame()), this, SLOT(OnGoToEndFrame()));
  connect(currTree, SIGNAL(FollowTrack()), this, SLOT(FollowTrack()));

  connect(currTree, SIGNAL(ShowLinkedEvents()), this,
          SLOT(OnShowLinkedEvents()));
  connect(currTree, SIGNAL(HideLinkedEvents()), this,
          SLOT(OnHideLinkedEvents()));
  connect(currTree, SIGNAL(ShowLinkedActivities()), this,
          SLOT(OnShowLinkedActivities()));
  connect(currTree, SIGNAL(HideLinkedActivities()), this,
          SLOT(OnHideLinkedActivities()));
  connect(currTree, SIGNAL(AddEventsToGraphModel()), this,
          SLOT(OnAddEventsToGraphModel()));
  connect(currTree, SIGNAL(AddTrackEventsToGraphModel()), this,
          SLOT(OnAddTrackEventsToGraphModel()));

  connect(currTree, SIGNAL(ContextMenuOpened(QMenu&)),
          this, SLOT(OnTreeContextMenu(QMenu&)));

  this->PrevTab = currTab;
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::RefreshTreeView()
{
  this->CurrentTree()->Refresh();
  this->ClearNeedsRefresh(this->CurrentTab());
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::RebuildTreeView()
{
  switch (this->CurrentTab())
    {
    case vgObjectTypeDefinitions::Activity:
      this->Ui->activityTree->Clear();
      this->Ui->activityTree->AddAllActivities();
      break;
    case vgObjectTypeDefinitions::Event:
      this->Ui->eventTree->Clear();
      this->Ui->eventTree->AddAllEvents();
      break;
    case vgObjectTypeDefinitions::Track:
      this->Ui->trackTree->Clear();
      this->Ui->trackTree->AddAllTracks();
      break;
    case vgObjectTypeDefinitions::SceneElement:
      this->Ui->fseTree->Clear();
      this->Ui->fseTree->AddAllSceneElements();
      break;
    }
  this->UpdateSort(this->CurrentTab());
  this->ClearNeedsRebuild(this->CurrentTab());
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnSortTypeChanged(int /*index*/)
{
  this->UpdateSort(this->CurrentTab());
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnSortDirectionChanged(int /*state*/)
{
  this->UpdateSort(this->CurrentTab());
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::UpdateSort(int tab)
{
  switch (tab)
    {
    case vgObjectTypeDefinitions::Activity:
      UpdateTreeSort(this->Ui->activityTree,
                     this->Ui->activitySortType,
                     this->Ui->activitySortDescending);
      break;

    case vgObjectTypeDefinitions::Event:
      UpdateTreeSort(this->Ui->eventTree,
                     this->Ui->eventSortType,
                     this->Ui->eventSortDescending);
      break;

    case vgObjectTypeDefinitions::Track:
      UpdateTreeSort(this->Ui->trackTree,
                     this->Ui->trackSortType,
                     this->Ui->trackSortDescending);
      break;

    case vgObjectTypeDefinitions::SceneElement:
      UpdateTreeSort(this->Ui->fseTree,
                     this->Ui->fseSortType,
                     this->Ui->fseSortDescending);
      break;
    }
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnShownItemsChanged()
{
  this->UpdateItemVisibility();
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnHideAll()
{
  this->ActivityManager->TurnOffAllActivities();
  this->EventModel->TurnOffAllEvents();
  this->TrackModel->TurnOffAllTracks();
  emit this->ItemsChanged();
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnShowAll()
{
  this->ActivityManager->TurnOnAllActivities();
  this->EventModel->TurnOnAllEvents();
  this->TrackModel->TurnOnAllTracks();
  emit this->ItemsChanged();
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnAddEventsToGraphModel()
{
  vpTreeView* tree = this->CurrentTree();
  QList<QTreeWidgetItem*> items = tree->selectedItems();
  QList<int> ids;

  vgItemInfo info;
  foreach (QTreeWidgetItem* item, items)
    {
    tree->GetItemInfo(item, info.Type, info.Id,
                      info.ParentId, info.Index);
    if (info.Type == vgObjectTypeDefinitions::Event)
      {
      ids << info.Id;
      }
    }

  emit this->AddEventsToGraphModel(ids, this->SessionId);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::OnAddTrackEventsToGraphModel()
{
  emit this->AddTrackEventsToGraphModel(this->SelectedItem.Id, this->SessionId);
}

//-----------------------------------------------------------------------------
void vpObjectSelectionPanel::UpdateItemVisibility()
{
  bool showExcluded = this->Ui->showExcludedItems->checkState() == Qt::Checked;
  bool showUnchecked = this->Ui->showUncheckedItems->checkState() == Qt::Checked;
  this->CurrentTree()->SetShowExcludedItems(showExcluded);
  this->CurrentTree()->SetShowUncheckedItems(showUnchecked);
}

//-----------------------------------------------------------------------------
int vpObjectSelectionPanel::SetItemDisplayState(QTreeWidgetItem* item, bool on)
{
  int itemType, itemId;
  int parentItemId, index;
  this->CurrentTree()->GetItemInfo(item, itemType, itemId, parentItemId, index);

  switch (itemType)
    {
    case vgObjectTypeDefinitions::Activity:
      // @TODO: this is linear in the number of activities
      this->ActivityManager->SetActivityState(
        this->ActivityManager->GetActivity(itemId), on);
      emit this->ItemsChanged();
      return vgObjectTypeDefinitions::Activity;

    case vgObjectTypeDefinitions::Event:
      this->EventModel->SetEventDisplayState(itemId, on);
      emit this->ItemsChanged();
      return vgObjectTypeDefinitions::Event;

    case vgObjectTypeDefinitions::Track:
    case vgObjectTypeDefinitions::SceneElement:
      this->TrackModel->SetTrackDisplayState(itemId, on);
      emit this->ItemsChanged();
      return itemType;
    }

  return -1;
}
