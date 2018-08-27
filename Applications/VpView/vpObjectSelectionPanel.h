/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpObjectSelectionPanel_h
#define __vpObjectSelectionPanel_h

#include <QWidget>

#include <vtkType.h>

#include "vtkVgTypeDefs.h"

namespace Ui
{
class vpObjectSelectionPanel;
}

class vpViewCore;
class vpTreeView;
class vpTrackTypeDialog;
class vtkVpTrackModel;
class vtkIdList;
class vtkVgActivityManager;
class vtkVgEventFilter;
class vtkVgEventModel;
class vtkVgEventTypeRegistry;
class vtkVgTimeStamp;
class vtkVgTrackFilter;
class vtkVgTrackTypeRegistry;

class QMenu;
class QSignalMapper;
class QTreeWidgetItem;

struct vgItemInfo
{
  int Type;
  int Id;
  int ParentId;
  int Index;
};

class vpObjectSelectionPanel : public QWidget
{
  Q_OBJECT

public:
  vpObjectSelectionPanel(QWidget* parent = 0);
  virtual ~vpObjectSelectionPanel();

  void Initialize(vpViewCore* viewCore,
                  vtkVgActivityManager* activityManager,
                  vtkVgEventModel* eventModel,
                  vtkVpTrackModel* trackModel,
                  vtkVgEventFilter* eventFilter,
                  vtkVgTrackFilter* trackFilter,
                  vtkVgEventTypeRegistry* eventTypes,
                  vtkVgTrackTypeRegistry* trackTypes,
                  int sessionId,
                  const QString& title);

  bool SelectItem(int type, int id);
  void AddAndSelectItem(int type, int id);

  bool GetSelectedItemInfo(vgItemInfo& info);
  bool GetHoveredItemInfo(vgItemInfo& info);

  QList<vtkIdType> GetSelectedItems(int type);

  void SetCurrentTab(int type);

  void Update(bool rebuild = false);

  void SetSessionId(int id) { this->SessionId = id; }
  int  GetSessionId() { return this->SessionId; }

  QString GetTitle() { return this->Title; }

signals:
  void SelectionChanged(int sessionId);
  void HoverItemChanged(int sessionId);

  void CreateEvent(int type, vtkIdList* ids, int sessionId);
  void DeleteEvent(int it, int sessionId);

  void EditTrack(int id, int sessionId);
  void StopEditingTrack(int sessionId);
  void DeleteTrack(int id, int sessionId);
  void SplitTrack(int id, int sessionId);
  void ImproveTrack(int id, int sessionId);

  void AddEventsToGraphModel(QList<int> ids, int sessionId);
  void AddTrackEventsToGraphModel(int id, int sessionId);

  void ShowStatusMessage(const QString& message, int timeout, int sessionId);

  void DisplayActivities(bool enable);
  void DisplayEvents(bool enable);
  void DisplayTracks(bool enable);
  void DisplayTrackHeads(bool enable);
  void DisplaySceneElements(bool enable);

  void ItemsChanged();

public slots:
  void FocusItemAlone();
  void FocusItem();

protected slots:
  void OnTreeSelectionChanged();
  void OnTreeContextMenu(QMenu& menu);
  void CreateEvent(int type);
  void DeleteEvent();
  void EditTrack();
  void StopEditingTrack();
  void DeleteTrack();
  void SplitTrack();
  void ImproveTrack();
  void AddEventToActivity();
  void RemoveEventFromActivity();
  void SetTracksType();

  void SetEventStatus(int status);
  void SetActivityStatus(int status);
  void SetTrackStatus(int status);

  void OnTreeHoverItemChanged(QTreeWidgetItem* item);
  void OnTreeHoverStopped();

  void OnTreeItemsChanged(const QList<QTreeWidgetItem*>& items,
                          bool updateStatus);

  void OnObjectTypeChanged(int);

  void OnTabBarContextMenu(QContextMenuEvent* event, int tab);
  void ShowEventType(int type);
  void ShowActivityType(int type);

  void OnGoToStartFrame();
  void OnGoToEndFrame();

  void OnSortTypeChanged(int index);
  void OnSortDirectionChanged(int state);

  void FollowTrack();

  void OnShowLinkedEvents();
  void OnHideLinkedEvents();

  void OnShowLinkedActivities();
  void OnHideLinkedActivities();

  void OnShownItemsChanged();

  void OnHideAll();
  void OnShowAll();

  void OnAddEventsToGraphModel();
  void OnAddTrackEventsToGraphModel();

protected:
  virtual void showEvent(QShowEvent* event);

  void ToggleLinkedEvents(bool state);
  void ToggleLinkedActivities(bool state);

private:
  void SetupTab();
  void RefreshTreeView();
  void RebuildTreeView();

  std::pair<vtkVgTimeStamp, vtkVgTimeStamp> GetSelectedItemFrameExtents();

  inline int         CurrentTab();
  inline vpTreeView* CurrentTree();

  bool NeedsRefresh(int tree)
    { return (this->RefreshFlags & (1 << tree)) != 0; }

  bool NeedsRebuild(int tree)
    { return (this->RebuildFlags & (1 << tree)) != 0; }

  void ClearNeedsRefresh(int tree)
    { this->RefreshFlags &= ~(1 << tree); }

  void ClearNeedsRebuild(int tree)
    { this->RebuildFlags &= ~(1 << tree); }

  void AddSetStatusActions(QMenu* menu, QSignalMapper* mapper, int curStatus);

  void UpdateSort(int tab);

  void UpdateItemVisibility();

  int SetItemDisplayState(QTreeWidgetItem* item, bool on);

  void FocusItem(QTreeWidgetItem* item);

private:
  Ui::vpObjectSelectionPanel* Ui;

  vpTrackTypeDialog* TrackTypeDialog;

  QSignalMapper* CreateEventMapper;
  QSignalMapper* SetEventStatusMapper;
  QSignalMapper* SetActivityStatusMapper;
  QSignalMapper* SetTrackStatusMapper;

  QSignalMapper* ShowEventTypeMapper;
  QSignalMapper* ShowActivityTypeMapper;

  int SessionId;
  QString Title;

  vpViewCore*            ViewCoreInstance;
  vtkVgActivityManager*  ActivityManager;
  vtkVgEventModel*       EventModel;
  vtkVpTrackModel*       TrackModel;

  vtkVgEventTypeRegistry* EventTypeRegistry;
  vtkVgTrackTypeRegistry* TrackTypeRegistry;

  vgItemInfo SelectedItem;
  vgItemInfo HoveredItem;

  int PrevTab;

  vpTreeView* Trees[4];

  unsigned RefreshFlags; // tree needs refresh only
  unsigned RebuildFlags; // tree needs full rebuild - this should only happen once
};

#endif
