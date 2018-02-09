/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpSessionView.h"

#include "vpProjectList.h"

#include <QSplitter>
#include <QStackedWidget>
#include <QTabWidget>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------
vpSessionView::vpSessionView(QWidget* p)
  : QWidget(p)
{
  this->Splitter = new QSplitter;
  this->ListWidget = new vpProjectList;
  this->StackedWidget = new QStackedWidget;
  this->Splitter->setObjectName("projectSplitter");
  this->ListWidget->setObjectName("projectList");
  this->StackedWidget->setObjectName("projectPanels");
  this->StackedWidget->addWidget(new vpObjectSelectionPanel);
  this->Splitter->addWidget(this->ListWidget);
  this->Splitter->addWidget(this->StackedWidget);

  this->ListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->ListWidget->setTextElideMode(Qt::ElideMiddle);

  QList<int> sizes;
  sizes << 0 << 1;
  this->Splitter->setSizes(sizes);
  this->Splitter->setCollapsible(1, false);

  connect(this->ListWidget,
          SIGNAL(itemChanged(QListWidgetItem*)),
          SLOT(ReactToItemChanged(QListWidgetItem*)));

  connect(this->ListWidget,
          SIGNAL(currentRowChanged(int)),
          SLOT(SetCurrentSession(int)));

  connect(this->ListWidget,
          SIGNAL(closeProjectRequested(int)),
          SIGNAL(CloseProjectRequested(int)));

  connect(this->StackedWidget,
          SIGNAL(currentChanged(int)),
          SIGNAL(SessionChanged(int)));

  QVBoxLayout* layout = new QVBoxLayout;
  layout->setMargin(5);
  layout->addWidget(this->Splitter);
  this->setLayout(layout);
}

//-----------------------------------------------------------------------------
vpSessionView::~vpSessionView()
{
}

//-----------------------------------------------------------------------------
void vpSessionView::AddSession(vpViewCore* viewCore,
                               vtkVgActivityManager* activityManager,
                               vtkVgEventModel* eventModel,
                               vtkVpTrackModel* trackModel,
                               vtkVgEventFilter* eventFilter,
                               vtkVgTrackFilter* trackFilter,
                               vtkVgEventTypeRegistry* eventTypes,
                               vtkVgTrackTypeRegistry* trackTypes,
                               const QString& title)
{
  vpObjectSelectionPanel* osp = new vpObjectSelectionPanel;
  osp->Initialize(viewCore, activityManager, eventModel, trackModel,
                  eventFilter, trackFilter, eventTypes, trackTypes,
                  this->GetSessionCount(), title);

  // Add a tab for the new session.
  this->StackedWidget->addWidget(osp);

  // remove the dummy panel if this is the first project loaded
  if (this->GetSessionCount() == 0)
    {
    QWidget* dummyPanel = this->StackedWidget->widget(0);
    this->StackedWidget->removeWidget(dummyPanel);
    delete dummyPanel;
    }

  // add the list item
  QListWidgetItem* item =
    new QListWidgetItem(title.isEmpty() ? QString::number(osp->GetSessionId())
                                        : title);
  item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
  this->ListWidget->addItem(item);
  item->setCheckState(Qt::Checked);

  // force the project list to be shown if more than one is loaded
  if (this->GetSessionCount() > 1)
    {
    QList<int> sizes = this->Splitter->sizes();
    if (sizes[0] == 0)
      {
      sizes[0] = 1;
      this->Splitter->setSizes(sizes);
      }
    this->Splitter->setCollapsible(0, false);
    }

  connect(osp, SIGNAL(SelectionChanged(int)), SIGNAL(SelectionChanged(int)));
  connect(osp, SIGNAL(HoverItemChanged(int)), SIGNAL(HoverItemChanged(int)));
  connect(osp, SIGNAL(ItemsChanged()), SIGNAL(ItemsChanged()));

  connect(osp, SIGNAL(CreateEvent(int, vtkIdList*, int)),
          SIGNAL(CreateEvent(int, vtkIdList*, int)));
  connect(osp, SIGNAL(DeleteEvent(int, int)),
          SIGNAL(DeleteEvent(int, int)));

  connect(osp, SIGNAL(EditTrack(int, int)),
          SIGNAL(EditTrack(int, int)));
  connect(osp, SIGNAL(StopEditingTrack(int)),
          SIGNAL(StopEditingTrack(int)));
  connect(osp, SIGNAL(DeleteTrack(int, int)),
          SIGNAL(DeleteTrack(int, int)));
  connect(osp, SIGNAL(SplitTrack(int, int)),
          SIGNAL(SplitTrack(int, int)));
  connect(osp, SIGNAL(ImproveTrack(int, int)),
          SIGNAL(ImproveTrack(int, int)));

  connect(osp, SIGNAL(AddEventsToGraphModel(QList<int>, int)),
          SIGNAL(AddEventsToGraphModel(QList<int>, int)));
  connect(osp, SIGNAL(AddTrackEventsToGraphModel(int, int)),
          SIGNAL(AddTrackEventsToGraphModel(int, int)));

  connect(osp, SIGNAL(ShowStatusMessage(const QString&, int, int)),
          SIGNAL(ShowStatusMessage(const QString&, int, int)));

  connect(osp, SIGNAL(DisplayActivities(bool)),
          SIGNAL(DisplayActivities(bool)));
  connect(osp, SIGNAL(DisplayEvents(bool)),
          SIGNAL(DisplayEvents(bool)));
  connect(osp, SIGNAL(DisplayTracks(bool)),
          SIGNAL(DisplayTracks(bool)));
  connect(osp, SIGNAL(DisplayTrackHeads(bool)),
          SIGNAL(DisplayTrackHeads(bool)));
  connect(osp, SIGNAL(DisplaySceneElements(bool)),
          SIGNAL(DisplaySceneElements(bool)));
}

//-----------------------------------------------------------------------------
void vpSessionView::RemoveSession(int sessionId)
{
  Q_ASSERT(sessionId < this->GetSessionCount());

  vpObjectSelectionPanel* osp =
    static_cast<vpObjectSelectionPanel*>(this->StackedWidget->widget(sessionId));

  this->StackedWidget->removeWidget(this->StackedWidget->widget(sessionId));
  delete osp;

  delete this->ListWidget->takeItem(sessionId);

  // Fix up the ids to match the tab indices
  for (int i = 0, end = this->StackedWidget->count(); i < end; ++i)
    {
    static_cast<vpObjectSelectionPanel*>(
      this->StackedWidget->widget(i))->SetSessionId(i);
    }

  if (this->GetSessionCount() == 1)
    {
    this->Splitter->setCollapsible(0, true);
    }
}

//-----------------------------------------------------------------------------
int vpSessionView::GetSessionCount()
{
  return this->ListWidget->count();
}

//-----------------------------------------------------------------------------
void vpSessionView::SetCurrentSession(int sessionId)
{
  this->ListWidget->setCurrentRow(sessionId);
  this->StackedWidget->setCurrentIndex(sessionId);
}

//-----------------------------------------------------------------------------
void vpSessionView::SetSessionVisible(int sessionId, bool visible)
{
  this->ListWidget->item(sessionId)->setCheckState(
    visible ? Qt::Checked : Qt::Unchecked);
}

//-----------------------------------------------------------------------------
int vpSessionView::GetCurrentSession()
{
  return this->StackedWidget->currentIndex();
}

//-----------------------------------------------------------------------------
void vpSessionView::Update(bool rebuild)
{
  for (int i = 0, end = this->StackedWidget->count(); i < end; ++i)
    {
    static_cast<vpObjectSelectionPanel*>(
      this->StackedWidget->widget(i))->Update(rebuild);
    }
}

//-----------------------------------------------------------------------------
bool vpSessionView::SelectItem(int type, int id)
{
  return this->CurrentTab()->SelectItem(type, id);
}

//-----------------------------------------------------------------------------
void vpSessionView::AddAndSelectItem(int type, int id)
{
  this->CurrentTab()->AddAndSelectItem(type, id);
}

//-----------------------------------------------------------------------------
void vpSessionView::FocusItem()
{
  this->CurrentTab()->FocusItem();
}

//-----------------------------------------------------------------------------
void vpSessionView::ReactToItemChanged(QListWidgetItem* item)
{
  for (int i = 0, end = this->ListWidget->count(); i < end; ++i)
    {
    if (this->ListWidget->item(i) == item)
      {
      emit this->SessionVisibilityChanged(i, item->checkState() == Qt::Checked);
      break;
      }
    }
}

//-----------------------------------------------------------------------------
bool vpSessionView::GetSelectedItemInfo(vgItemInfo& info)
{
  return this->CurrentTab()->GetSelectedItemInfo(info);
}

//-----------------------------------------------------------------------------
bool vpSessionView::GetHoveredItemInfo(vgItemInfo& info)
{
  return this->CurrentTab()->GetHoveredItemInfo(info);
}

//-----------------------------------------------------------------------------
QList<vtkIdType> vpSessionView::GetSelectedItems(int type)
{
  return this->CurrentTab()->GetSelectedItems(type);
}

//-----------------------------------------------------------------------------
void vpSessionView::SetCurrentTab(int type)
{
  return this->CurrentTab()->SetCurrentTab(type);
}

//-----------------------------------------------------------------------------
vpObjectSelectionPanel* vpSessionView::CurrentTab()
{
  return static_cast<vpObjectSelectionPanel*>(
    this->StackedWidget->currentWidget());
}
