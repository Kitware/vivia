// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vqTreeView.h"

#include <QActionGroup>
#include <QApplication>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QIcon>
#include <QMenu>
#include <QPalette>
#include <QSignalMapper>
#include <QTreeWidget>

#include <qtUtil.h>

#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>

#include <vtkVgGroupNode.h>
#include <vtkVgVideoModel0.h>
#include <vtkVgVideoNode.h>
#include <vtkVgVideoRepresentationBase0.h>

#include <vgTextEditDialog.h>
#include <vgUnixTime.h>

#include <algorithm>

#include "vqEventInfo.h"
#include "vqUtil.h"

namespace
{

//-----------------------------------------------------------------------------
enum TreeColumn
{
  TC_RatingIcon,
  TC_Id,
  TC_Rank,
  TC_Star,
  TC_Score,
  TC_Type,
  TC_Color,
  TC_MissionId,
  TC_StartTime,
  TC_EndTime,
  TC_Duration,
  TC_Date,
  TC_Rating,
  TC_Source,
  TC_EventType,
  TC_Note,
  TC_NumColumnTypes
};

//-----------------------------------------------------------------------------
struct ColumnInfo
{
  TreeColumn Id;
  const char* Name;
  const char* ToolTip;
};

//-----------------------------------------------------------------------------
struct BlockSignals
{
  vqTreeView* User;

  BlockSignals(vqTreeView* user)
    : User(user)
    {
    this->User->blockSignals(true);
    }

  ~BlockSignals()
    {
    this->User->blockSignals(false);
    }
};

} // end anonymous namespace

//-----------------------------------------------------------------------------
struct vqTreeView::TreeInternal
{
  QIcon YesIcon, NoIcon, BlankIcon, StarIcon, StarOffIcon;

  std::vector<ColumnInfo> Columns;
  int ColumnMap[TC_NumColumnTypes];

  void ResetColumns()
    {
    this->Columns.clear();
    std::fill(this->ColumnMap, this->ColumnMap + TC_NumColumnTypes, -1);
    }

  void AddColumn(TreeColumn id, const char* name, const char* tooltip = 0)
    {
    this->ColumnMap[id] = static_cast<int>(this->Columns.size());

    ColumnInfo ci;
    ci.Id = id;
    ci.Name = name;
    ci.ToolTip = tooltip ? tooltip : name;
    this->Columns.push_back(ci);
    }

  int GetColumn(TreeColumn id)
    {
    return this->ColumnMap[id];
    }
};

//-----------------------------------------------------------------------------
class vqTreeView::TreeWidgetItem : public QTreeWidgetItem
{
  vqTreeView* Tree;

public:
  TreeWidgetItem(vqTreeView* parent, const QStringList& strings)
    : QTreeWidgetItem(strings), Tree(parent)
    {}

private:
  virtual bool operator<(const QTreeWidgetItem& other) const
    {
    if (Tree->sortColumn() == Tree->Internal->GetColumn(TC_RatingIcon))
      {
      return Tree->GetVideoNode(this)->GetUserScore() <
             Tree->GetVideoNode(&other)->GetUserScore();
      }
    if (Tree->sortColumn() == Tree->Internal->GetColumn(TC_Star))
      {
      return Tree->GetVideoNode(this)->GetIsStarResult() <
             Tree->GetVideoNode(&other)->GetIsStarResult();
      }
    return QTreeWidgetItem::operator<(other);
    }

  virtual QVariant data(int column, int role) const
    {
    if (role == Qt::ToolTipRole)
      {
      if (column == Tree->Internal->GetColumn(TC_RatingIcon))
        {
        return vqUtil::uiIqrClassificationString(
                 static_cast<vvIqr::Classification>(
                   Tree->GetVideoNode(this)->GetUserScore()));
        }
      if (char* note = Tree->GetVideoNode(this)->GetNote())
        {
        return QString(note);
        }
      if (!Tree->GetVideoNode(this)->GetHasVideoData())
        {
        return "No video data available";
        }
      }

    return QTreeWidgetItem::data(column, role);
    }
};

//-----------------------------------------------------------------------------
vqTreeView::vqTreeView(QWidget* p)
  : QTreeWidget(p)
{
  this->Internal = new TreeInternal;

  this->IsFeedbackList = false;
  this->AllowRefinement = false;
  this->AllowStarToggle = false;
  this->ShowHiddenItems = true;
  this->LookupTable = 0;
  this->IgnoreActivation = false;

  this->LastActivated = 0;

  // setup status icons to be used later
  this->Internal->YesIcon = qtUtil::standardIcon("okay", 16);
  this->Internal->NoIcon = qtUtil::standardIcon("cancel", 16);
  this->Internal->BlankIcon = qtUtil::standardIcon("blank", 16);
  this->Internal->StarIcon = qtUtil::standardIcon("star", 16);
  this->Internal->StarOffIcon = qtUtil::standardIcon("star-off", 16);

  this->SetStatusMapper = new QSignalMapper;

  connect(this->SetStatusMapper, SIGNAL(mapped(int)),
          this, SLOT(SetRating(int)));

  connect(this,
          SIGNAL(itemActivated(QTreeWidgetItem*, int)),
          SLOT(ItemActivated(QTreeWidgetItem*)));

  connect(this,
          SIGNAL(itemSelectionChanged()),
          SLOT(ItemSelectionChanged()));

  // set up IQR status actions with keyboard shortcuts N,G,B
  this->SetStatusActions = new QActionGroup(this);
  this->SetStatusGood = this->CreateStatusAction(
                          vqUtil::uiIqrClassificationString(vvIqr::PositiveExample,
                              vqUtil::UI_IncludeAccelerator),
                          Qt::Key_Plus, Qt::Key_Equal, vvIqr::PositiveExample);
  this->SetStatusBad = this->CreateStatusAction(
                         vqUtil::uiIqrClassificationString(vvIqr::NegativeExample,
                             vqUtil::UI_IncludeAccelerator),
                         Qt::Key_Minus, Qt::Key_Underscore, vvIqr::NegativeExample);
  this->SetStatusNone = this->CreateStatusAction(
                          vqUtil::uiIqrClassificationString(vvIqr::UnclassifiedExample,
                              vqUtil::UI_IncludeAccelerator),
                          Qt::Key_Slash, Qt::Key_Backslash, vvIqr::UnclassifiedExample);
}

//-----------------------------------------------------------------------------
vqTreeView::~vqTreeView()
{
  this->LastActivated = 0;

  delete this->SetStatusMapper;
  delete this->Internal;
}

//-----------------------------------------------------------------------------
QAction* vqTreeView::CreateStatusAction(QString text, Qt::Key shortcut,
                                        Qt::Key otherShortcut, int mapValue)
{
  QAction* action = new QAction(text, this);
  action->setCheckable(true);
  QList<QKeySequence> shortcuts;
  shortcuts << shortcut << otherShortcut;
  action->setShortcuts(shortcuts);
  action->setShortcutContext(Qt::WidgetShortcut);
  this->SetStatusMapper->setMapping(action, mapValue);
  connect(action, SIGNAL(triggered(bool)), this->SetStatusMapper, SLOT(map()));
  this->addAction(action);
  this->SetStatusActions->addAction(action);
  return action;
}

//-----------------------------------------------------------------------------
void vqTreeView::Initialize(QList<vtkVgVideoNode*> nodes,
                            bool isFeedbackList,
                            bool allowRefinement,
                            bool allowStarToggle,
                            bool showEventType)
{
  this->Internal->ResetColumns();

  this->Internal->AddColumn(TC_RatingIcon,  "", "Rating");
  this->Internal->AddColumn(TC_Id,        "Id", "Result Id");
  this->Internal->AddColumn(TC_Rank,    "Rank", "Result Rank");

  if (allowStarToggle)
    {
    this->Internal->AddColumn(TC_Star, "", "Starred");
    }

  if (showEventType)
    {
    this->Internal->AddColumn(TC_EventType, "Event Type");
    }
  else
    {
    if (isFeedbackList)
      {
      this->Internal->AddColumn(TC_Score, "Preference",
                                "Feedback Preference Score");
      }
    else
      {
      this->Internal->AddColumn(TC_Score, "Score", "Relevancy Score");
      }

    this->Internal->AddColumn(TC_Type,  "*", "Feedback Requested");
    }

  this->Internal->AddColumn(TC_Color,     "Color");
  this->Internal->AddColumn(TC_MissionId, "Mission Id");
  this->Internal->AddColumn(TC_StartTime, "Start Time");
  this->Internal->AddColumn(TC_EndTime,   "End Time");
  this->Internal->AddColumn(TC_Duration,  "Duration");
  this->Internal->AddColumn(TC_Date,      "Date",   "Start Date");
  this->Internal->AddColumn(TC_Rating,    "Rating");
  this->Internal->AddColumn(TC_Source,    "Source", "Source Clip Location");
  this->Internal->AddColumn(TC_Note,      "Note");

  this->setColumnCount(static_cast<int>(this->Internal->Columns.size()));

  if (!showEventType)
    {
    // center the "*"
    this->model()->setHeaderData(this->Internal->GetColumn(TC_Type),
                                 Qt::Horizontal,
                                 Qt::AlignHCenter, Qt::TextAlignmentRole);
    }

  QStringList labels;
  for (size_t i = 0, end = this->Internal->Columns.size(); i < end; ++i)
    {
    ColumnInfo& ci = this->Internal->Columns[i];
    labels.append(ci.Name);

    // set tooltip for column header
    this->model()->setHeaderData(static_cast<int>(i), Qt::Horizontal,
                                 ci.ToolTip, Qt::ToolTipRole);
    }
  this->setHeaderLabels(labels);
  this->setHeaderHidden(false);

  // some columns get icons for their header
  this->headerItem()->setIcon(this->Internal->GetColumn(TC_RatingIcon),
                              this->Internal->YesIcon);
  if (allowStarToggle)
    {
    this->headerItem()->setIcon(this->Internal->GetColumn(TC_Star),
                                this->Internal->StarIcon);
    }

  this->IsFeedbackList = isFeedbackList;
  this->AllowRefinement = allowRefinement;
  this->AllowStarToggle = allowStarToggle;

  this->setSortingEnabled(false);
  this->clear();

  // the node list should already be sorted by score, giving us implicit ranks
  foreach (vtkVgVideoNode* node, nodes)
    this->AddItemForNode(node);

  for (int i = 0; i < labels.count(); ++i)
    this->resizeColumnToContents(i);

  if (isFeedbackList)
    {
    this->header()->setSortIndicator(this->Internal->GetColumn(TC_Score),
                                     Qt::DescendingOrder);
    }
  else
    {
    this->header()->setSortIndicator(this->Internal->GetColumn(TC_Rank),
                                     Qt::AscendingOrder);
    }

  this->setSortingEnabled(true);

  // Resize the id column again, since resizeColumnToContents doesn't seem to
  // work correctly when sorting is disabled *and* an icon is present.
  this->resizeColumnToContents(this->Internal->GetColumn(TC_Id));
}

//-----------------------------------------------------------------------------
void vqTreeView::AddItemForNode(vtkVgVideoNode* node)
{
  QStringList strings;
  for (size_t i = 0, end = this->Internal->Columns.size(); i < end; ++i)
    {
    switch (this->Internal->Columns[i].Id)
      {
      default:
        Q_ASSERT_X(0, __FUNCTION__, "unknown column id");
        break;
      case TC_RatingIcon:
      case TC_Id:
      case TC_Rank:
      case TC_Star:
      case TC_Color:
      case TC_Rating:
      case TC_Note:
        strings.append(QString());
        break;
      case TC_Score:
        {
        double score = this->IsFeedbackList ? node->GetPreferenceScore() :
                       node->GetRelevancyScore();
        strings.append(QString("%1").arg(score, 0, 'f', 6));
        break;
        }
      case TC_Type:
        strings.append(node->GetIsRefinementResult() ? "*" : QString());
        break;
      case TC_MissionId:
        strings.append(node->GetMissionId());
        break;
      case TC_StartTime:
        strings.append(vgUnixTime(node->GetTimeRange()[0]).timeString());
        break;
      case TC_EndTime:
        strings.append(vgUnixTime(node->GetTimeRange()[1]).timeString());
        break;
      case TC_Duration:
        strings.append(vgUnixTime(node->GetTimeRange()[1] -
                                  node->GetTimeRange()[0]).timeString());
        break;
      case TC_Date:
        strings.append(vgUnixTime(node->GetTimeRange()[0]).dateString());
        break;
      case TC_Source:
        strings.append(node->GetStreamId());
        break;
      case TC_EventType:
        int at = node->GetActivityType();
        strings.append(at >= 0 ? vqEventInfo::name(at) : QString());
        break;
      }
    }

  TreeWidgetItem* item = new TreeWidgetItem(this, strings);
  item->setData(0, Qt::UserRole, QVariant::fromValue((void*) node));

  // prevent row height from changing
  item->setData(0, Qt::SizeHintRole, 10);

  item->setData(this->Internal->GetColumn(TC_Id),
                Qt::DisplayRole,
                node->GetInstanceId());
  item->setData(this->Internal->GetColumn(TC_Rank),
                Qt::DisplayRole,
                node->GetRank());

  int typeCol = this->Internal->GetColumn(TC_Type);
  if (typeCol >= 0)
    {
    item->setTextAlignment(typeCol, Qt::AlignHCenter);
    }

  this->UpdateItem(item);

  this->addTopLevelItem(item);
}

//-----------------------------------------------------------------------------
void vqTreeView::UpdateItem(QTreeWidgetItem* item)
{
  vtkVgVideoNode* node = this->GetVideoNode(item);
  if (!node)
    {
    return;
    }

  // are we showing hidden items in the list?
  if (!this->ShowHiddenItems &&
      node->GetVisibleNodeMask() != vtkVgNodeBase::VISIBLE_NODE_MASK)
    {
    item->setHidden(true);
    return;
    }
  else
    {
    item->setHidden(false);
    }

  if (this->AllowStarToggle)
    {
    item->setIcon(this->Internal->GetColumn(TC_Star),
                  node->GetIsStarResult() ? this->Internal->StarIcon :
                  this->Internal->StarOffIcon);
    }

  vvIqr::Classification rating =
    static_cast<vvIqr::Classification>(node->GetUserScore());

  int ratingCol = this->Internal->GetColumn(TC_RatingIcon);
  switch (rating)
    {
    case vvIqr::UnclassifiedExample:
      item->setIcon(ratingCol, this->Internal->BlankIcon);
      break;
    case vvIqr::PositiveExample:
      item->setIcon(ratingCol, this->Internal->YesIcon);
      break;
    case vvIqr::NegativeExample:
      item->setIcon(ratingCol, this->Internal->NoIcon);
      break;
    }
  item->setText(this->Internal->GetColumn(TC_Rating),
                vqUtil::uiIqrClassificationString(rating));

  item->setText(this->Internal->GetColumn(TC_Note), node->GetNote());

  // show indicator color
  if (this->LookupTable)
    {
    if (node->GetVisibleNodeMask() == vtkVgNodeBase::VISIBLE_NODE_MASK)
      {
      unsigned char* c = this->LookupTable->MapValue(node->GetColorScalar());
      QColor color(c[0], c[1], c[2]);
      item->setBackground(this->Internal->GetColumn(TC_Color), QBrush(color));
      }
    else
      {
      item->setBackground(this->Internal->GetColumn(TC_Color), QBrush());
      }
    }

  QPalette palette;
  if (node->GetVisibleNodeMask() != vtkVgNodeBase::VISIBLE_NODE_MASK)
    {
    palette.setCurrentColorGroup(QPalette::Disabled);
    }
  else if (!node->GetHasVideoData())
    {
    palette.setColor(QPalette::WindowText, Qt::red);
    }

  const QBrush& brush = palette.windowText();
  for (int i = 0; i < item->columnCount(); ++i)
    item->setForeground(i, brush);
}

//-----------------------------------------------------------------------------
void vqTreeView::contextMenuEvent(QContextMenuEvent* event)
{
  vtkVgVideoNode* vidNode = 0;
  int numSelectedItems = this->selectedItems().size();
  if (numSelectedItems == 1)
    {
    vidNode = this->GetVideoNode(this->selectedItems().front());
    if (!vidNode)
      {
      return;
      }
    }

  QMenu menu(this);

  // add menu items for controlling clip visibility
  menu.addAction("Hide All Except", this, SLOT(HideAllExcept()));
  if (numSelectedItems != 1
      || vidNode->GetVisibleNodeMask() == vtkVgNodeBase::VISIBLE_NODE_MASK)
    {
    menu.addAction("Hide", this, SLOT(Hide()));
    }
  if (numSelectedItems != 1
      || vidNode->GetVisibleNodeMask() != vtkVgNodeBase::VISIBLE_NODE_MASK)
    {
    menu.addAction("Show", this, SLOT(Show()));
    }

  if (this->AllowRefinement)
    {
    menu.addSeparator();

    // add menu for setting IQR status
    QMenu* submenu = menu.addMenu("Rate As");
    submenu->addAction(this->SetStatusGood);
    submenu->addAction(this->SetStatusBad);
    submenu->addAction(this->SetStatusNone);

    this->SetStatusNone->setChecked(
      vidNode && vidNode->GetUserScore() == vvIqr::UnclassifiedExample);
    this->SetStatusGood->setChecked(
      vidNode && vidNode->GetUserScore() == vvIqr::PositiveExample);
    this->SetStatusBad->setChecked(
      vidNode && vidNode->GetUserScore() == vvIqr::NegativeExample);
    }

  if (numSelectedItems != 0)
    {
    if (this->AllowStarToggle)
      {
      bool allStar = true;
      foreach (QTreeWidgetItem* item, this->selectedItems())
        {
        vtkVgVideoNode* node = this->GetVideoNode(item);
        if (node && !node->GetIsStarResult())
          {
          allStar = false;
          break;
          }
        }

      QAction* a = menu.addAction("Star", this, SLOT(ToggleStar()));
      a->setCheckable(true);
      a->setChecked(allStar);
      }

    menu.addSeparator();
    menu.addAction("View Tracking Clip(s)", this, SLOT(ShowTrackingClips()));
    menu.addSeparator();
    QAction* a = menu.addAction("Edit Note...", this, SLOT(EditNote()));
    a->setEnabled(numSelectedItems == 1);
    }

  // allow someone else to add items to the menu
  emit this->ContextMenuOpened(menu);

  menu.exec(event->globalPos());
}

//-----------------------------------------------------------------------------
void vqTreeView::leaveEvent(QEvent* event)
{
  QTreeWidget::leaveEvent(event);
  emit this->MouseLeft();
}

//-----------------------------------------------------------------------------
void vqTreeView::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
    {
    if (this->ToggleStar(event))
      ;
    else if (this->CycleRating(event))
      {
      emit this->ItemsChanged();
      }
    }

  this->QTreeWidget::mousePressEvent(event);
}

//-----------------------------------------------------------------------------
void vqTreeView::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
    {
    if (this->ToggleStar(event))
      {
      this->IgnoreActivation = true;
      }
    else if (this->CycleRating(event))
      {
      emit this->ItemsChanged();
      this->IgnoreActivation = true;
      }
    }

  this->QTreeWidget::mouseDoubleClickEvent(event);
  this->IgnoreActivation = false;
}

//-----------------------------------------------------------------------------
bool vqTreeView::CycleRating(QMouseEvent* event)
{
  // Cycle through IQR states if there was a click in the IQR column.
  QModelIndex indexClicked = indexAt(event->pos());
  if (indexClicked.isValid() &&
      indexClicked.column() == this->Internal->GetColumn(TC_RatingIcon))
    {
    if (QTreeWidgetItem* item = this->itemFromIndex(indexClicked))
      {
      if (vtkVgVideoNode* vidNode = this->GetVideoNode(item))
        {
        int status = vidNode->GetUserScore();
        status = (status + 1) % 3;
        this->SetItemRating(item,
                            static_cast<vvIqr::Classification>(status));
        return true;
        }
      }
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vqTreeView::ToggleStar(QMouseEvent* event)
{
  if (!this->AllowStarToggle)
    {
    return false;
    }

  QModelIndex indexClicked = indexAt(event->pos());
  if (indexClicked.isValid() &&
      indexClicked.column() == this->Internal->GetColumn(TC_Star))
    {
    if (QTreeWidgetItem* item = this->itemFromIndex(indexClicked))
      {
      if (vtkVgVideoNode* vidNode = this->GetVideoNode(item))
        {
        vidNode->SetIsStarResult(!vidNode->GetIsStarResult());
        this->UpdateItem(item);
        emit this->StarredChanged(vidNode->GetInstanceId(),
                                  vidNode->GetIsStarResult());
        return true;
        }
      }
    }

  return false;
}

//-----------------------------------------------------------------------------
void vqTreeView::SetItemRating(
  QTreeWidgetItem* item, vvIqr::Classification rating)
{
  if (!item)
    {
    return;
    }

  if (vtkVgVideoNode* vidNode = this->GetVideoNode(item))
    {
    vidNode->SetUserScore(rating);
    this->UpdateItem(item);
    emit this->UserScoreChanged(vidNode->GetInstanceId(), rating);
    }
}

//-----------------------------------------------------------------------------
void vqTreeView::SetRating(int rating)
{
  QList<QTreeWidgetItem*> items = this->selectedItems();
  foreach (QTreeWidgetItem* item, items)
    {
    this->SetItemRating(item, static_cast<vvIqr::Classification>(rating));
    }
  emit this->ItemsChanged();
}

//-----------------------------------------------------------------------------
void vqTreeView::ToggleStar()
{
  bool star = static_cast<QAction*>(this->sender())->isChecked();

  foreach (QTreeWidgetItem* item, this->selectedItems())
    {
    if (vtkVgVideoNode* vidNode = this->GetVideoNode(item))
      {
      vidNode->SetIsStarResult(star);
      this->UpdateItem(item);
      emit this->StarredChanged(vidNode->GetInstanceId(), star);
      }
    }
}

//-----------------------------------------------------------------------------
void vqTreeView::SelectNodes(QList<vtkVgNodeBase*> nodes)
{
  QTreeWidgetItem* root = this->invisibleRootItem();
  if (!root)
    {
    return;
    }

  QItemSelection selection;
  foreach (vtkVgNodeBase* node, nodes)
    {
    if (QTreeWidgetItem* item = this->FindItem(root, node))
      {
      QModelIndex idx = this->indexFromItem(item);
      selection.select(idx, idx);
      }
    }

  BlockSignals bs(this);

  if (selection.empty())
    {
    this->selectionModel()->clear();
    return;
    }

  this->selectionModel()->select(selection,
                                 QItemSelectionModel::ClearAndSelect |
                                 QItemSelectionModel::Rows);

  this->selectionModel()->setCurrentIndex(
    selection.front().topLeft(),
    QItemSelectionModel::Rows | QItemSelectionModel::NoUpdate);
}

//-----------------------------------------------------------------------------
void vqTreeView::Refresh()
{
  for (int i = 0; i < this->topLevelItemCount(); ++i)
    {
    this->UpdateItem(this->topLevelItem(i));
    }
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* vqTreeView::FindItem(QTreeWidgetItem* root, vtkVgNodeBase* node)
{
  vtkVgVideoNode* vidNode = dynamic_cast<vtkVgVideoNode*>(node);
  if (!vidNode)
    {
    return 0;
    }

  int numChildren = root->childCount();
  for (int i = 0; i < numChildren; ++i)
    {
    // look at this child
    QTreeWidgetItem* item = root->child(i);
    if (item->data(0, Qt::UserRole).value<void*>() == static_cast<void*>(node))
      {
      return item;
      }
    // recurse
    if (QTreeWidgetItem* child = this->FindItem(item, node))
      {
      return child;
      }
    }
  return 0; // not found
}

//-----------------------------------------------------------------------------
inline vtkVgVideoNode* vqTreeView::GetVideoNode(const QTreeWidgetItem* item)
{
  vtkVgNodeBase* node =
    static_cast<vtkVgNodeBase*>(item->data(0, Qt::UserRole).value<void*>());

  return dynamic_cast<vtkVgVideoNode*>(node);
}

//-----------------------------------------------------------------------------
void vqTreeView::ItemActivated(QTreeWidgetItem* item)
{
  if (this->IgnoreActivation)
    {
    return;
    }

  if (vtkVgVideoNode::SmartPtr videoNode = this->GetVideoNode(item))
    {
    // If this node is hidden don't activate it.
    if (videoNode->GetVisibleNodeMask() == ~vtkVgNodeBase::VISIBLE_NODE_MASK)
      {
      return;
      }
    else
      {
      // Nothing to do.
      }
    }
  else
    {
    return;
    }

  // NOTE: This may need to be commented out.
  this->ShowVideoItem(item);

  vtkVgNodeBase* node =
    static_cast<vtkVgNodeBase*>(item->data(0, Qt::UserRole).value<void*>());

  if (!this->LastActivated || this->LastActivated != node)
    {
    this->LastActivated = node;
    }
  else
    {
    this->LastActivated = 0;
    }

  emit this->NodeActivated(*node);
}

//-----------------------------------------------------------------------------
void vqTreeView::ItemSelectionChanged()
{
  QList<vtkVgNodeBase*> nodes;
  QList<QTreeWidgetItem*> items = this->selectedItems();
  foreach (QTreeWidgetItem* item, items)
    {
    nodes.append(
      static_cast<vtkVgNodeBase*>(item->data(0, Qt::UserRole).value<void*>()));
    }

  emit this->NodesSelected(nodes);
}

//-----------------------------------------------------------------------------
void vqTreeView::HideUnselectedShowSelected(QTreeWidgetItem* root)
{
  // recursively hide all the unselected objects and show the selected ones
  for (int i = 0; i < root->childCount(); ++i)
    {
    QTreeWidgetItem* child = root->child(i);
    if (child->isSelected())
      {
      this->ShowVideoItem(child);
      }
    else
      {
      this->HideVideoItem(child);
      }
    this->HideUnselectedShowSelected(child);
    }
}

//-----------------------------------------------------------------------------
void vqTreeView::HideAllExcept()
{
  this->HideUnselectedShowSelected(this->invisibleRootItem());
  emit this->ItemsChanged();
  this->Refresh();
}

//-----------------------------------------------------------------------------
void vqTreeView::Hide()
{
  // hide all selected objects
  QList<QTreeWidgetItem*> selected = this->selectedItems();
  foreach (QTreeWidgetItem* item, selected)
    {
    this->HideVideoItem(item);
    }
  emit this->ItemsChanged();
  this->Refresh();
}

//-----------------------------------------------------------------------------
void vqTreeView::Show()
{
  // show all selected objects
  QList<QTreeWidgetItem*> selected = this->selectedItems();
  foreach (QTreeWidgetItem* item, selected)
    {
    this->ShowVideoItem(item);
    }
  emit this->ItemsChanged();
  this->Refresh();
}

//-----------------------------------------------------------------------------
void vqTreeView::HideVideoItem(QTreeWidgetItem* item)
{
  vtkVgVideoNode* vidNode = this->GetVideoNode(item);
  if (vidNode)
    {

    // Check if a video is playing, disallow hiding.
    if (vtkVgVideoRepresentationBase0::SmartPtr vidRep =
          vidNode->GetVideoRepresentation())
      {
      vtkVgVideoModel0::SmartPtr vidModel = vidRep->GetVideoModel();
      if (vidModel->IsPlaying())
        {
        return;
        }
      }

    vidNode->SetVisible(0);
    vidNode->SetVisibleNodeMask(~vtkVgNodeBase::VISIBLE_NODE_MASK);
    }
}

//-----------------------------------------------------------------------------
void vqTreeView::ShowVideoItem(QTreeWidgetItem* item)
{
  bool someVideoPlayingOrPaused = false;

  // Make sure that we are not playing any video. If we are,
  // we cannot change visibility of any video nodes in the scene.
  vtkVgVideoNode* lastPlayed =
    vtkVgVideoNode::SafeDownCast(this->LastActivated);
  if (lastPlayed && lastPlayed->GetVideoRepresentation())
    {
    vtkVgVideoModel0* vidModel =
      lastPlayed->GetVideoRepresentation()->GetVideoModel();
    if (vidModel)
      {
      if (vidModel->IsPlaying() || vidModel->IsPaused())
        {
        someVideoPlayingOrPaused = true;
        }
      }
    }

  vtkVgVideoNode* vidNode = this->GetVideoNode(item);
  if (vidNode)
    {
    if (someVideoPlayingOrPaused)
      {
      vidNode->SetVisibleNodeMask(vtkVgNodeBase::VISIBLE_NODE_MASK);
      return;
      }

    vidNode->SetVisibleNodeMask(vtkVgNodeBase::VISIBLE_NODE_MASK);
    vidNode->SetVisible(1);
    }
}

//-----------------------------------------------------------------------------
void vqTreeView::HideAllItems()
{
  for (int i = 0; i < this->topLevelItemCount(); ++i)
    {
    this->HideVideoItem(this->topLevelItem(i));
    }
  emit this->ItemsChanged();
  this->Refresh();
}

//-----------------------------------------------------------------------------
void vqTreeView::ShowAllItems()
{
  for (int i = 0; i < this->topLevelItemCount(); ++i)
    {
    this->ShowVideoItem(this->topLevelItem(i));
    }
  emit this->ItemsChanged();
  this->Refresh();
}

//-----------------------------------------------------------------------------
void vqTreeView::SetShowHiddenItems(bool enable)
{
  if (enable == this->ShowHiddenItems)
    {
    return;
    }
  this->ShowHiddenItems = enable;
  this->Refresh();
}

//-----------------------------------------------------------------------------
QList<vtkVgVideoNode*> vqTreeView::GetSelectedNodes()
{
  QList<vtkVgVideoNode*> nodeList;

  QList<QTreeWidgetItem*> items = this->selectedItems();
  foreach (QTreeWidgetItem* item, items)
    {
    nodeList.append(this->GetVideoNode(item));
    }

  return nodeList;
}

//-----------------------------------------------------------------------------
QList<vtkVgVideoNode*> vqTreeView::GetStarredNodes()
{
  QList<vtkVgVideoNode*> nodeList;

  foreach_child (auto* item, this->invisibleRootItem())
    {
    vtkVgVideoNode* node = this->GetVideoNode(item);
    if (node->GetIsStarResult())
      {
      nodeList.append(node);
      }
    }

  return nodeList;
}

//-----------------------------------------------------------------------------
void vqTreeView::ShowTrackingClips()
{
  if (this->selectedItems().empty())
    {
    return;
    }

  QList<QTreeWidgetItem*> items = this->selectedItems();
  QList<vtkVgVideoNode*> nodes;

  foreach (QTreeWidgetItem* item, items)
    {
    nodes.append(this->GetVideoNode(item));
    }

  emit this->ShowTrackingClips(nodes);
}

//-----------------------------------------------------------------------------
void vqTreeView::EditNote()
{
  if (this->selectedItems().empty())
    {
    return;
    }

  vtkVgVideoNode* node = this->GetVideoNode(this->selectedItems().front());

  bool ok = false;
  QString s =
    vgTextEditDialog::getText(this, "Edit Result Note", node->GetNote(), &ok);

  if (!ok)
    {
    return;
    }

  // Make sure we still have an item for the node, which may not be the case
  // if Initialize() happened to be called while the note edit dialog was open.
  if (QTreeWidgetItem* item = this->FindItem(this->invisibleRootItem(), node))
    {
    node->SetNote(s.isEmpty() ? 0 : qPrintable(s));
    this->UpdateItem(item);
    emit this->ItemsChanged();
    emit this->NoteChanged(node->GetInstanceId(), s);
    }
}

//-----------------------------------------------------------------------------
void vqTreeView::UpdateActivated(vtkVgVideoNode& node)
{
  vtkVgNodeBase* nodeBase = static_cast<vtkVgNodeBase*>(&node);

  if (this->LastActivated != nodeBase)
    {
    this->LastActivated = nodeBase;
    }
}

//-----------------------------------------------------------------------------
void vqTreeView::Reset()
{
  this->LastActivated = 0;
  this->clear();
}
