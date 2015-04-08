/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsTimelineViewer.h"

#include "vsTimelineSelectionCallback.h"

#include <vsCore.h>
#include <vsScene.h>

#include <vsDisplayInfo.h>

#include <vtkVgChartTimeline.h>
#include <vtkVgEvent.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVgPlotTimeline.h>
#include <vtkVgTimeStamp.h>
#include <vtkVgTrack.h>

#include <vgCheckArg.h>
#include <vgUtil.h>

#include <qtColorUtil.h>
#include <qtScopedValueChange.h>
#include <qtStlUtil.h>

#include <vtkAxis.h>
#include <vtkContextScene.h>
#include <vtkContextView.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkPen.h>
#include <vtkRenderer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkTextProperty.h>

namespace // anonymous
{

typedef vsDisplayInfo (vsScene::*GetInfoMethod)(vtkIdType);

typedef vgRange<double> Interval;
typedef QList<Interval> IntervalList;

enum TableColumn
{
  TC_X = 0,
  TC_Y,
  TC_Time,
  TC_Id,
  TC_Label
};

enum EntityType
{
  TRACK_TYPE,
  EVENT_TYPE,
  INVALID_TYPE,
};

//-----------------------------------------------------------------------------
struct TimelineEntityReference
{
  TimelineEntityReference(vtkIdType id = -1, int type = INVALID_TYPE) :
    Id(id), Type(type) {}

  vtkIdType Id;
  int Type;

  operator bool() const
    { return this->Id != -1 && this->Type != INVALID_TYPE; }

  bool operator!() const { bool b = *this; return !b; }
};

//-----------------------------------------------------------------------------
struct TimelineEntityInfo : TimelineEntityReference
{
  TimelineEntityInfo() {}
  TimelineEntityInfo(vtkVgTrack*);
  TimelineEntityInfo(vtkVgEvent*);

  vtkStdString Name;

  double StartTime;
  double EndTime;
  double VirtualEndTime;

  float Width;
  int Classification;

  vtkIdType Plot;
  vtkIdType TableRow;
  vtkSmartPointer<vtkTable> Table;

  int Y; // Index of Y container (IntervalList) to which this entity belongs
  int YX; // Index of this entity's Interval in its IntervalList
};

typedef QHash<vtkIdType, TimelineEntityInfo> EntityInfoMap;

//-----------------------------------------------------------------------------
TimelineEntityInfo::TimelineEntityInfo(vtkVgTrack* track) :
  TimelineEntityReference(track->GetId(), TRACK_TYPE),
  StartTime(track->GetStartFrame().GetTime()),
  EndTime(track->GetEndFrame().GetTime()),
  Width(0.5f)
{
  char* const name = track->GetName();
  this->Name =
    (name ? vtkStdString(name) : stdString(QString::number(track->GetId())));
}

//-----------------------------------------------------------------------------
TimelineEntityInfo::TimelineEntityInfo(vtkVgEvent* event) :
  TimelineEntityReference(event->GetId(), EVENT_TYPE),
  Name(qPrintable(QString::number(event->GetId()))),
  StartTime(event->GetStartFrame().GetTime()),
  EndTime(event->GetEndFrame().GetTime()),
  Width(0.8f)
{
}

} // namespace <anonymous>

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsTimelineViewerPrivate

//-----------------------------------------------------------------------------
class vsTimelineViewerPrivate
{
public:
  vsTimelineViewerPrivate(vsTimelineViewer* q, vsCore* core, vsScene* scene);

  void addNewEntity(
    EntityInfoMap&, const TimelineEntityInfo&, const vsDisplayInfo&);
  void updateEntity(
    EntityInfoMap&, const TimelineEntityInfo&, const vsDisplayInfo&);

  void updateEntity(TimelineEntityInfo&, const vsDisplayInfo&, vtkPlot* = 0);
  void updateEntities(EntityInfoMap&, GetInfoMethod);

  void addEntityToRow(TimelineEntityInfo& info);
  void updateEntityYPosition(const TimelineEntityInfo& info);
  void updateRowInterval(const TimelineEntityInfo& info);

  void recomputeLayout();

  void updateYScale();
  void setYScale(EntityInfoMap& entities, double scale);

  vtkSmartPointer<vtkTable> createTable();

  vgRange<double> getXExtents() const;
  void updateChart();

  static TimelineEntityReference getEntityReference(
    vtkPlot* plot, vtkIdType row);

protected:
  QTE_DECLARE_PUBLIC_PTR(vsTimelineViewer)

  bool ChartDirty;
  bool SizeDirty;
  bool ScaleDirty;
  bool LayoutDirty;
  bool RenderPending;

  bool UpdatingSelection;

  // Artificially increase displayed duration to at least this value
  double MinimumSize;

  double MinimumXPadding;
  double MaximumYScale;

  int MinY, MaxY;
  double YScale;

  QList<IntervalList> Rows;
  double LastStartTime;

  vtkSmartPointer<vtkContextView> Viewer;
  vtkSmartPointer<vtkVgChartTimeline> Chart;

  EntityInfoMap Tracks;
  EntityInfoMap Events;

  QSet<vtkIdType> SelectedTrackIds;
  QSet<vtkIdType> SelectedEventIds;

  vsCore* const Core;
  vsScene* const Scene;

private:
  QTE_DECLARE_PUBLIC(vsTimelineViewer)
};

QTE_IMPLEMENT_D_FUNC(vsTimelineViewer)

//-----------------------------------------------------------------------------
vsTimelineViewerPrivate::vsTimelineViewerPrivate(
  vsTimelineViewer* q, vsCore* core, vsScene* scene) :
  q_ptr(q),
  Core(core),
  Scene(scene)
{
  this->ChartDirty = false;
  this->SizeDirty = true;
  this->ScaleDirty = false;
  this->LayoutDirty = false;
  this->RenderPending = false;
  this->UpdatingSelection = false;

  this->MinimumSize = 1e6;
  this->MinimumXPadding = 1e6;
  this->MaximumYScale = 15.0;

  this->LastStartTime = -std::numeric_limits<double>::max();

  this->MinY = 0;
  this->MaxY = -1; // Ensure first addition will set ScaleDirty

  this->YScale = 10.0; // Meaningless, but will recompute on first paint anyway

  this->Viewer = vtkSmartPointer<vtkContextView>::New();
  this->Viewer->SetInteractor(q->GetInteractor());
  q->SetRenderWindow(this->Viewer->GetRenderWindow());

  const QPalette& palette = q->palette();
  vgColor baseColor(palette.color(QPalette::Base));
  vgColor textColor(palette.color(QPalette::Text));
  vgColor gridColor(qtColorUtil::blend(baseColor.toQColor(),
                                       textColor.toQColor(), 0.4));

  this->Viewer->GetRenderer()->SetBackground(baseColor.data().array);

  this->Chart = vtkSmartPointer<vtkVgChartTimeline>::New();
  this->Chart->SetAutoAxes(false);
  this->Chart->SetShowLegend(false);
  this->Chart->SetNormalizeInput(true);
  this->Chart->SetHiddenAxisBorder(15);
  this->Chart->GetTitleProperties()->SetColor(textColor.data().array);

  // Register selection callback
  vtkNew<vsTimelineSelectionCallback> scb;
  scb->SetParent(q);
  this->Chart->AddObserver(vtkCommand::SelectionChangedEvent,
                           scb.GetPointer());

  vtkAxis* yAxis = this->Chart->GetAxis(vtkAxis::LEFT);
  vtkAxis* xAxisMinor = this->Chart->GetAxis(vtkAxis::BOTTOM);
  vtkAxis* xAxisMajor = this->Chart->GetAxis(vtkVgChartTimeline::BOTTOM_MAJOR);
  vtkAxis* tAxis = this->Chart->GetAxis(vtkAxis::TOP);

  xAxisMinor->SetTitle("time");
  xAxisMinor->GetTitleProperties()->SetColor(textColor.data().array);
  xAxisMinor->GetLabelProperties()->SetColor(textColor.data().array);

  yAxis->SetTitle("");
  yAxis->SetNumberOfTicks(0);
  yAxis->SetBehavior(vtkAxis::CUSTOM);
  yAxis->SetRange(-0.5, 0.5);
  yAxis->SetNumberOfTicks(0);
  yAxis->SetVisible(false);

  xAxisMinor->GetPen()->SetColorF(textColor.data().array);
  xAxisMinor->GetGridPen()->SetColorF(gridColor.data().array);
  xAxisMajor->GetPen()->SetColorF(textColor.data().array);
  xAxisMajor->GetGridPen()->SetColorF(gridColor.data().array);

  vgColor timeColor(qtColorUtil::blend(baseColor.toQColor(),
                                       this->Scene->selectionColor(), 0.5));
  tAxis->GetGridPen()->SetColorF(timeColor.data().array);
  tAxis->SetVisible(true);

  // TODO Add callback here
  this->Viewer->GetScene()->AddItem(this->Chart);
}

//-----------------------------------------------------------------------------
void vsTimelineViewerPrivate::addNewEntity(
  EntityInfoMap& map, const TimelineEntityInfo& tei, const vsDisplayInfo& di)
{
  // Compute display end time and insert into map
  TimelineEntityInfo& entry = map.insert(tei.Id, tei).value();
  entry.VirtualEndTime =
    qMax(entry.EndTime, entry.StartTime + this->MinimumSize);

  // Create timeline plot for the entity
  entry.Table = this->createTable();
  vtkNew<vtkVgPlotTimeline> plot;
  plot->SetProperty("type", entry.Type);
  plot->SetIsIntervalPlot(true);
  plot->SetInputData(entry.Table, TC_X, TC_Y);
  plot->SetWidth(entry.Width * this->YScale);
  plot->Update();
  entry.Plot = this->Chart->AddPlot(plot.GetPointer());
  this->updateEntity(entry, di, plot.GetPointer());

  // Create data table entries for entity
  vtkIdType row = entry.TableRow = entry.Table->GetNumberOfRows();
  entry.Table->InsertNextBlankRow();
  entry.Table->SetValue(row, TC_X, 0);
  entry.Table->SetValue(row, TC_Time, entry.StartTime);
  entry.Table->SetValue(row, TC_Id, entry.Id);
  entry.Table->SetValue(row, TC_Label, entry.Name);

  ++row;
  entry.Table->InsertNextBlankRow();
  entry.Table->SetValue(row, TC_X, 0);
  entry.Table->SetValue(row, TC_Time, entry.VirtualEndTime);
  entry.Table->SetValue(row, TC_Id, entry.Id);
  entry.Table->SetValue(row, TC_Label, entry.Name);

  // Add entity to the next available row
  this->addEntityToRow(entry);
  this->updateEntityYPosition(entry); // Also marks table as modified
}

//-----------------------------------------------------------------------------
void vsTimelineViewerPrivate::updateEntity(
  EntityInfoMap& map, const TimelineEntityInfo& tei, const vsDisplayInfo& di)
{
  // Get existing entry
  Q_ASSERT(map.contains(tei.Id));
  TimelineEntityInfo& entry = map[tei.Id];

  // Update entity time, if changed (assuming 'virtual' end only changes if
  // 'real' end also changes)
  if (entry.StartTime != tei.StartTime || entry.EndTime != tei.EndTime)
    {
    // Update start and end time
    entry.StartTime = tei.StartTime;
    entry.EndTime = tei.EndTime;
    entry.VirtualEndTime = qMax(tei.EndTime, tei.StartTime + this->MinimumSize);
    this->updateRowInterval(entry);

    entry.Table->SetValue(entry.TableRow + 0, TC_Time, entry.StartTime);
    entry.Table->SetValue(entry.TableRow + 1, TC_Time, entry.VirtualEndTime);

    // NOTE: This may not be necessary?
    entry.Table->Modified();
    }

  if (entry.Classification != di.BestClassification)
    {
    this->updateEntity(entry, di);
    }
}

//-----------------------------------------------------------------------------
void vsTimelineViewerPrivate::updateEntity(
  TimelineEntityInfo& tei, const vsDisplayInfo& di, vtkPlot* plot)
{
  if (!plot)
    {
    // Get plot from index if not provided by caller
    plot = this->Chart->GetPlot(tei.Plot);
    }

  // Set current classification and get corresponding color
  tei.Classification = di.BestClassification;
  vgColor color = di.Color;

  // Set alpha based on visibility
  if (di.DisplayState)
    {
    // If not hidden by user, if visible => 1.0, else => 0.2
    color.data().color.alpha = (di.Visible ? 1.0 : 0.2);
    }
  else
    {
    // If hidden by user, if it has a classification => 0.5, else => 0.1
    color.data().color.alpha = (di.BestClassification != -1 ? 0.5 : 0.1);
    }

  // Update color; vtkPlot only accepts colors as uchar*4 or double*3, so we
  // can't pass the color by components (which would allow us to also omit the
  // temporary vgColor to set alpha), but must convert to uchar... fortunately,
  // vgColor::fillArray can do that for us
  unsigned char ca[4];
  color.fillArray(ca);
  plot->SetColor(ca[0], ca[1], ca[2], ca[3]);
}

//-----------------------------------------------------------------------------
void vsTimelineViewerPrivate::updateEntities(
  EntityInfoMap& map, GetInfoMethod getInfo)
{
  QTE_Q(vsTimelineViewer);

  foreach_iter (EntityInfoMap::iterator, iter, map)
    {
    this->updateEntity(iter.value(), (this->Scene->*getInfo)(iter.key()));
    }

  q->update();
}

//-----------------------------------------------------------------------------
void vsTimelineViewerPrivate::addEntityToRow(TimelineEntityInfo& info)
{
  if (this->LayoutDirty || info.StartTime < this->LastStartTime)
    {
    // For any entity seen "out of order" (or if the layout is dirty), there is
    // no point computing a position now, so just punt (marking the layout as
    // dirty if it isn't already) and let things get sorted out at the next
    // update when we recompute the whole layout anyway
    info.Y = info.YX = 0;
    this->LayoutDirty = true;
    return;
    }
  this->LastStartTime = info.StartTime;

  const Interval interval(info.StartTime, info.VirtualEndTime);
  const double padding = this->MinimumXPadding;

  // Find the best row into which to insert the entity (no-op if no rows exist)
  int candidate = 0;
  const int k = this->Rows.count();
  while (candidate < k)
    {
    const double available = this->Rows[candidate].last().upper + padding;
    if (available < info.StartTime)
      {
      // This one will do
      break;
      }
    ++candidate;
    }

  // Did we find a spot in an existing row?
  if (candidate < k)
    {
    // Yes; add the entity there
    IntervalList& row = this->Rows[candidate];
    info.Y = candidate;
    info.YX = row.count();
    row.append(interval);
    }
  else
    {
    // No; make a new row
    IntervalList row;
    row.append(interval);
    this->Rows.append(row);
    info.Y = k;
    info.YX = 0;
    }
}

//-----------------------------------------------------------------------------
void vsTimelineViewerPrivate::updateEntityYPosition(
  const TimelineEntityInfo& info)
{
  // Alternate y at integral values above and below y = 0
  const int y = ((info.Y % 2) ? (info.Y + 1) : -(info.Y + 1)) / 2;

  info.Table->SetValue(info.TableRow + 0, TC_Y, y);
  info.Table->SetValue(info.TableRow + 1, TC_Y, y);

  // Update Y extents of chart
  if (y < this->MinY)
    {
    this->MinY = y;
    this->ScaleDirty = true;
    }
  else if (y > this->MaxY)
    {
    this->MaxY = y;
    this->ScaleDirty = true;
    }

  info.Table->Modified();
}

//-----------------------------------------------------------------------------
void vsTimelineViewerPrivate::updateRowInterval(const TimelineEntityInfo& info)
{
  // Do nothing if layout is already dirty, as interval indices may be invalid,
  // and we're due to relayout anyway
  CHECK_ARG(!this->LayoutDirty);

  // Get row and interval for entity
  IntervalList& row = this->Rows[info.Y];
  Interval& interval = row[info.YX];

  // Test if lower boundary overlaps previous entity in row (if any)
  if (interval.lower > info.StartTime && info.YX > 0)
    {
    const double available = row[info.YX - 1].upper + this->MinimumXPadding;
    if (info.StartTime < available)
      {
      this->LayoutDirty = true;
      return;
      }
    }

  // Test if upper boundary overlaps next entity in row (if any)
  if (interval.upper < info.VirtualEndTime && info.YX < row.count() - 1)
    {
    const double available = row[info.YX + 1].lower - this->MinimumXPadding;
    if (info.VirtualEndTime > available)
      {
      this->LayoutDirty = true;
      return;
      }
    }

  // Updated entity still fits; update its interval
  interval.lower = info.StartTime;
  interval.upper = info.VirtualEndTime;
}

//-----------------------------------------------------------------------------
void vsTimelineViewerPrivate::recomputeLayout()
{
  // Clear existing rows
  this->Rows.clear();
  this->LayoutDirty = false;
  this->LastStartTime = -std::numeric_limits<double>::max();

  // Build time-sorted list of all entities
  QMultiMap<double, TimelineEntityInfo*> entities;
  foreach_iter (EntityInfoMap::iterator, tIter, this->Tracks)
    {
    TimelineEntityInfo& tInfo = tIter.value();
    entities.insert(tInfo.StartTime, &tInfo);
    }
  foreach_iter (EntityInfoMap::iterator, eIter, this->Events)
    {
    TimelineEntityInfo& eInfo = eIter.value();
    entities.insert(eInfo.StartTime, &eInfo);
    }

  // Re-add each entity
  foreach (TimelineEntityInfo* info, entities)
    {
    this->addEntityToRow(*info);
    this->updateEntityYPosition(*info);
    }
}

//-----------------------------------------------------------------------------
void vsTimelineViewerPrivate::updateYScale()
{
  // Get points defining Y axis, and Y axis height
  vtkAxis* yAxis = this->Chart->GetAxis(vtkAxis::LEFT);
  vtkVector2f pt1 = yAxis->GetPosition1();
  vtkVector2f pt2 = yAxis->GetPosition2();
  const double ph = pt2.GetY() - pt1.GetY();

  // Check that the chart height is non-zero (it might not be if it the initial
  // render hasn't happened yet)
  if (ph > 0.0)
    {
    const double lh = 1.5 + this->MaxY - this->MinY;
    const double s = qMin(this->MaximumYScale, ph / lh);

    // Set Y scale and mark as not-dirty
    setYScale(this->Tracks, s);
    setYScale(this->Events, s);

    this->YScale = s;
    this->ScaleDirty = false;
    }
  else
    {
    // If the chart has no height, we can't compute the Y scale, so mark it as
    // needed to be computed (it should be already, but paranoia won't hurt)
    this->ScaleDirty = true;
    }
}

//-----------------------------------------------------------------------------
void vsTimelineViewerPrivate::setYScale(EntityInfoMap& entities, double scale)
{
  foreach (const TimelineEntityInfo tei, entities)
    {
    vtkPlot* const plot = this->Chart->GetPlot(tei.Plot);
    plot->SetWidth(tei.Width * scale);
    }
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkTable> vsTimelineViewerPrivate::createTable()
{
  // Create containers
  vtkNew<vtkFloatArray> xColumn;
  xColumn->SetName("X");
  vtkNew<vtkIntArray> yColumn;
  yColumn->SetName("Y");
  vtkNew<vtkDoubleArray> xdColumn;
  xdColumn->SetName("XD");
  vtkNew<vtkIdTypeArray> idColumn;
  idColumn->SetName("Id");
  vtkNew<vtkStringArray> labelColumn;
  labelColumn->SetName("Label");

  vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();
  table->AddColumn(xColumn.GetPointer());
  table->AddColumn(yColumn.GetPointer());
  table->AddColumn(xdColumn.GetPointer());
  table->AddColumn(idColumn.GetPointer());
  table->AddColumn(labelColumn.GetPointer());

  return table;
}

//-----------------------------------------------------------------------------
vgRange<double> vsTimelineViewerPrivate::getXExtents() const
{
  CHECK_ARG(!this->Rows.empty(), vgRange<double>(0.0, 0.0));

  const double maxDouble = std::numeric_limits<double>::max();
  vgRange<double> x(maxDouble, -maxDouble);

  foreach (const IntervalList& row, this->Rows)
    {
    if (!row.isEmpty())
      {
      vgExpandLowerBoundary(x.lower, row.first().lower);
      vgExpandUpperBoundary(x.upper, row.last().upper);
      }
    }

  return x;
}

//-----------------------------------------------------------------------------
void vsTimelineViewerPrivate::updateChart()
{
  if (this->Chart)
    {
    // Only reset view if we are fully zoomed out
    const bool resetView =
      (this->Chart->GetXExtents() == this->Chart->GetXExtentLimits());

    const double yPad = 1.0 - (0.5 / qMax(1, this->MaxY - this->MinY));
    vtkAxis* const yAxis = this->Chart->GetAxis(vtkAxis::LEFT);
    yAxis->SetRange(this->MinY - yPad, this->MaxY + 0.5);

    // Recompute X extents... this is a little less efficient than if we
    // maintained them continuously, but that is hard to do in the face of
    // "shrinking" entities, and at least our row bookkeeping makes it much
    // faster than if we had to iterate over every item
    vgRange<double> xRange = this->getXExtents();
    const double xPad = qMax(1e6, (xRange.upper - xRange.lower) * 1e-2);
    xRange.lower -= xPad;
    xRange.upper += xPad;

    this->Chart->SetXExtentLimits(xRange);
    if (resetView)
      {
      this->Chart->SetXExtents(xRange);
      }

    this->Chart->RecalculateBounds();
    this->Chart->Update();
    }
}

//-----------------------------------------------------------------------------
TimelineEntityReference vsTimelineViewerPrivate::getEntityReference(
  vtkPlot* plot, vtkIdType row)
{
  CHECK_ARG(plot, TimelineEntityReference());
  CHECK_ARG(row >= 0, TimelineEntityReference());

  const vtkVariant plotType = plot->GetProperty("type");
  const int type = (plotType.IsValid() ? plotType.ToInt() : INVALID_TYPE);

  vtkTable* table = plot->GetInput();
  const vtkVariant idData = table->GetValue(row, TC_Id);
  const vtkIdType id =
    (idData.IsValid() ? static_cast<vtkIdType>(idData.ToLongLong()) : -1);

  return TimelineEntityReference(id, type);
}

//END vsTimelineViewerPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsTimelineViewer

//-----------------------------------------------------------------------------
vsTimelineViewer::vsTimelineViewer(
  vsCore* core, vsScene* scene, QWidget* parent, Qt::WindowFlags flag) :
  QVTKWidget(parent, flag),
  d_ptr(new vsTimelineViewerPrivate(this, core, scene))
{
  // TODO Fix this
}

//-----------------------------------------------------------------------------
vsTimelineViewer::~vsTimelineViewer()
{
}

//-----------------------------------------------------------------------------
void vsTimelineViewer::setSelectedEntity(vtkPlot* plot, vtkIdType row)
{
  QTE_D(vsTimelineViewer);
  CHECK_ARG(!d->UpdatingSelection);

  // We don't want to use the chart's selection (we show selection by changing
  // the color instead), so make it empty again
  d->Chart->ClearSelection();

  if (!this->GetInteractor()->GetControlKey())
    {
    // Single click when not multi-selecting clears selection
    d->SelectedTrackIds.clear();
    d->SelectedEventIds.clear();
    }

  const TimelineEntityReference ref = d->getEntityReference(plot, row);
  if (ref)
    {
    // Get appropriate selection set
    QSet<vtkIdType>& ids =
      (ref.Type == EVENT_TYPE ? d->SelectedEventIds : d->SelectedTrackIds);

    // Toggle selection; okay to do this regardless since the selection set was
    // made empty if we are not multi-selecting, thus this will always choose
    // the insert branch
    if (ids.contains(ref.Id))
      {
      ids.remove(ref.Id);
      }
    else
      {
      ids.insert(ref.Id);
      }
    }

  qtScopedValueChange<bool> guard(d->UpdatingSelection, true);
  emit this->trackSelectionChanged(d->SelectedTrackIds);
  emit this->eventSelectionChanged(d->SelectedEventIds);
}

//-----------------------------------------------------------------------------
void vsTimelineViewer::activateEntity(vtkPlot* plot, vtkIdType row)
{
  QTE_D(vsTimelineViewer);

  TimelineEntityReference ref = d->getEntityReference(plot, row);
  CHECK_ARG(ref);

  if (ref.Type == EVENT_TYPE)
    {
    emit this->eventPicked(ref.Id);
    }
  else
    {
    emit this->trackPicked(ref.Id);
    }
}

//-----------------------------------------------------------------------------
void vsTimelineViewer::update()
{
  QTE_D(vsTimelineViewer);

  if (!d->RenderPending)
    {
    // Render at the end of the event loop, which is when everything should
    // be done updating and we will be ready to render the scene
    d->RenderPending = true;
    QMetaObject::invokeMethod(this, "render", Qt::QueuedConnection);
    }
}

//-----------------------------------------------------------------------------
void vsTimelineViewer::render()
{
  QTE_D(vsTimelineViewer);

  if (d->LayoutDirty)
    {
    d->recomputeLayout();
    d->ChartDirty = true;
    }

  if (d->ChartDirty)
    {
    d->updateChart();
    d->ChartDirty = false;
    }

  // Update scale if it has changed due to the axis range changing; if the size
  // has changed, we must do a render first in order to know the new size :-(
  if (d->ScaleDirty && !d->SizeDirty)
    {
    d->updateYScale();
    }

  d->RenderPending = false;
  d->Viewer->Render();
  d->SizeDirty = false;

  // If scale is still dirty, size was also dirty; now that we know the new
  // size we can update the scale
  if (d->ScaleDirty)
    {
    d->updateYScale();
    d->Viewer->Render();
    }
}

//-----------------------------------------------------------------------------
void vsTimelineViewer::resizeEvent(QResizeEvent* e)
{
  QTE_D(vsTimelineViewer);

  d->SizeDirty = true;
  d->ScaleDirty = true;
  this->update();

  QVTKWidget::resizeEvent(e);
}

//-----------------------------------------------------------------------------
void vsTimelineViewer::addTrack(vtkVgTrack* track)
{
  CHECK_ARG(track);
  CHECK_ARG(track->IsStarted());

  QTE_D(vsTimelineViewer);

  const TimelineEntityInfo tei(track);
  const vsDisplayInfo& di = d->Scene->trackDisplayInfo(track->GetId());
  if (d->Tracks.contains(tei.Id))
    {
    d->updateEntity(d->Tracks, tei, di);
    }
  else
    {
    d->addNewEntity(d->Tracks, tei, di);
    }

  d->ChartDirty = true;
  this->update();
}

//-----------------------------------------------------------------------------
void vsTimelineViewer::addEvent(vtkVgEvent* event)
{
  CHECK_ARG(event);

  QTE_D(vsTimelineViewer);

  const TimelineEntityInfo tei(event);
  const vsDisplayInfo& di = d->Scene->eventDisplayInfo(event->GetId());
  if (d->Events.contains(tei.Id))
    {
    d->updateEntity(d->Events, tei, di);
    }
  else
    {
    d->addNewEntity(d->Events, tei, di);
    }

  d->ChartDirty = true;
  this->update();
}

//-----------------------------------------------------------------------------
void vsTimelineViewer::updateTracks()
{
  QTE_D(vsTimelineViewer);
  d->updateEntities(d->Tracks, &vsScene::trackDisplayInfo);
}

//-----------------------------------------------------------------------------
void vsTimelineViewer::updateEvents()
{
  QTE_D(vsTimelineViewer);
  d->updateEntities(d->Events, &vsScene::eventDisplayInfo);
}

//-----------------------------------------------------------------------------
void vsTimelineViewer::setSelectedTracks(QSet<vtkIdType> trackIds)
{
  QTE_D(vsTimelineViewer);
  d->SelectedTrackIds = trackIds;
}

//-----------------------------------------------------------------------------
void vsTimelineViewer::setSelectedEvents(QSet<vtkIdType> eventIds)
{
  QTE_D(vsTimelineViewer);
  d->SelectedEventIds = eventIds;
}

//-----------------------------------------------------------------------------
void vsTimelineViewer::updateTimeFromMetadata(
  const vtkVgVideoFrameMetaData& md)
{
  QTE_D(vsTimelineViewer);
  d->Chart->SetCurrentTime(md.Time.GetTime());
  this->update();
}

//-----------------------------------------------------------------------------
QSize vsTimelineViewer::minimumSizeHint() const
{
  return QSize(150, 150);
}

//END vsTimelineViewer
