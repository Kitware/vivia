// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpTimelineDialog_h
#define __vpTimelineDialog_h

#include <QDialog>
#include "vtkSmartPointer.h"

namespace Ui
{
class vpTimelineDialog;
};

class vpViewCore;

class vtkChartXY;
class vtkContextView;
class vtkPlot;
class vtkTable;

class vtkVgChartTimeline;
class vtkVgEventModel;
class vtkVgEventTypeRegistry;

class vpTimelineDialog : public QDialog
{
  Q_OBJECT

public:
  vpTimelineDialog(QWidget* parent = NULL, Qt::WindowFlags flags = 0);
  virtual ~vpTimelineDialog();

  void SetViewCoreInstance(vpViewCore* vc)
    { this->ViewCoreInstance = vc; }

  void SetEventModel(vtkVgEventModel* em)
    { this->EventModel = em; }

  void SetEventTypeRegistry(vtkVgEventTypeRegistry* etr)
    { this->EventTypeRegistry = etr; }

protected:
  // re-implemented from QWidget
  virtual void showEvent(QShowEvent* event);
  virtual void hideEvent(QHideEvent* event);

private:
  void BuildTimelineChart();
  void BuildHistogramChart();

  void BuildTimelineTables();

  void BuildHistogramTableColumns();
  void UpdateHistogramTable();
  void UpdateHistogramAxis(int numTicks);

  void AddHistogramPlots(int type);
  void AddTimelinePlots();

  int  GetCurrentPlotType();

  void OnTimelineSelectionChanged(vtkPlot* plot, vtkIdType val);

signals:
  void SelectedObject(int type, int id);

public slots:
  void SelectEvent(int id);

private slots:
  void OnPageChanged();
  void OnFrameChanged(int frame);
  void OnPlotTypeChanged(int index);
  void OnBinCountChanged(int count);

private:
  class TimelineSelectionCallback;

  Ui::vpTimelineDialog* Ui;
  QPoint LastWindowPosition;

  bool IsSelecting;
  struct SelectionGuard;

  vpViewCore* ViewCoreInstance;
  vtkVgEventModel* EventModel;
  vtkVgEventTypeRegistry* EventTypeRegistry;

  vtkContextView* TimelineView;
  vtkContextView* HistogramView;

  vtkSmartPointer<vtkVgChartTimeline> TimelineChart;
  vtkSmartPointer<vtkChartXY> HistogramChart;

  int NumEventBins;
  double MaxHistogramX;

  vtkSmartPointer<vtkTable> HistogramTable;
  vtkSmartPointer<vtkTable>* TimelineTables;
};

#endif
