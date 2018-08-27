/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpSessionView_h
#define __vpSessionView_h

#include <QWidget>

#include <vtkType.h>

#include "vtkVgTypeDefs.h"

#include "vpObjectSelectionPanel.h"

class vpViewCore;

class vtkVpTrackModel;

class vtkIdList;

class vtkVgActivityManager;
class vtkVgEventModel;

class QListWidget;
class QListWidgetItem;
class QSplitter;
class QStackedWidget;

class vpSessionView : public QWidget
{
  Q_OBJECT

public:
  vpSessionView(QWidget* parent = 0);
  virtual ~vpSessionView();

  void AddSession(vpViewCore* viewCore,
                  vtkVgActivityManager* activityManager,
                  vtkVgEventModel* eventModel,
                  vtkVpTrackModel* trackModel,
                  vtkVgEventFilter* eventFilter,
                  vtkVgTrackFilter* trackFilter,
                  vtkVgEventTypeRegistry* eventTypes,
                  vtkVgTrackTypeRegistry* trackTypes,
                  const QString& title = QString());

  void RemoveSession(int sessionId);

  int  GetSessionCount();

  int  GetCurrentSession();

  bool SelectItem(int type, int id);
  void AddAndSelectItem(int type, int id);

  bool GetSelectedItemInfo(vgItemInfo& info);
  bool GetHoveredItemInfo(vgItemInfo& info);

  QList<vtkIdType> GetSelectedItems(int type);

  void SetCurrentTab(int type);

  void Update(bool rebuild = false);

signals:
  void SelectionChanged(int sessionId);
  void HoverItemChanged(int sessionId);

  void ItemsChanged();
  void ObjectTypeUpdated();

  void CreateEvent(int type, vtkIdList* ids, int sessionId);
  void DeleteEvent(int type, int sessionId);

  void EditTrack(int trackId, int sessionId);
  void StopEditingTrack(int sessionId);
  void DeleteTrack(int trackId, int sessionId);
  void SplitTrack(int trackId, int sessionId);
  void ImproveTrack(int trackId, int sessionId);

  void AddEventsToGraphModel(QList<int> eventIds, int sessionId);
  void AddTrackEventsToGraphModel(int trackId, int sessionId);

  void ShowStatusMessage(const QString& message, int timeout, int sessionId);

  void DisplayActivities(bool);
  void DisplayEvents(bool);
  void DisplayTracks(bool);
  void DisplayTrackHeads(bool);
  void DisplaySceneElements(bool);

  void SessionChanged(int sessionId);
  void SessionVisibilityChanged(int sessionId, bool visible);

  void CloseProjectRequested(int sessionId);

public slots:
  void SetCurrentSession(int sessionId);
  void SetSessionVisible(int sessionId, bool visible);

  void FocusItem();

protected slots:
  void ReactToItemChanged(QListWidgetItem* item);

protected:
  vpObjectSelectionPanel* CurrentTab();

private:
  QStackedWidget* StackedWidget;
  QListWidget* ListWidget;
  QSplitter* Splitter;
};

#endif
