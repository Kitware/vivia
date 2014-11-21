/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqTimeline.h"

#include <vtkVgChartTimeline.h>
#include <vtkVgGroupNode.h>
#include <vtkVgNodeVisitor.h>
#include <vtkVgPlotTimeline.h>
#include <vtkVgSceneManager.h>
#include <vtkVgVideoModel0.h>
#include <vtkVgVideoNode.h>
#include <vtkVgVideoRepresentationBase0.h>

#include <vtkVgQtUtil.h>

#include <vgUnixTime.h>

#include <QVTKWidget.h>

#include <vtkAxis.h>
#include <vtkChartXY.h>
#include <vtkColorSeries.h>
#include <vtkContextMapper2D.h>
#include <vtkContextScene.h>
#include <vtkContextView.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkInteractorObserver.h>
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkPlot.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTable.h>
#include <vtkTextProperty.h>
#include <vtkVariantArray.h>

#include <QTime>

#include <algorithm>
#include <vector>

namespace TimelineInternal
{

enum TableColumns
{
  TC_X = 0,
  TC_Y,
  TC_XD,
  TC_Id,
  TC_Object,
  TC_Color
};

struct NodeInfo
{
  vtkVgVideoNode* Node;
  double StartTime;
  double EndTime;
  bool IsGroundTruth;

  bool operator<(const NodeInfo& other) const
    {
    return this->StartTime < other.StartTime;
    }
};

typedef std::vector<NodeInfo> NodeVector;

//-----------------------------------------------------------------------------
struct SceneVisitor : public vtkVgNodeVisitor
{
  SceneVisitor()
    : minTime(-1), maxTime(-1) {}

  void Reset()
    {
    this->VideoNodeInfos.clear();
    this->minTime = this->maxTime = -1;
    }

  using vtkVgNodeVisitor::Visit;

  virtual void Visit(vtkVgVideoNode* node)
    {
    NodeInfo NI;
    NI.Node = node;
    NI.StartTime = this->Timeline->GetUnixEpochTime(node->GetTimeRange()[0]);
    NI.EndTime = this->Timeline->GetUnixEpochTime(node->GetTimeRange()[1]);
    NI.IsGroundTruth = !(node->GetIsNormalResult() ||
                         node->GetIsRefinementResult());

    // keep running min of start times
    if (this->minTime < 0.0f)
      {
      this->minTime = NI.StartTime;
      }
    else
      {
      if (NI.StartTime < this->minTime)
        {
        this->minTime = NI.StartTime;
        }
      }

    // keep running max of end times
    if (this->maxTime < 0.0f)
      {
      this->maxTime = NI.EndTime;
      }
    else
      {
      if (NI.EndTime > this->maxTime)
        {
        this->maxTime = NI.EndTime;
        }
      }

    this->VideoNodeInfos.push_back(NI);
    }

  vqTimeline* Timeline;

  QList<vtkVgVideoNode*> VideoNodes;
  NodeVector VideoNodeInfos;

  QList<vtkVgNodeBase*> Selected;

  double minTime, maxTime;
};

//-----------------------------------------------------------------------------
template <typename T>
void addTableColumn(vtkTable* table, const char* label)
{
  vtkSmartPointer<T> column = vtkSmartPointer<T>::New();
  column->SetName(label);

  table->AddColumn(column);
}

} // namespace TimelineInternal

using namespace TimelineInternal;

//-----------------------------------------------------------------------------
struct vqTimeline::SelectionGuard
{
  vqTimeline* Widget;

  SelectionGuard(vqTimeline* d)
    {
    this->Widget = d;
    this->Widget->IsSelecting = true;
    }

  ~SelectionGuard()
    {
    this->Widget->IsSelecting = false;
    }
};

//-----------------------------------------------------------------------------
class vqTimeline::TimelineSelectionCallback : public vtkCommand
{
  vqTimeline*  Parent;

  vtkPlot*     LastPlot;
  vtkIdType    LastSelectedIndex;
  vtkIdType    LastSelectionSize;
  vtkIdType    LastSelectionFirstVal;
  vtkIdType    LastSelectionLastVal;

  TimelineSelectionCallback()
    : Parent(0),
      LastPlot(0),
      LastSelectedIndex(-1),
      LastSelectionSize(0),
      LastSelectionFirstVal(-1),
      LastSelectionLastVal(-1)
    {}

public:
  static TimelineSelectionCallback* New()
    {
    return new TimelineSelectionCallback;
    }

  void SetParent(vqTimeline* parent)
    {
    this->Parent = parent;
    }

  virtual void Execute(vtkObject* vtkNotUsed(caller),
                       unsigned long vtkNotUsed(eventId), void* callData)
    {
    if (this->Parent->IsSelecting)
      {
      return;
      }

    if (callData)
      {
      vtkVgChartTimeline::SelectionData* sd =
        static_cast<vtkVgChartTimeline::SelectionData*>(callData);

      vtkPlot* plot = sd->Plot;
      if (!plot)
        {
        return;
        }

      // single-click selection
      vtkIdTypeArray* selection = plot->GetSelection();
      vtkIdType size = selection->GetNumberOfTuples();

      if (size == 0)
        {
        emit this->Parent->SelectedNodes(QList<vtkVgNodeBase*>());
        return;
        }

      vtkIdType index;
      vtkIdType firstVal = selection->GetValue(0);
      vtkIdType lastVal = selection->GetValue(size - 1);

      // Try to determine if this is the same as the last selection. If so,
      // cycle through the items in the group. This allows us to cycle through
      // overlapping points when the user clicks the same cluster multiple times.
      if (this->LastPlot == plot &&
          this->LastSelectionSize == size &&
          this->LastSelectionFirstVal == firstVal &&
          this->LastSelectionLastVal == lastVal)
        {
        index = (this->LastSelectedIndex + 1) % size;
        }
      else
        {
        // just pick the first item if this is a different selection group
        index = 0;
        }

      vtkIdType selectedVal = selection->GetValue(index);

      // remember selection group info for next time
      this->LastPlot = plot;
      this->LastSelectedIndex = index;
      this->LastSelectionSize = size;
      this->LastSelectionFirstVal = firstVal;
      this->LastSelectionLastVal = lastVal;

      // modify the selection
      selection->SetNumberOfTuples(0);
      selection->InsertNextValue(selectedVal);

      if (sd->IsActivation)
        {
        vtkObjectBase* obj =
          plot->GetInput()->GetValue(selectedVal, TC_Object).ToVTKObject();
        vtkVgNodeBase* node = vtkVgNodeBase::SafeDownCast(obj);
        emit this->Parent->ActivatedNode(*node);
        }
      else
        {
        std::vector<int> rows(1);
        rows[0] = selectedVal;
        this->SelectionChangeNotify(plot, rows);
        }
      }
    else
      {
      // rubberband selection
      this->LastPlot = 0;

      vtkPlotLine* linePlot =
        vtkPlotLine::SafeDownCast(this->Parent->TimelineChart->GetPlot(0));

      vtkIdTypeArray* selection = linePlot->GetSelection();
      vtkIdType size = selection->GetNumberOfTuples();

      std::vector<int> rows(size);

      for (vtkIdType i = 0; i != size; ++i)
        {
        rows[i] = selection->GetValue(i);
        }

      this->SelectionChangeNotify(linePlot, rows);
      }
    }

  void SelectionChangeNotify(vtkPlot* plot, const std::vector<int>& rows)
    {
    SelectionGuard sg(this->Parent);
    vtkTable* table = plot->GetInput();

    QList<vtkVgNodeBase*> nodes;
    nodes.reserve(static_cast<int>(rows.size()));

    for (size_t i = 0, end = rows.size(); i != end; ++i)
      {
      vtkObjectBase* obj = table->GetValue(rows[i], TC_Object).ToVTKObject();
      vtkVgNodeBase* node = vtkVgNodeBase::SafeDownCast(obj);
      nodes << node;
      }

    emit this->Parent->SelectedNodes(nodes);
    }
};

//-----------------------------------------------------------------------------
vqTimeline::vqTimeline(QWidget* parent) :
  QVTKWidget(parent), SceneManager(0), LookupTable(0)
{
  this->Visitor = new SceneVisitor();
  this->Visitor->Timeline = this;

  this->IsSelecting = false;

  // Set up the context view
  this->TimelineView = vtkSmartPointer<vtkContextView>::New();
  this->TimelineView->SetInteractor(this->GetInteractor());
  this->TimelineView->GetRenderer()->SetBackground(1.0, 1.0, 1.0);
  this->SetRenderWindow(this->TimelineView->GetRenderWindow());

  vtkConnect(this->TimelineView->GetInteractor()->GetInteractorStyle(),
             vtkCommand::KeyPressEvent, this, SLOT(OnKeyPressed()));

  this->TimelineChart = vtkSmartPointer<vtkVgChartTimeline>::New();
  this->TimelineChart->SetAutoAxes(false);
  this->TimelineChart->SetShowLegend(false);
  this->TimelineChart->SetNormalizeInput(true);
  this->TimelineChart->SetHiddenAxisBorder(15);

  vtkAxis* const yAxisL = this->TimelineChart->GetAxis(vtkAxis::LEFT);
  vtkAxis* const yAxisR = this->TimelineChart->GetAxis(vtkAxis::RIGHT);
  vtkAxis* const xAxis = this->TimelineChart->GetAxis(vtkAxis::BOTTOM);

  xAxis->SetTitle(vtkStdString());

  yAxisL->SetTitle(vtkStdString());
  yAxisL->SetBehavior(vtkAxis::CUSTOM);
  yAxisL->SetNumberOfTicks(0);

  yAxisR->SetTitle(vtkStdString());
  yAxisR->SetBehavior(vtkAxis::CUSTOM);
  yAxisR->SetNumberOfTicks(0);
  yAxisR->SetVisible(true);

  this->TimelineView->GetScene()->AddItem(this->TimelineChart);

  // Build timeline table
  this->TimelineTable = vtkSmartPointer<vtkTable>::New();
  addTableColumn<vtkFloatArray>  (this->TimelineTable.GetPointer(), "X");
  addTableColumn<vtkFloatArray>  (this->TimelineTable.GetPointer(), "Y");
  addTableColumn<vtkDoubleArray> (this->TimelineTable.GetPointer(), "XD");
  addTableColumn<vtkIntArray>    (this->TimelineTable.GetPointer(), "Id");
  addTableColumn<vtkVariantArray>(this->TimelineTable.GetPointer(), "Object");
  addTableColumn<vtkFloatArray>  (this->TimelineTable.GetPointer(), "Colors");

  // Register selection callback
  TimelineSelectionCallback* cb = TimelineSelectionCallback::New();
  cb->SetParent(this);
  this->TimelineChart->AddObserver(vtkCommand::SelectionChangedEvent, cb);
  cb->Delete();
}

//-----------------------------------------------------------------------------
vqTimeline::~vqTimeline()
{
  delete this->Visitor;
  this->Visitor = 0;
}

//-----------------------------------------------------------------------------
void vqTimeline::Initialize(QList<vtkVgVideoNode*> nodes)
{
  this->Visitor->VideoNodes = nodes;

  if (this->isVisible())
    {
    this->UpdateTimelineChart();
    this->ResetView();
    }
}

//-----------------------------------------------------------------------------
void vqTimeline::SelectNodes(QList<vtkVgNodeBase*> nodes)
{
  // avoid recursion
  if (this->IsSelecting)
    {
    return;
    }

  this->Visitor->Selected = nodes;
  this->UpdateSelection();
  this->TimelineView->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void vqTimeline::UpdateColors()
{
  if (!this->TimelineChart)
    {
    return;
    }

  // update color scalars in the table
  int row = 0;
  for (NodeVector::iterator itr = this->Visitor->VideoNodeInfos.begin(),
       end = this->Visitor->VideoNodeInfos.end();
       itr != end; ++itr, ++row)
    {
    double color = itr->Node->GetColorScalar();
    this->TimelineTable->SetValue(row, TC_Color, color);
    this->TimelineTable->SetValue(++row, TC_Color, color);
    }

  this->TimelineTable->Modified();
  this->TimelineView->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void vqTimeline::Update()
{
  if (!this->TimelineChart)
    {
    return;
    }

  // Rebuild the table
  this->UpdateTimelineTable();
  this->UpdateSelection();

  this->TimelineTable->Modified();
  this->TimelineView->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void vqTimeline::OnFrameChanged(int frame)
{
  this->TimelineChart->GetAxis(1)->SetRange(frame - 1, frame + 50);
  this->TimelineChart->RecalculateBounds();
  this->TimelineView->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
int vqTimeline::FindFreeY(const TimelineInternal::NodeInfo& info,
                          std::vector<std::pair<bool, double> >& used,
                          double padding)
{
  for (size_t i = 0, end = used.size(); i < end; ++i)
    {
    if (used[i].first && (used[i].second + padding) < info.StartTime)
      {
      used[i].second = info.EndTime;
      return static_cast<int>(i);
      }
    else if (!used[i].first)
      {
      used[i].first = true;
      used[i].second = info.EndTime;
      return static_cast<int>(i);
      }
    }

  // should never get here
  return -1;
}

//-----------------------------------------------------------------------------
int vqTimeline::SetRows(int row,
                        const TimelineInternal::NodeInfo& info,
                        std::vector<std::pair<bool, double> >& used,
                        double padding)
{
  // find the next available y position that we can place this interval
  // ground truth nodes are allowed to overlap with normal result nodes
  int i = this->FindFreeY(info, used, padding);
  if (i == -1)
    {
    std::cerr << "No available y?\n";
    return row;
    }

  double x1 = info.StartTime;
  double x2 = info.EndTime;

  // alternate y at integral values above and below y = 0
  int y = (i % 2 == 0) ? -(i + 1) / 2 : (i + 1) / 2;

  this->MinX = std::min(this->MinX, x1);
  this->MaxX = std::max(this->MaxX, x2);

  this->MinY = std::min(this->MinY, y);
  this->MaxY = std::max(this->MaxY, y);

  // don't add intervals for invisible nodes (only use them for layout purposes)
  if (info.Node->GetVisibleNodeMask() != vtkVgNodeBase::VISIBLE_NODE_MASK)
    {
    return row;
    }

  // set row data for interval start
  vtkIdType id = info.Node->GetInstanceId();
  double color = info.Node->GetColorScalar();
  this->TimelineTable->SetValue(row, TC_X, 0); // filled by chart
  this->TimelineTable->SetValue(row, TC_Y, y);
  this->TimelineTable->SetValue(row, TC_XD, x1);
  this->TimelineTable->SetValue(row, TC_Id, id);
  this->TimelineTable->SetValue(row, TC_Object, info.Node);
  this->TimelineTable->SetValue(row++, TC_Color, color);

  // set row data for interval end
  this->TimelineTable->SetValue(row, TC_X, 0); // filled by chart
  this->TimelineTable->SetValue(row, TC_Y, y);
  this->TimelineTable->SetValue(row, TC_XD, x2);
  this->TimelineTable->SetValue(row, TC_Id, id);
  this->TimelineTable->SetValue(row, TC_Object, info.Node);
  this->TimelineTable->SetValue(row++, TC_Color, color);
  return row;
}

//-----------------------------------------------------------------------------
void vqTimeline::UpdateTimelineTable()
{
  // traverse the node list and collect video nodes
  int visibleCount = 0;
  this->Visitor->Reset();
  foreach (vtkVgVideoNode* node, this->Visitor->VideoNodes)
    {
    this->Visitor->Visit(node);
    if (node->GetVisibleNodeMask() == vtkVgNodeBase::VISIBLE_NODE_MASK)
      {
      ++visibleCount;
      }
    }

  // sort all video nodes in the graph by the start time of their sources
  std::sort(this->Visitor->VideoNodeInfos.begin(),
            this->Visitor->VideoNodeInfos.end());

  int numNodes = static_cast<int>(this->Visitor->VideoNodeInfos.size());

  std::vector<std::pair<bool, double> > used(numNodes);
  std::vector<std::pair<bool, double> > usedGT(numNodes);

  this->TimelineTable->SetNumberOfRows(2 * visibleCount);

  this->MinX = VTK_DOUBLE_MAX;
  this->MaxX = VTK_DOUBLE_MIN;
  this->MinY = 0;
  this->MaxY = 0;

  double timeRange = this->Visitor->maxTime - this->Visitor->minTime;
  double xPadding = timeRange * 0.005;

  // If the range of data is large (spanning multiple days), our computed
  // padding may cause the graph to vertically spread too far at higher zoom
  // levels. In that case, clamp the maximum padding to improve the layout at
  // the "interesting" zoom levels (on the order of seconds, the length of a
  // typical event). This may cause results to overlap when viewing time spans
  // of several days, but it seems to be an acceptable tradeoff.
  if (xPadding > 3.0e6)
    {
    xPadding = 3.0e6;
    }

  // Points are added to the table in two passes: first we add all
  // non-ground truth items, then all ground truth items. This ensures that
  // the (translucent) ground truth timeline intervals will blend correctly.

  // set rows for non-ground truth items
  int row = 0;
  for (NodeVector::iterator itr = this->Visitor->VideoNodeInfos.begin(),
       end = this->Visitor->VideoNodeInfos.end();
       itr != end; ++itr)
    {
    if (!itr->IsGroundTruth)
      {
      row = this->SetRows(row, *itr, used, xPadding);
      }
    }

  // set rows for ground truth items
  for (NodeVector::iterator itr = this->Visitor->VideoNodeInfos.begin(),
       end = this->Visitor->VideoNodeInfos.end();
       itr != end; ++itr)
    {
    if (itr->IsGroundTruth)
      {
      row = this->SetRows(row, *itr, usedGT, xPadding);
      }
    }
}

//-----------------------------------------------------------------------------
void vqTimeline::UpdateSelection()
{
  SelectionGuard sg(this);
  this->TimelineChart->ClearSelection();

  vtkIdTypeArray* selection = vtkIdTypeArray::New();
  selection->SetNumberOfTuples(0);

  vtkTable* table = this->TimelineTable.GetPointer();
  vtkIdType numRows = table->GetNumberOfRows();

  foreach (vtkVgNodeBase* node, this->Visitor->Selected)
    {
    // don't try to select things that aren't being shown
    if (node->GetVisibleNodeMask() != vtkVgNodeBase::VISIBLE_NODE_MASK)
      {
      continue;
      }

    // find the index of the item within the table
    for (vtkIdType i = 0; i < numRows; ++i)
      {
      vtkObjectBase* obj = table->GetValue(i, TC_Object).ToVTKObject();
      if (vtkVgNodeBase::SafeDownCast(obj) == node)
        {
        selection->InsertNextValue(i);
        break;
        }
      }
    }

  this->TimelineChart->SetSelection(0, selection);
  selection->FastDelete();
}

//-----------------------------------------------------------------------------
void vqTimeline::AddTimelinePlots()
{
  int numRows = this->TimelineTable->GetNumberOfRows();
  if (numRows > 0)
    {
    vtkVgPlotTimeline* plot = vtkVgPlotTimeline::New();
    plot->SetInputData(this->TimelineTable, 0, 1);
    plot->SetIsIntervalPlot(true);
    plot->SetScalarVisibility(1);
    plot->SelectColorArray("Colors");
    plot->SetLookupTable(this->LookupTable);

    this->TimelineChart->AddPlot(plot);
    plot->FastDelete();
    }
}

//-----------------------------------------------------------------------------
void vqTimeline::UpdateTimelineChart()
{
  this->UpdateTimelineTable();
  this->AddTimelinePlots();

  vtkAxis* const yAxis = this->TimelineChart->GetAxis(vtkAxis::LEFT);
  yAxis->SetRange(this->MinY - 0.5, this->MaxY + 0.5);
}

//-----------------------------------------------------------------------------
void vqTimeline::showEvent(QShowEvent* /*event*/)
{
  this->UpdateTimelineChart();
}

//-----------------------------------------------------------------------------
double vqTimeline::GetUnixEpochTime(double useconds)
{
  return vgUnixTime(useconds).toInt64();
}

//-----------------------------------------------------------------------------
void vqTimeline::OnKeyPressed()
{
  switch (this->TimelineView->GetInteractor()->GetKeyCode())
    {
    case 'r':
    case 'R':
      this->ResetView();
      break;
    }
}

//-----------------------------------------------------------------------------
void vqTimeline::ResetView()
{
  if (this->TimelineChart)
    {
    vtkAxis* y = this->TimelineChart->GetAxis(vtkAxis::LEFT);
    y->SetRange(this->MinY - 0.5, this->MaxY + 0.5);
    this->TimelineChart->ResetExtents();
    this->TimelineChart->RecalculateBounds();
    }
}
