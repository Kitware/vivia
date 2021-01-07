// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpTreeView_h
#define __vpTreeView_h

#include <QTreeWidget>

#include "vtkVgTypeDefs.h"

class vtkVgActivityManager;
class vtkVgEvent;
class vtkVgEventFilter;
class vtkVgEventModel;
class vtkVgEventTypeRegistry;
class vtkVgTrack;
class vtkVgTrackFilter;
class vtkVgTrackModel;
class vtkVgTrackTypeRegistry;

class SignalBlocker;

class vpTreeView : public QTreeWidget
{
  Q_OBJECT

public:
  typedef vgObjectTypeDefinitions                  ItemType;
  typedef vgObjectTypeDefinitions::enumObjectTypes ItemTypeEnum;

  enum SortType
    {
    ST_Id,
    ST_Name,
    ST_Normalcy,
    ST_Saliency,
    ST_Probability,
    ST_Length
    };

public:
  vpTreeView(QWidget* parent = 0);
  virtual ~vpTreeView();

  void Initialize(vtkVgActivityManager* activityManager,
                  vtkVgEventModel* eventModel,
                  vtkVgTrackModel* trackModel,
                  vtkVgEventFilter* eventFilter,
                  vtkVgTrackFilter* trackFilter,
                  vtkVgEventTypeRegistry* eventTypes,
                  vtkVgTrackTypeRegistry* trackTypes);

  void AddAllActivities();
  void AddAllEvents();
  void AddAllTracks();
  void AddAllSceneElements();

  void AddAndSelectTrack(vtkVgTrack* track);
  void AddAndSelectEvent(vtkVgEvent* event);
  void AddAndSelectSceneElement(vtkVgTrack* track);

  void Clear();
  void Refresh();

  bool SelectItem(int type, int id);
  bool SelectChildItem(int parentType, int parentId, int index);

  void GetItemInfo(QTreeWidgetItem* item,
                   int& type, int& id,
                   int& parentId, int& index);

  void UpdateItemStatus(QTreeWidgetItem* item);

  void UpdateActivityItem(QTreeWidgetItem* item);

  // reimplemented from QWidget
  virtual QSize sizeHint() const;
  virtual void contextMenuEvent(QContextMenuEvent* event);
  virtual void leaveEvent(QEvent* event);

  int GetSortType() { return this->SortType; }

  static const char* GetSortTypeString(int sortType);

signals:
  void ItemsChanged(const QList<QTreeWidgetItem*>& items,
                    bool updateStatus = true);

  void MouseLeft();
  void ContextMenuOpened(QMenu& menu);

  void FocusItemAlone();
  void FocusItem();
  void GoToStartFrame();
  void GoToEndFrame();
  void FollowTrack();

  void ShowLinkedEvents();
  void HideLinkedEvents();

  void ShowLinkedActivities();
  void HideLinkedActivities();

  void AddEventsToGraphModel();
  void AddTrackEventsToGraphModel();

public slots:
  void ShowAll();
  void HideAll();

  void SortBy(int type, Qt::SortOrder direction);

  void ShowAllTracks();
  void HideAllTracks();

  void ShowEventType(int type);
  void ShowAllEvents();
  void HideAllEvents();

  void ShowActivityType(int type);
  void ShowAllActivities();
  void HideAllActivities();

  void ShowAllSceneElements();
  void HideAllSceneElements();

  void SetShowExcludedItems(bool show);
  void SetShowUncheckedItems(bool show);

private slots:
  void OnItemChanged(QTreeWidgetItem* item);

  void ShowItems();
  void HideItems();
  void HideItemsExcept();

  void ToggleSticky();

private:
  enum ItemDataRole
    {
    IDR_ItemType = Qt::UserRole,
    IDR_ItemId,
    IDR_ItemIndex,
    IDR_ItemSticky,
    IDR_ItemSortValsStart // must be last
    };

  class TreeWidgetItem;
  friend class SignalBlocker;

  QTreeWidgetItem* CreateItem(ItemTypeEnum type, const char* name, int id);
  QTreeWidgetItem* CreateItem(vtkVgTrack* track, bool isFseTrack = false);

  void AddEventTracks(vtkVgEvent* event, QTreeWidgetItem* eventItem);

  bool RefreshItem(QTreeWidgetItem* item, bool isTopLevel = false);

  void RefreshRecursive(QTreeWidgetItem* root, int depth);

  void SetStateRecursive(QTreeWidgetItem* root, Qt::CheckState state,
                         QList<QTreeWidgetItem*>& changed);

  void SetStateRecursive(QTreeWidgetItem* root, Qt::CheckState state,
                         QList<QTreeWidgetItem*>& changed, int type);

  void SetEventStateRecursive(QTreeWidgetItem* root, Qt::CheckState state,
                              QList<QTreeWidgetItem*>& changed, int eventType);

  void SetActivityStateRecursive(QTreeWidgetItem* root, Qt::CheckState state,
                                 QList<QTreeWidgetItem*>& changed, int activityType);

  QTreeWidgetItem* FindItem(QTreeWidgetItem* root, int type, int id);
  QTreeWidgetItem* FindTopLevelItem(int type, int id);

  void HideUnselectedShowSelected(QTreeWidgetItem* root,
                                  QList<QTreeWidgetItem*>& changed);

  void SetAll(Qt::CheckState state);
  void SetAll(Qt::CheckState state, int type);
  void SetAllEvents(Qt::CheckState state, int eventType);
  void SetAllActivities(Qt::CheckState state, int activityType);

private:
  vtkVgActivityManager* ActivityManager;
  vtkVgEventModel* EventModel;
  vtkVgTrackModel* TrackModel;

  vtkVgTrackFilter* TrackFilter;
  vtkVgEventFilter* EventFilter;

  vtkVgTrackTypeRegistry* TrackTypeRegistry;
  vtkVgEventTypeRegistry* EventTypeRegistry;

  int SortType;
  bool ShowExcludedItems;
  bool ShowUncheckedItems;
};

#endif
