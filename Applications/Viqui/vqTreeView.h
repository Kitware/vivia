// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqTreeView_h
#define __vqTreeView_h

#include <QTreeWidget>
#include <vvIqr.h>

class QAction;
class QActionGroup;
class QSignalMapper;
class QString;
class QTreeWidgetItem;
class vtkLookupTable;
class vtkVgNodeBase;
class vtkVgSceneManager;
class vtkVgVideoNode;

class vqTreeView : public QTreeWidget
{
  Q_OBJECT

public:
  vqTreeView(QWidget* parent = 0);
  virtual ~vqTreeView();

  void SetLookupTable(vtkLookupTable* lut)
    { this->LookupTable = lut; }

  void Initialize(QList<vtkVgVideoNode*> nodes,
                  bool isFeedbackList,
                  bool allowRefinement = false,
                  bool allowStarToggle = false,
                  bool showEventType = false);

  QList<vtkVgVideoNode*> GetSelectedNodes();
  QList<vtkVgVideoNode*> GetStarredNodes();

  // reimplemented from QWidget
  virtual void contextMenuEvent(QContextMenuEvent* event);
  virtual void leaveEvent(QEvent* event);

  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseDoubleClickEvent(QMouseEvent* event);

  virtual QSize sizeHint() const
    { return QSize(500, 300); }

  virtual QSize minimumSizeHint() const
    { return QSize(300, 200); }

signals:
  void NodesSelected(QList<vtkVgNodeBase*> nodes);
  void NodeActivated(vtkVgNodeBase& node);
  void MouseLeft();
  void ContextMenuOpened(QMenu& menu);
  void ItemsChanged();

  void UserScoreChanged(long long iid, int score);
  void NoteChanged(long long iid, const QString& note);
  void StarredChanged(long long iid, bool starred);

  void ShowTrackingClips(QList<vtkVgVideoNode*> nodes);

public slots:
  void SelectNodes(QList<vtkVgNodeBase*> nodes);
  void Refresh();

  void HideAllItems();
  void ShowAllItems();
  void SetShowHiddenItems(bool enable);

protected slots:
  void SetRating(int rating);
  void ItemActivated(QTreeWidgetItem* item);
  void ItemSelectionChanged();

  void ToggleStar();

  void HideAllExcept();
  void Hide();
  void Show();

  void ShowTrackingClips();
  void EditNote();

  void UpdateActivated(vtkVgVideoNode& node);

  void Reset();

protected:
  QAction* CreateStatusAction(QString, Qt::Key, Qt::Key, int);

  void AddItemForNode(vtkVgVideoNode* node);
  void UpdateItem(QTreeWidgetItem* item);

  QTreeWidgetItem* FindItem(QTreeWidgetItem* root, vtkVgNodeBase* node);
  vtkVgVideoNode* GetVideoNode(const QTreeWidgetItem* item);

  void HideUnselectedShowSelected(QTreeWidgetItem* root);

  void HideVideoItem(QTreeWidgetItem* item);
  void ShowVideoItem(QTreeWidgetItem* item);

  void SetItemRating(QTreeWidgetItem* item, vvIqr::Classification rating);

  bool CycleRating(QMouseEvent* event);
  bool ToggleStar(QMouseEvent* event);

private:
  struct TreeInternal;
  TreeInternal* Internal;

  class TreeWidgetItem;

  QSignalMapper* SetStatusMapper;

  bool IsFeedbackList;
  bool AllowRefinement;
  bool AllowStarToggle;
  bool ShowHiddenItems;

  vtkLookupTable* LookupTable;

  vtkVgNodeBase* LastActivated;

  QActionGroup* SetStatusActions;
  QAction* SetStatusGood;
  QAction* SetStatusBad;
  QAction* SetStatusNone;

  bool IgnoreActivation;
};

#endif
