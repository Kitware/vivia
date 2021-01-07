// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "ui_vpTimelineDialog.h"

#include "vpTimelineDialog.h"

#include "vgEventType.h"

#include "vpViewCore.h"

#include "vtkVgChartTimeline.h"
#include "vtkVgEvent.h"
#include "vtkVgEventModel.h"
#include "vtkVgEventTypeRegistry.h"
#include "vtkVgPlotTimeline.h"
#include "vtkVgTypeDefs.h"

#include "vtkAxis.h"
#include "vtkChartXY.h"
#include "vtkColorSeries.h"
#include "vtkContextMapper2D.h"
#include "vtkContextScene.h"
#include "vtkContextView.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkPlot.h"
#include "vtkPlotBar.h"
#include "vtkPlotLine.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkTable.h"
#include "vtkTextProperty.h"
#include "vtkVariantArray.h"

namespace
{
enum ChartType
{
  BarGraph = vtkChart::BAR,
  LineGraph = vtkChart::LINE,
  StackedBarGraph,
};
}

// selection callback class for the timeline chart
class vpTimelineDialog::TimelineSelectionCallback : public vtkCommand
{
  vpTimelineDialog* Parent;

  vtkPlot*     LastPlot;
  vtkIdType    LastSelectedIndex;
  vtkIdType    LastSelectionSize;
  vtkIdType    LastSelectionFirstVal;
  vtkIdType    LastSelectionLastVal;

  TimelineSelectionCallback()
    : Parent(0), LastPlot(0), LastSelectedIndex(-1), LastSelectionSize(0),
      LastSelectionFirstVal(-1), LastSelectionLastVal(-1) { }

public:
  static TimelineSelectionCallback* New() { return new TimelineSelectionCallback; }

  void SetParent(vpTimelineDialog* parent) { this->Parent = parent; }

  virtual void Execute(vtkObject*, unsigned long, void* callData)
    {
    if (!callData)
      {
      return;
      }

    vtkVgChartTimeline::SelectionData* sd =
      static_cast<vtkVgChartTimeline::SelectionData*>(callData);

    vtkPlot* plot = sd->Plot;
    if (!plot)
      {
      this->LastPlot = 0;
      return;
      }

    vtkIdTypeArray* selection = plot->GetSelection();
    vtkIdType size = selection->GetNumberOfTuples();

    if (size == 0)
      {
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

    this->Parent->OnTimelineSelectionChanged(plot, selectedVal);
    }
};

//-----------------------------------------------------------------------------
vpTimelineDialog::vpTimelineDialog(QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags),  ViewCoreInstance(0), EventModel(0),
    TimelineView(0), HistogramView(0), NumEventBins(20), MaxHistogramX(0.0),
    TimelineTables(0)
{
  this->Ui = new Ui::vpTimelineDialog;
  this->Ui->setupUi(this);

  this->IsSelecting = false;

  connect(this->Ui->tabWidget, SIGNAL(currentChanged(int)),
          this, SLOT(OnPageChanged()));

  this->Ui->plotTypeCombo->addItem("Bar Plot", BarGraph);
  this->Ui->plotTypeCombo->addItem("Line Plot", LineGraph);
  this->Ui->plotTypeCombo->addItem("Stacked Bars", StackedBarGraph);

  connect(this->Ui->plotTypeCombo, SIGNAL(currentIndexChanged(int)),
          this, SLOT(OnPlotTypeChanged(int)));

  this->Ui->binSlider->setMinimum(1);
  this->Ui->binSlider->setMaximum(100);
  this->Ui->binSlider->setValue(this->NumEventBins);

  this->Ui->binSpinBox->setMinimum(this->Ui->binSlider->minimum());
  this->Ui->binSpinBox->setMaximum(this->Ui->binSlider->maximum());
  this->Ui->binSpinBox->setValue(this->Ui->binSlider->value());

  connect(this->Ui->binSlider, SIGNAL(valueChanged(int)),
          this->Ui->binSpinBox, SLOT(setValue(int)));

  connect(this->Ui->binSpinBox, SIGNAL(valueChanged(int)),
          this->Ui->binSlider, SLOT(setValue(int)));

  connect(this->Ui->binSpinBox, SIGNAL(valueChanged(int)),
          this, SLOT(OnBinCountChanged(int)));

  this->resize(800, 400);

  QPoint p = parent->pos();
  this->LastWindowPosition = p += QPoint(parent->width() / 2, parent->height() / 4);
}

//-----------------------------------------------------------------------------
vpTimelineDialog::~vpTimelineDialog()
{
  delete[] this->TimelineTables;

  if (this->TimelineView)
    {
    this->TimelineView->Delete();
    }

  if (this->HistogramView)
    {
    this->HistogramView->Delete();
    }

  delete this->Ui;
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::SelectEvent(int id)
{
  // avoid recursion
  if (this->IsSelecting)
    {
    return;
    }

  int type = this->EventModel->GetEvent(id)->GetActiveClassifierType();
  vtkTable* table =
    this->TimelineTables[this->EventTypeRegistry->GetTypeIndex(type)];

  // find the index of the event within the table
  vtkIdType numRows = table->GetNumberOfRows();
  for (vtkIdType i = 0; i < numRows; ++i)
    {
    if (table->GetValue(i, 2).ToInt() == id)
      {
      // figure out which plot is associated with this table
      for (int j = 0; j < this->TimelineChart->GetNumberOfPlots(); ++j)
        {
        if (this->TimelineChart->GetPlot(j)->GetInput() == table)
          {
          // select the item in the plot
          vtkIdTypeArray* selection = vtkIdTypeArray::New();
          selection->SetNumberOfTuples(0);
          selection->InsertNextValue(i);
          this->TimelineChart->ClearSelection();
          this->TimelineChart->SetSelection(j, selection);
          selection->Delete();

          this->TimelineView->GetRenderWindow()->Render();
          return;
          }
        }
      std::cerr << "Plot for event selection not found.\n";
      return;
      }
    }
  std::cerr << "Event selection not found in table.\n";
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::OnPageChanged()
{
  if (this->Ui->tabWidget->currentWidget() == this->Ui->TimelineTab)
    {
    this->BuildTimelineChart();
    }
  else
    {
    this->BuildHistogramChart();
    }
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::OnFrameChanged(int frame)
{
  this->TimelineChart->GetAxis(1)->SetRange(frame - 1, frame + 50);
  this->TimelineChart->RecalculateBounds();
  this->TimelineView->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
int vpTimelineDialog::GetCurrentPlotType()
{
  int index = this->Ui->plotTypeCombo->currentIndex();
  return this->Ui->plotTypeCombo->itemData(index).toInt();
}

//-----------------------------------------------------------------------------
struct vpTimelineDialog::SelectionGuard
{
  vpTimelineDialog* Dialog;

  SelectionGuard(vpTimelineDialog* d)
    {
    this->Dialog = d;
    this->Dialog->IsSelecting = true;
    }

  ~SelectionGuard()
    {
    this->Dialog->IsSelecting = false;
    }
};

//-----------------------------------------------------------------------------
void vpTimelineDialog::OnTimelineSelectionChanged(vtkPlot* plot, vtkIdType row)
{
  SelectionGuard sg(this);
  vtkTable* table = plot->GetInput();
  emit this->SelectedObject(vgObjectTypeDefinitions::Event,
                            table->GetValue(row, 2).ToInt());
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::OnPlotTypeChanged(int /*index*/)
{
  this->HistogramChart->ClearPlots();
  this->AddHistogramPlots(this->GetCurrentPlotType());
  this->HistogramView->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::OnBinCountChanged(int count)
{
  int divided = count / 26;

  // halve the effective bin count every time lg(count) goes up by 1 - this
  // ensures that the number of x axis tick marks is bounded
  int timesToHalve = divided > 0 ? 1 : 0;
  while (divided >>= 1) ++timesToHalve;

  // force the number of bins to be an even multiple of our divisor so that
  // we can be sure tick marks will align correctly with bin boundaries
  count &= ~0 << timesToHalve;

  int prevNumBins = this->NumEventBins;
  this->NumEventBins = count;

  this->UpdateHistogramTable();

  // show ticks at boundary between bins (or every 2^n bin if there are many)
  this->UpdateHistogramAxis((count >> timesToHalve) + 1);

  // it is possible for the underlying plot type to change when we have
  // a line graph with only one bin, so rebuild the plots in this case
  if (this->GetCurrentPlotType() == LineGraph && (prevNumBins > 1) != (count > 1))
    {
    this->HistogramChart->ClearPlots();
    this->AddHistogramPlots(LineGraph);
    this->HistogramView->GetRenderWindow()->Render();
    return;
    }

  this->HistogramChart->RecalculateBounds();
  this->HistogramView->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::BuildTimelineTables()
{
  int numFrames = this->ViewCoreInstance->getNumberOfFrames();
  int numEventTypes = this->EventTypeRegistry->GetNumberOfTypes();
  this->TimelineTables = new vtkSmartPointer<vtkTable>[numEventTypes];

  // add columns to tables
  for (int i = 0; i < numEventTypes; ++i)
    {
    // @TODO: The y column is a waste of space since it will end up containing
    //        the same value for every entry. We need added functionality in
    //        vtkPlotPoints to efficiently support fixed y-value plots.
    //        (see vtkPlotPoints::UpdateTableCache)
    vtkSmartPointer<vtkIntArray> xColumn = vtkSmartPointer<vtkIntArray>::New();
    vtkSmartPointer<vtkIntArray> yColumn = vtkSmartPointer<vtkIntArray>::New();
    vtkSmartPointer<vtkIntArray> idColumn = vtkSmartPointer<vtkIntArray>::New();

    xColumn->SetName("X");
    yColumn->SetName(this->EventTypeRegistry->GetType(i).GetName());
    idColumn->SetName("Id");

    this->TimelineTables[i] = vtkSmartPointer<vtkTable>::New();
    this->TimelineTables[i]->AddColumn(xColumn);
    this->TimelineTables[i]->AddColumn(yColumn);
    this->TimelineTables[i]->AddColumn(idColumn);
    }

  // iterate through the events and set the corresponding table values
  this->EventModel->InitEventTraversal();
  while (vtkVgEvent* event = this->EventModel->GetNextEvent().GetEvent())
    {
    vtkIdType startFrame = event->GetStartFrame().GetFrameNumber();
    if (startFrame < numFrames)
      {
      int type =
        this->EventTypeRegistry->GetTypeIndex(event->GetActiveClassifierType());

      // Ignore events with types that the registry doesn't know about
      // @TODO: Evaluate if something better suited to the event should happen instead
      if (0 <= type && type < numEventTypes)
        {
        // insert x and y value for this event (y value = normalized event type index)
        int prevNumRows = this->TimelineTables[type]->GetNumberOfRows();
        this->TimelineTables[type]->InsertNextBlankRow();
        this->TimelineTables[type]->SetValue(prevNumRows, 0, startFrame + 1);
        this->TimelineTables[type]->SetValue(prevNumRows, 1, type);
        this->TimelineTables[type]->SetValue(prevNumRows, 2, event->GetId());
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::BuildHistogramTableColumns()
{
  std::vector<vtkSmartPointer<vtkIntArray> > columns;
  columns.resize(this->EventTypeRegistry->GetNumberOfTypes() + 1);

  // create the table columns
  for (unsigned i = 0; i < columns.size(); ++i)
    {
    columns[i] = vtkSmartPointer<vtkIntArray>::New();
    if (i == 0)
      {
      columns[i]->SetName("Bin");
      }
    else
      {
      columns[i]->SetName(this->EventTypeRegistry->GetType(i - 1).GetName());
      }
    this->HistogramTable->AddColumn(columns[i]);
    }
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::UpdateHistogramTable()
{
  // one extra row is needed to get correct bar width in the case of
  // one event bin, since the chart computes bar width based on the difference
  // in x values of the first two rows
  this->HistogramTable->SetNumberOfRows(this->NumEventBins + 1);

  int numFrames = this->ViewCoreInstance->getNumberOfFrames();
  this->MaxHistogramX = static_cast<double>(numFrames);

  // fill with initial values
  for (int i = 0; i < this->HistogramTable->GetNumberOfRows(); ++i)
    {
    int binMidpoint = ((i * numFrames) + ((i + 1) * numFrames)) / 2 / this->NumEventBins;
    this->HistogramTable->SetValue(i, 0, binMidpoint);
    for (int j = 1; j < this->HistogramTable->GetNumberOfColumns(); ++j)
      {
      this->HistogramTable->SetValue(i, j, 0);
      }
    }

  // iterate through the events and build the histogram
  this->EventModel->InitEventTraversal();
  while (vtkVgEvent* event = this->EventModel->GetNextEvent().GetEvent())
    {
    vtkIdType startFrame = event->GetStartFrame().GetFrameNumber();
    if (startFrame < numFrames)
      {
      int bin = (startFrame * this->NumEventBins) / numFrames;
      int type =
        this->EventTypeRegistry->GetTypeIndex(event->GetActiveClassifierType());
      vtkVariant prev = this->HistogramTable->GetValue(bin, type + 1);
      this->HistogramTable->SetValue(bin, type + 1, prev.ToInt() + 1);
      }
    }

  // inform the table that it has been modified
  for (int i = 0; i < this->HistogramTable->GetNumberOfColumns(); ++i)
    {
    this->HistogramTable->GetColumn(i)->Modified();
    }
  this->HistogramTable->Modified();
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::UpdateHistogramAxis(int numTicks)
{
  vtkAxis* x = this->HistogramChart->GetAxis(1);
  x->SetNumberOfTicks(numTicks);

  // we need to set some specially crafted points to get autoscale to do what we want
  x->SetPoint1(0.0f, 0.0f);
  x->SetPoint2(this->MaxHistogramX * 45.0f, 0.0f);
  x->SetRange(0.0, this->MaxHistogramX);

  x->SetBehavior(vtkAxis::FIXED);
  x->AutoScale();

  // prevent the chart from trying to scale the axes
  x->SetBehavior(vtkAxis::CUSTOM);
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::AddHistogramPlots(int type)
{
  vtkPlot* plot = 0;
  int eventTypesPlotted = 0;

  vtkSmartPointer<vtkColorSeries> colors = vtkSmartPointer<vtkColorSeries>::New();
  colors->ClearColors();

  // a stacked bar graph is a special type of bar plot
  bool isStackedPlot = false;
  if (type == StackedBarGraph)
    {
    isStackedPlot = true;
    type = vtkChart::BAR;
    }

  int numEventTypes = this->EventTypeRegistry->GetNumberOfTypes();
  for (int i = 0; i < numEventTypes; ++i)
    {
    // see if we have any of this type of event
    bool hasEvents = false;
    vtkAbstractArray* col = this->HistogramTable->GetColumn(i + 1);
    for (vtkIdType j = 0, colSize = col->GetSize(); j < colSize; ++j)
      {
      if (col->GetVariantValue(j).ToInt() > 0)
        {
        hasEvents = true;
        break;
        }
      }

    // don't bother adding a plot if there are no events of this type
    if (!hasEvents)
      {
      continue;
      }

    ++eventTypesPlotted;

    // add color for this event type to the color series
    double r, g, b;
    unsigned char r2, g2, b2;
    this->EventTypeRegistry->GetType(i).GetColor(r, g, b);
    r2 = static_cast<unsigned char>(r * 255);
    g2 = static_cast<unsigned char>(g * 255);
    b2 = static_cast<unsigned char>(b * 255);
    colors->AddColor(vtkColor3ub(r2, g2, b2));

    // add to stacked plot?
    if (isStackedPlot && eventTypesPlotted > 1)
      {
      vtkPlotBar* bar = vtkPlotBar::SafeDownCast(plot);
      bar->SetInputArray(i + 1, this->HistogramTable->GetColumnName(i + 1));
      bar->SetColorSeries(colors);
      continue;
      }

    // don't attempt to draw a line plot with only one point
    if (type == vtkChart::LINE && this->HistogramTable->GetNumberOfRows() < 2)
      {
      type = vtkChart::POINTS;
      }

    plot = this->HistogramChart->AddPlot(type);
    plot->SetInputData(this->HistogramTable, 0, i + 1);
    plot->SetColor(r, g, b);
    }
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::AddTimelinePlots()
{
  int numEventTypes = this->EventTypeRegistry->GetNumberOfTypes();
  for (int i = 0; i < numEventTypes; ++i)
    {
    int numRows = this->TimelineTables[i]->GetNumberOfRows();
    if (numRows > 0)
      {
      vtkVgPlotTimeline* plot = vtkVgPlotTimeline::New();
      this->TimelineChart->AddPlot(plot);
      plot->SetInputData(this->TimelineTables[i], 0, 1);

      double r, g, b;
      this->EventTypeRegistry->GetType(i).GetColor(r, g, b);
      plot->SetColor(r, g, b);
      plot->FastDelete();
      }
    }
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::BuildTimelineChart()
{
  if (this->TimelineView)
    {
    this->Ui->frameSlider->setValue(this->ViewCoreInstance->getCurrentFrameNumber());
    return;
    }

  // setup the context view
  this->TimelineView = vtkContextView::New();
  this->TimelineView->SetInteractor(this->Ui->timelineRenderWidget->GetInteractor());
  this->Ui->timelineRenderWidget->SetRenderWindow(this->TimelineView->GetRenderWindow());
  this->TimelineView->GetRenderer()->SetBackground(1.0, 1.0, 1.0);

  this->TimelineChart = vtkSmartPointer<vtkVgChartTimeline>::New();
  this->TimelineView->GetScene()->AddItem(this->TimelineChart);

  this->BuildTimelineTables();
  this->AddTimelinePlots();

  this->TimelineChart->SetShowLegend(true);

  vtkAxis* yAxis = this->TimelineChart->GetAxis(0);
  vtkAxis* xAxis = this->TimelineChart->GetAxis(1);

  xAxis->SetTitle("time");
  xAxis->SetBehavior(vtkAxis::FIXED);
  xAxis->SetMaximum(xAxis->GetMinimum() + 50.0);

  yAxis->SetBehavior(vtkAxis::FIXED);
  yAxis->SetRange(yAxis->GetMinimum() - 0.5, yAxis->GetMaximum() + 0.5);
  yAxis->SetLabelsVisible(false);
  yAxis->SetTitle("");

  // register selection callback
  TimelineSelectionCallback* cb = TimelineSelectionCallback::New();
  cb->SetParent(this);
  this->TimelineChart->AddObserver(vtkCommand::SelectionChangedEvent, cb);
  cb->Delete();

  // setup frame control widgets
  connect(this->Ui->frameSlider, SIGNAL(valueChanged(int)),
          this->Ui->frameSpinBox, SLOT(setValue(int)));

  connect(this->Ui->frameSpinBox, SIGNAL(valueChanged(int)),
          this->Ui->frameSlider, SLOT(setValue(int)));

  connect(this->Ui->frameSpinBox, SIGNAL(valueChanged(int)),
          this, SLOT(OnFrameChanged(int)));

  this->Ui->frameSlider->setMinimum(1);
  this->Ui->frameSlider->setMaximum(this->ViewCoreInstance->getNumberOfFrames());

  this->Ui->frameSpinBox->setMinimum(this->Ui->frameSlider->minimum());
  this->Ui->frameSpinBox->setMaximum(this->Ui->frameSlider->maximum());

  this->Ui->frameSlider->setValue(this->ViewCoreInstance->getCurrentFrameNumber());
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::BuildHistogramChart()
{
  if (this->HistogramView)
    {
    return;
    }

  // setup the context view
  this->HistogramView = vtkContextView::New();
  this->HistogramView->SetInteractor(this->Ui->histogramRenderWidget->GetInteractor());
  this->Ui->histogramRenderWidget->SetRenderWindow(this->HistogramView->GetRenderWindow());
  this->HistogramView->GetRenderer()->SetBackground(1.0, 1.0, 1.0);

  this->HistogramChart = vtkSmartPointer<vtkChartXY>::New();
  this->HistogramView->GetScene()->AddItem(this->HistogramChart);

  this->HistogramTable = vtkSmartPointer<vtkTable>::New();
  this->BuildHistogramTableColumns();
  this->UpdateHistogramTable();

  this->AddHistogramPlots(this->Ui->plotTypeCombo->itemData(0).toInt());

  // make adjacent bars flush
  this->HistogramChart->SetBarWidthFraction(1.0f);
  this->HistogramChart->SetShowLegend(true);

  vtkAxis* x = this->HistogramChart->GetAxis(1);
  vtkAxis* y = this->HistogramChart->GetAxis(0);
  x->SetTitle("time");
  y->SetTitle("count");

  x->GetTitleProperties()->SetFontSize(14);
  y->GetTitleProperties()->SetFontSize(14);

  // show integer labels
  x->SetNotation(vtkAxis::FIXED_NOTATION);
  x->SetPrecision(0);

  this->UpdateHistogramAxis(this->NumEventBins + 1);
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::showEvent(QShowEvent* /*event*/)
{
  this->move(this->LastWindowPosition);

  if (this->Ui->tabWidget->currentWidget() == this->Ui->TimelineTab)
    {
    this->BuildTimelineChart();
    }
  else
    {
    this->BuildHistogramChart();
    }
}

//-----------------------------------------------------------------------------
void vpTimelineDialog::hideEvent(QHideEvent* /*event*/)
{
  this->LastWindowPosition = this->pos();
}
