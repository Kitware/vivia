// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgChartTimeline.h"

#include "vtkVgPlotTimeline.h"

#include "vtkAxis.h"
#include "vtkCommand.h"
#include "vtkContext2D.h"
#include "vtkContextScene.h"
#include "vtkContextMouseEvent.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkPen.h"
#include "vtkPlot.h"
#include "vtkPlotGrid.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkTextProperty.h"
#include "vtkTooltipItem.h"
#include "vtkTransform2D.h"
#include "vtkVector.h"

#include <sstream>
#include "boost/date_time/posix_time/posix_time.hpp"

#define TIMELINE_MIN_RANGE 1.0e7  // 10 s
#define TIMELINE_MAX_RANGE 1.0e15 // ~30 yrs

namespace
{

boost::gregorian::date GetEpoch()
{
  return boost::gregorian::date(1970, 1, 1);
}

boost::posix_time::ptime GetPosixTimeSeconds(double us)
{
  boost::posix_time::ptime pt(GetEpoch());
  return pt + boost::posix_time::seconds(static_cast<int64_t>(us * 1.0e-6));
}

boost::posix_time::ptime GetPosixTime(double us)
{
  long long micros = static_cast<long long>(us);
  return GetPosixTimeSeconds(us) +
         boost::posix_time::microseconds(micros % 1000000);
}

double GetInputTime(boost::posix_time::ptime pt)
{
  return (pt - boost::posix_time::ptime(GetEpoch())).total_microseconds();
}

enum { NumFixedTickMarks = 5 };

enum TickInterval
{
  TI_Day,
  TI_Month,
  TI_Year
};

} // end anonymous namespace

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkVgChartTimeline);

//-----------------------------------------------------------------------------
vtkVgChartTimeline::vtkVgChartTimeline()
{
  this->GetAxis(vtkAxis::BOTTOM)->SetBehavior(vtkAxis::CUSTOM);
  this->GetAxis(vtkAxis::BOTTOM)->SetRange(0.0, 1.0);

  this->GetAxis(vtkAxis::TOP)->SetBehavior(vtkAxis::CUSTOM);
  this->GetAxis(vtkAxis::TOP)->SetRange(0.0, 1.0);
  this->GetAxis(vtkAxis::TOP)->GetPen()->SetOpacity(0);
  this->GetAxis(vtkAxis::TOP)->SetLabelsVisible(false);

  // The second x axis is a way to apply a different rendering style for the
  // 'major' date tick marks and labels. The render order for these is not
  // quite right, since we are forced to append them at the end of the item
  // list...but it works well enough for now.
  this->xAxis2 = vtkAxis::New();
  this->xAxis2->SetRange(0.0, 1.0);
  this->xAxis2->SetBehavior(vtkAxis::CUSTOM);
  this->xAxis2->GetGridPen()->SetColor(100, 100, 100);
  this->xAxis2->GetLabelProperties()->BoldOn();
  this->xAxis2->SetPosition(vtkAxis::BOTTOM);
  this->xAxis2->SetVisible(true);

  vtkPlotGrid* grid = vtkPlotGrid::New();
  grid->SetXAxis(xAxis2);
  grid->SetYAxis(this->GetAxis(vtkAxis::LEFT));

  this->AddItem(grid);
  this->AddItem(this->xAxis2);

  grid->FastDelete();

  this->MinXLimit = 0.0;          // 1-1-1970
  this->MaxXLimit = 2145916800e6; // 1-1-2038
  this->MinX = MinXLimit;
  this->MaxX = MaxXLimit;
  this->NormalizeInput = false;
  this->ExtentsAreValid = false;
  this->Actions.Zoom() = vtkContextMouseEvent::LEFT_BUTTON;
  this->Actions.Pan() = vtkContextMouseEvent::MIDDLE_BUTTON;
  this->Actions.Select() = vtkContextMouseEvent::RIGHT_BUTTON;
  this->ActionsClick.Select() = vtkContextMouseEvent::LEFT_BUTTON;
}

//-----------------------------------------------------------------------------
vtkVgChartTimeline::~vtkVgChartTimeline()
{
  this->xAxis2->Delete();
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::FixZoomLevel()
{
  // Keep the chart from getting into a "bad state" by limiting the max zoom.
  double range = this->MaxX - this->MinX;
  double mid = 0.5 * (this->MinX + this->MaxX);
  if (range < TIMELINE_MIN_RANGE)
    {
    this->SetXExtents(mid - 0.5 * TIMELINE_MIN_RANGE,
                      mid + 0.5 * TIMELINE_MIN_RANGE);
    }
  else if (range > TIMELINE_MAX_RANGE)
    {
    this->SetXExtents(mid - 0.5 * TIMELINE_MAX_RANGE,
                      mid + 0.5 * TIMELINE_MAX_RANGE);
    }
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::BuildAxis()
{
  this->AxisBuildTime.Modified();

  vtkAxis* xAxis = this->GetAxis(vtkAxis::BOTTOM);
  xAxis->Update();

  // Figure out how many fixed ticks we want to show, based on chart width.
  float width = xAxis->GetPosition2().GetX() - xAxis->GetPosition1().GetX();
  float padding = 50.0f;
  int maxTicks = 1 +
                 vtkContext2D::FloatToInt((width - 2.0f * padding) / 100.0f);

  boost::posix_time::ptime mint = GetPosixTime(this->MinX);
  boost::posix_time::ptime maxt = GetPosixTime(this->MaxX);

  bool showFixedTicks = true;

  // Figure out a good interval for floating ticks.
  TickInterval tickInterval = TI_Day;
  int days = (maxt - mint).hours() / 24;
  int daysPerTick = 1;
  if (days >= 3)
    {
    showFixedTicks = false;
    if (days >= 365 * 3)
      {
      tickInterval = TI_Year;
      daysPerTick = 365;
      }
    else if (days >= 30 * 3)
      {
      tickInterval = TI_Month;
      daysPerTick = 30;
      }
    }

  // Compute how many ticks to skip between the ones that are actually drawn.
  int stride = 0;
  int totalTicks = days / daysPerTick;
  while (totalTicks / ++stride > maxTicks)
    ;

  vtkStringArray* newLabels = vtkStringArray::New();
  vtkDoubleArray* newPositions = vtkDoubleArray::New();

  // Add ticks at fixed intervals displaying time at the level of seconds.
  if (showFixedTicks)
    {
    newLabels->SetNumberOfTuples(maxTicks);
    newPositions->SetNumberOfTuples(maxTicks);

    double axisRange = xAxis->GetMaximum() - xAxis->GetMinimum();
    double axisPadding = (padding / width) * axisRange;

    double x = xAxis->GetMinimum() + axisPadding;
    for (int i = 0; i < maxTicks; ++i)
      {
      double t = this->MinX + x * (this->MaxX - this->MinX);
      std::ostringstream ostr;
      ostr << GetPosixTimeSeconds(t).time_of_day() << 'Z';

      newLabels->SetValue(i, ostr.str());
      newPositions->SetValue(i, x);

      x += (axisRange - 2 * axisPadding) / (maxTicks - 1);
      }
    }

  // Don't attempt to display dates if the chart range is beyond what we are
  // expecting. Doing so has little benefit and causes and seems to cause
  // problems on linux systems.
  if (this->MinXLimit < 0.0 || this->MaxXLimit > 2145916800e6)
    {
    xAxis->SetCustomTickPositions(newPositions, newLabels);

    newPositions->FastDelete();
    newLabels->FastDelete();
    return;
    }

  boost::posix_time::ptime pt = GetPosixTime(this->MinX);

  // Need a reference point to test stride against.
  boost::gregorian::date epoch = GetEpoch();

  // Compute the position of the first floating tick mark by 'rounding up'
  // to the start of the next day, month, or year in the calendar that is
  // an even multiple of the stride.
  double t;
  switch (tickInterval)
    {
    case TI_Day:
      {
      boost::posix_time::ptime tmp = pt + boost::gregorian::days(1);
      tmp -= boost::posix_time::microseconds(1);
      boost::gregorian::date d = tmp.date();
      long days = (d - epoch).days();
      if (long rem = days % stride)
        {
        d += boost::gregorian::days(stride - rem);
        }
      t = GetInputTime(boost::posix_time::ptime(d));
      break;
      }

    case TI_Month:
      {
      boost::posix_time::ptime tmp = pt + boost::gregorian::months(1);
      tmp -= boost::posix_time::microseconds(1);
      boost::gregorian::date d(tmp.date().year(), tmp.date().month(), 1);
      long months = 12 * (d.year() - epoch.year()) + d.month() - epoch.month();
      if (long rem = months % stride)
        {
        d += boost::gregorian::months(stride - rem);
        }
      t = GetInputTime(boost::posix_time::ptime(d));
      break;
      }

    case TI_Year:
      {
      boost::posix_time::ptime tmp = pt + boost::gregorian::years(1);
      tmp -= boost::posix_time::microseconds(1);
      boost::gregorian::date d(tmp.date().year(), 1, 1);
      long years = d.year() - epoch.year();
      if (long rem = years % stride)
        {
        d += boost::gregorian::years(stride - rem);
        }
      t = GetInputTime(boost::posix_time::ptime(d));
      break;
      }
    }

  double minLabelPadding = 65.0f / width;

  vtkStringArray* newFloatingLabels = vtkStringArray::New();
  vtkDoubleArray* newFloatingPositions = vtkDoubleArray::New();

  // Add floating tick marks indicating the start of a day, month, or year.
  while (t <= this->MaxX)
    {
    double val = (t - this->MinX) / (this->MaxX - this->MinX);

    // Don't show the label of fixed ticks that are overlapping with us,
    // but continue to show the tick itself as a reference point.
    if (showFixedTicks)
      {
      for (int i = 0; i < maxTicks; ++i)
        {
        if (fabs(val - newPositions->GetValue(i)) < minLabelPadding)
          {
          newLabels->SetValue(i, "");
          }
        }
      }

    std::ostringstream ostr;
    boost::posix_time::ptime pt = GetPosixTimeSeconds(t);
    double prevt = t;

    // Add a label and compute the position of the next floating tick.
    switch (tickInterval)
      {
      case TI_Day:
        ostr << pt.date();
        t = GetInputTime(pt + boost::gregorian::days(stride));
        break;

      case TI_Month:
        ostr << pt.date().year() << '-' << pt.date().month();
        t = GetInputTime(pt + boost::gregorian::months(stride));
        break;

      case TI_Year:
        ostr << pt.date().year();
        t = GetInputTime(pt + boost::gregorian::years(stride));
        break;
      }

    // Escape hatch in case we overflow.
    if (t <= prevt)
      {
      break;
      }

    newFloatingPositions->InsertNextValue(val);
    newFloatingLabels->InsertNextValue(ostr.str());
    }

  xAxis->SetCustomTickPositions(newPositions, newLabels);
  xAxis->Update();

  xAxis2->SetCustomTickPositions(newFloatingPositions, newFloatingLabels);
  xAxis2->Update();

  newPositions->FastDelete();
  newLabels->FastDelete();
  newFloatingPositions->FastDelete();
  newFloatingLabels->FastDelete();
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::SetCurrentTime(double t)
{
  vtkAxis* const tAxis = this->GetAxis(vtkAxis::TOP);

  vtkDoubleArray* newPositions = vtkDoubleArray::New();
  newPositions->SetNumberOfTuples(1);
  newPositions->InsertNextValue(t);

  tAxis->SetCustomTickPositions(newPositions);
  tAxis->Update();

  newPositions->FastDelete();
}

//-----------------------------------------------------------------------------
vtkAxis* vtkVgChartTimeline::GetAxis(int axisIndex)
{
  switch (axisIndex)
  {
    case BOTTOM_MAJOR:
      return this->xAxis2;
    default:
      return vtkChartXY::GetAxis(axisIndex);
  }
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::Update()
{
  // Set some reasonable extents if this is the first update and no one
  // has otherwise told us what the extents should be.
  if (!this->ExtentsAreValid)
    {
    this->ResetExtents();
    }

  if (this->NormalizeInput)
    {
    if (this->GetMTime() > this->DataBuildTime)
      {
      this->BuildXColumn();
      }
    else
      {
      for (int i = 0, end = this->GetNumberOfPlots(); i < end; ++i)
        {
        if (this->GetPlot(i)->GetInput()->GetMTime() > this->DataBuildTime)
          {
          this->BuildXColumn();
          break;
          }
        }
      }
    }

  this->Superclass::Update();
}

//-----------------------------------------------------------------------------
bool vtkVgChartTimeline::UpdateLayout(vtkContext2D* painter)
{
  bool changed = this->Superclass::UpdateLayout(painter);

  // This duplicates some code in vtkChartXY, which is necessary in order to
  // keep the x axis border from from behaving badly.
  vtkAxis* axis = this->GetAxis(vtkAxis::BOTTOM);
  int border = 0;
  if (axis->GetVisible())
    {
    vtkRectf bounds = axis->GetBoundingRect(painter);
    border = static_cast<int>(bounds.GetHeight());

    // The x axis may not have any ticks, but we still want to keep
    // a consistent border height so that the second x axis (which
    // the base class doesn't know about) doesn't get occluded.
    border = std::max(24, border);
    }
  border += this->GetLegendBorder(painter, vtkAxis::BOTTOM);
  border = border < this->HiddenAxisBorder ? this->HiddenAxisBorder : border;

  // Fix the x axis border that was set by vtkChartXY::UpdateLayout()
  if (changed)
    {
    this->SetBottomBorder(border);

    this->GetAxis(vtkAxis::LEFT)->SetPoint1(this->Point1[0], this->Point1[1]);
    this->GetAxis(vtkAxis::LEFT)->SetPoint2(this->Point1[0], this->Point2[1]);

    this->GetAxis(vtkAxis::BOTTOM)->SetPoint1(this->Point1[0], this->Point1[1]);
    this->GetAxis(vtkAxis::BOTTOM)->SetPoint2(this->Point2[0], this->Point1[1]);

    this->GetAxis(vtkAxis::RIGHT)->SetPoint1(this->Point2[0], this->Point1[1]);
    this->GetAxis(vtkAxis::RIGHT)->SetPoint2(this->Point2[0], this->Point2[1]);

    this->GetAxis(vtkAxis::TOP)->SetPoint1(this->Point1[0], this->Point2[1]);
    this->GetAxis(vtkAxis::TOP)->SetPoint2(this->Point2[0], this->Point2[1]);

    this->xAxis2->SetPoint1(this->Point1[0], this->Point1[1]);
    this->xAxis2->SetPoint2(this->Point2[0], this->Point1[1]);

    for (int i = 0; i < 4; ++i)
      {
      this->GetAxis(i)->Update();
      }
    this->xAxis2->Update();
    }

  // Update the ticks and labels for the x axis. This is not really 'layout',
  // but it needs to happen here since it follows vtkChartXY::UpdateLayout(),
  // but precedes rendering.
  if (this->NormalizeInput &&
      (changed || this->MTime > this->AxisBuildTime))
    {
    this->BuildAxis();
    }

  return changed;
}

//-----------------------------------------------------------------------------
bool vtkVgChartTimeline::MouseMoveEvent(const vtkContextMouseEvent& mouse)
{
  if (mouse.GetButton() == vtkContextMouseEvent::NO_BUTTON)
    {
    this->Scene->SetDirty(true);
    this->Tooltip->SetVisible(this->UpdateTooltip(mouse));
    return true;
    }

  vtkAxis* y = this->GetAxis(vtkAxis::LEFT);
  double min = y->GetMinimum();
  double max = y->GetMaximum();

  bool result = this->Superclass::MouseMoveEvent(mouse);

  // never pan the y axis
  if (mouse.GetButton() == this->Actions.Pan())
    {
    if (this->NormalizeInput)
      {
      this->NormalizeXAxis();
      }
    y->SetMinimum(min);
    y->SetMaximum(max);
    this->RecalculatePlotTransforms();
    }

  return result;
}

//-----------------------------------------------------------------------------
bool vtkVgChartTimeline::MouseDoubleClickEvent(const vtkContextMouseEvent& mouse)
{
  if (mouse.GetButton() == this->ActionsClick.Select())
    {
    this->DoSelect(mouse, true);
    return true;
    }
  return vtkAbstractContextItem::MouseDoubleClickEvent(mouse);
}

//-----------------------------------------------------------------------------
bool vtkVgChartTimeline::MouseButtonReleaseEvent(
  const vtkContextMouseEvent& mouse)
{
  if (mouse.GetButton() == this->ActionsClick.Select())
    {
    // Treat as a single click if the event wasn't a valid drag.
    if (fabs(mouse.GetPos().GetX() - this->MouseBox.GetX()) < 5.0f &&
        fabs(mouse.GetPos().GetY() - this->MouseBox.GetY()) < 5.0f)
      {
      this->DoSelect(mouse, false);
      this->MouseBox.SetWidth(0.0);
      this->MouseBox.SetHeight(0.0);
      this->DrawBox = false;
      return true;
      }
    }

  vtkAxis* y = this->GetAxis(vtkAxis::LEFT);
  double min = y->GetMinimum();
  double max = y->GetMaximum();

  bool result = this->Superclass::MouseButtonReleaseEvent(mouse);

  // force y axis scale to remain constant
  if (mouse.GetButton() == this->Actions.Zoom() &&
      this->Scene->GetDirty())
    {
    if (this->NormalizeInput)
      {
      this->NormalizeXAxis();
      this->FixZoomLevel();
      }
    y->SetMinimum(min);
    y->SetMaximum(max);
    y->RecalculateTickSpacing();
    this->RecalculatePlotTransforms();
    }

  return result;
}

//-----------------------------------------------------------------------------
bool vtkVgChartTimeline::MouseWheelEvent(const vtkContextMouseEvent& event,
                                         int delta)
{
  vtkAxis* y = this->GetAxis(vtkAxis::LEFT);
  double min = y->GetMinimum();
  double max = y->GetMaximum();

  Superclass::MouseWheelEvent(event, delta);

  if (this->NormalizeInput)
    {
    this->NormalizeXAxis();
    this->FixZoomLevel();
    }
  // un-zoom the y axis - we want the y axis scale to remain constant
  y->SetMinimum(min);
  y->SetMaximum(max);
  y->RecalculateTickSpacing();

  this->RecalculatePlotTransforms();

  return true;
}

//-----------------------------------------------------------------------------
bool vtkVgChartTimeline::Hit(const vtkContextMouseEvent& mouse)
{
  // Allow a mouse event to be initiated anywhere within the bounds of the
  // chart, *including* the borders. This allows rubberbands to still work
  // correctly even if the first click is outside of the chart proper.
  if (mouse.GetScenePos().GetX() >= this->Size.GetX() &&
      mouse.GetScenePos().GetX() < this->Size.GetX() + this->Size.GetWidth() &&
      mouse.GetScenePos().GetY() >= this->Size.GetY() &&
      mouse.GetScenePos().GetY() < this->Size.GetY() + this->Size.GetHeight())
    {
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::SetSelection(int p, vtkIdTypeArray* selection)
{
  vtkPlot* plot = this->GetPlot(p);
  if (!plot)
    {
    return;
    }

  vtkTable* input = plot->GetInput();

  // first make sure the selected item is visible
  if (selection->GetNumberOfTuples() != 0)
    {
    this->MakePointVisible(input->GetValue(selection->GetValue(0), 0).ToFloat(),
                           input->GetValue(selection->GetValue(0), 1).ToInt());
    }

  // select the item
  plot->SetSelection(selection);
  this->InvokeEvent(vtkCommand::SelectionChangedEvent);
  this->Scene->SetDirty(true);
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::ClearSelection()
{
  for (int i = 0; i < this->GetNumberOfPlots(); ++i)
    {
    this->GetPlot(i)->SetSelection(0);
    }
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::MakePointVisible(float x, float y)
{
  double min = this->GetAxis(vtkAxis::BOTTOM)->GetMinimum();
  double max = this->GetAxis(vtkAxis::BOTTOM)->GetMaximum();
  double pad = 0.1 * (max - min);

  bool boundsChanged = false;

  if (x < min + pad)
    {
    this->GetAxis(vtkAxis::TOP)->SetRange(x - pad, x - pad + max - min);
    this->GetAxis(vtkAxis::BOTTOM)->SetRange(x - pad, x - pad + max - min);
    boundsChanged = true;
    }
  else if (x > max - pad)
    {
    this->GetAxis(vtkAxis::TOP)->SetRange(x + pad - (max - min), x + pad);
    this->GetAxis(vtkAxis::BOTTOM)->SetRange(x + pad - (max - min), x + pad);
    boundsChanged = true;
    }

  min = this->GetAxis(vtkAxis::LEFT)->GetMinimum();
  max = this->GetAxis(vtkAxis::LEFT)->GetMaximum();
  pad = 0.0; // 0.2 * (max - min);

  if (y < min + pad)
    {
    this->GetAxis(vtkAxis::LEFT)->SetRange(y - pad, y - pad + max - min);
    boundsChanged = true;
    }
  else if (y > max - pad)
    {
    this->GetAxis(vtkAxis::LEFT)->SetRange(y + pad - (max - min), y + pad);
    boundsChanged = true;
    }

  if (boundsChanged)
    {
    this->RecalculateBounds();
    }
}

//-----------------------------------------------------------------------------
bool vtkVgChartTimeline::UpdateTooltip(const vtkContextMouseEvent& mouse)
{
  // iterate through the plots and build a selection
  int plots = this->GetNumberOfPlots();
  if (plots)
    {
    // @TODO: Since vtkChartXY keeps its transforms private, we have to
    // recalculate the transform every time the mouse moves. We need access to
    // the private transforms, or at least know when they have been invalidated.
    vtkTransform2D* transform = vtkTransform2D::New();
    this->CalculatePlotTransform(this->GetAxis(vtkAxis::BOTTOM),
                                 this->GetAxis(vtkAxis::LEFT), transform);

    vtkVector2f position;
    transform->InverseTransformPoints(mouse.GetPos().GetData(),
                                      position.GetData(), 1);

    // use a tolerance of +/- 5 pixels
    vtkVector2f tolerance(5 * (1.0 / transform->GetMatrix()->GetElement(0, 0)),
                          5 * (1.0 / transform->GetMatrix()->GetElement(1, 1)));
    transform->Delete();

    // search for hits
    for (int j = 0; j < plots; ++j)
      {
      if (vtkVgPlotTimeline* plot =
            vtkVgPlotTimeline::SafeDownCast(this->GetPlot(j)))
        {
        vtkIdTypeArray* ids;
        if (plot->GetIsIntervalPlot())
          {
          ids = plot->GetIntervalIdsInArea(position, tolerance);
          }
        else
          {
          ids = plot->GetPointIdsInArea(position, tolerance);
          }

        if (ids && ids->GetNumberOfTuples() > 0)
          {
          std::ostringstream ostr;

          // display the event ids in the tooltip
          int size = ids->GetNumberOfTuples();
          for (int k = 0; k < size; ++k)
            {
            vtkVariant label =
              plot->GetInput()->GetValueByName(ids->GetValue(k), "Label");
            if (label.IsValid())
              {
              ostr << label.ToString();
              }
            else
              {
              ostr << plot->GetInput()->GetValueByName(ids->GetValue(k), "Id");
              }

            // limit the tooltip to a reasonable size
            if (k + 1 == 16)
              {
              if (size > k + 1)
                {
                ostr << "...";
                }
              break;
              }

            if (size > k + 1)
              {
              ostr << ", ";
              }
            }

          this->Tooltip->SetText(ostr.str().c_str());
          this->Tooltip->SetPosition(mouse.GetScreenPos().GetX() + 20,
                                     mouse.GetScreenPos().GetY() - 35);
          return true;
          }
        }
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::DoSelect(const vtkContextMouseEvent& mouse,
                                  bool activate)
{
  vtkPlot* plotChanged = 0;

  // iterate through the plots and build a selection
  int plots = this->GetNumberOfPlots();
  if (plots)
    {
    vtkTransform2D* transform = vtkTransform2D::New();
    this->CalculatePlotTransform(this->GetAxis(vtkAxis::BOTTOM),
                                 this->GetAxis(vtkAxis::LEFT),
                                 transform);

    // check within a 5 pixel radius of the click
    float tol = 5.0f;
    float pos1[] = { mouse.GetPos().GetX() - tol, mouse.GetPos().GetY() - tol };
    float pos2[] = { mouse.GetPos().GetX() + tol, mouse.GetPos().GetY() + tol };
    transform->InverseTransformPoints(pos1, pos1, 1);
    transform->InverseTransformPoints(pos2, pos2, 1);

    vtkVector2f min(pos1);
    vtkVector2f max(pos2);

    // select nearby points in plots
    for (int j = 0; j < plots; ++j)
      {
      if (vtkPlot* plot = this->GetPlot(j))
        {
        vtkVgPlotTimeline* timelinePlot =
          vtkVgPlotTimeline::SafeDownCast(plot);
        if (timelinePlot && timelinePlot->GetIsIntervalPlot())
          {
          // select intervals, not points
          vtkVector2f pos (mouse.GetPos().GetX(), mouse.GetPos().GetY());
          transform->InverseTransformPoints(pos.GetData(), pos.GetData(), 1);
          vtkVector2f tolerance(
            tol * (1.0 / transform->GetMatrix()->GetElement(0, 0)),
            tol * (1.0 / transform->GetMatrix()->GetElement(1, 1)));

          vtkVector2f posv(pos);
          if (timelinePlot->SelectIntervals(posv, tolerance))
            {
            plotChanged = timelinePlot;
            break;
            }
          }
        else
          {
          if (plot->SelectPoints(min, max))
            {
            plotChanged = plot;
            break;
            }
          }
        }
      }
    transform->Delete();
    }

  // NOTE: If plotChanged is still not initialized, then I am not sure if the
  //       code below even should be executed. Leaving it for now for as it is.
  SelectionData sd;
  sd.Plot = plotChanged;
  sd.IsActivation = activate;

  this->InvokeEvent(vtkCommand::SelectionChangedEvent, &sd);
  this->Scene->SetDirty(true);
}

//-----------------------------------------------------------------------------
vgRange<double> vtkVgChartTimeline::GetXExtents() const
{
  return vgRange<double>(this->MinX, this->MaxX);
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::SetXExtents(double min, double max)
{
  this->MinX = std::max(min, this->MinXLimit);
  this->MaxX = std::min(max, this->MaxXLimit);
  this->ExtentsAreValid = true;

  this->GetAxis(vtkAxis::TOP)->SetRange(this->MinX, this->MaxX);

  // Show the date that the chart is centered on.
  double center = 0.5 * (this->MinX + this->MaxX);
  boost::posix_time::ptime pt = GetPosixTime(center);
  this->SetTitle(boost::gregorian::to_simple_string(pt.date()));

  this->Modified();
}

//-----------------------------------------------------------------------------
vgRange<double> vtkVgChartTimeline::GetXExtentLimits() const
{
  return vgRange<double>(this->MinXLimit, this->MaxXLimit);
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::SetXExtentLimits(double min, double max)
{
  this->MinXLimit = min;
  this->MaxXLimit = max;
  this->SetXExtents(this->MinX, this->MaxX);
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::ResetExtents()
{
  double minx = VTK_DOUBLE_MAX;
  double maxx = VTK_DOUBLE_MIN;

  // Compute the extents of the input.
  for (int i = 0, end = this->GetNumberOfPlots(); i < end; ++i)
    {
    vtkTable* table = this->GetPlot(i)->GetInput();
    vtkDoubleArray* xd = vtkDoubleArray::SafeDownCast(
                           table->GetColumnByName("XD"));
    if (!xd)
      {
      return;
      }

    for (int row = 0, end = xd->GetNumberOfTuples(); row < end; ++row)
      {
      double val = xd->GetValue(row);
      if (val < minx)
        {
        minx = val;
        }
      if (val > maxx)
        {
        maxx = val;
        }
      }
    }

  if (minx == maxx)
    {
    minx -= 1e-6;
    maxx += 1e-6;
    }

  double padding = (maxx - minx) * 0.1;
  minx -= padding;
  maxx += padding;

  // Fit the chart to the input extents plus a small amount of padding.
  this->MinXLimit = minx;
  this->MaxXLimit = maxx;
  this->SetXExtents(minx, maxx);
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::BuildXColumn()
{
  // This is where the X column data for the plots are built, based on the
  // full precision input data ("XD"), and the current extents of the chart.
  // The plots can't use the full precision data directly, since the plots'
  // internal transform would cause unacceptable round-off error when
  // the range of the X axis is small (e.g. spanning a few hours or less).
  //
  // In order to mitigate this, we fix the X axis range to [0,1] and normalize
  // the data based on the current extents. This allows us to maintain
  // sub-second precision when zoomed in, while allowing large input time
  // spans (on the order of years).
  for (int i = 0, end = this->GetNumberOfPlots(); i < end; ++i)
    {
    vtkTable* table = this->GetPlot(i)->GetInput();

    vtkFloatArray* x = vtkFloatArray::SafeDownCast(
                         table->GetColumnByName("X"));

    vtkDoubleArray* xd = vtkDoubleArray::SafeDownCast(
                           table->GetColumnByName("XD"));

    assert(x && xd);
    assert(x->GetNumberOfTuples() == xd->GetNumberOfTuples());

    for (int row = 0, erow = xd->GetNumberOfTuples(); row < erow; ++row)
      {
      double v = xd->GetValue(row);
      v = (v - this->MinX) / (this->MaxX - this->MinX);
      x->SetValue(row, v);
      }

    x->Modified();
    table->Modified();
    }

  this->DataBuildTime.Modified();
}

//-----------------------------------------------------------------------------
void vtkVgChartTimeline::NormalizeXAxis()
{
  vtkAxis* x = this->GetAxis(vtkAxis::BOTTOM);
  double xmin = x->GetMinimum();
  double xmax = x->GetMaximum();

  // Compute new extents based on the change to the x-axis range, and then
  // renormalize the axis. The data will be updated elsewhere to ensure
  // everything shows up in the right place.
  double newXMin = this->MinX + xmin * (this->MaxX - this->MinX);
  double newXMax = this->MinX + xmax * (this->MaxX - this->MinX);

  if (newXMin < this->MinXLimit)
    {
    double delta = this->MinXLimit - newXMin;
    this->SetXExtents(this->MinXLimit, newXMax + delta);
    }
  else if (newXMax > this->MaxXLimit)
    {
    double delta = newXMax - this->MaxXLimit;
    this->SetXExtents(newXMin - delta, this->MaxXLimit);
    }
  else
    {
    this->SetXExtents(newXMin, newXMax);
    }

  x->SetRange(0.0, 1.0);
}
