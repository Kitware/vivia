/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqTimeline_h
#define __vqTimeline_h

#include <QVTKWidget.h>

#include <vtkSmartPointer.h>

class vtkChartXY;
class vtkContextView;
class vtkLookupTable;
class vtkPlot;
class vtkTable;

class vtkVgChartTimeline;
class vtkVgNodeBase;
class vtkVgSceneManager;
class vtkVgVideoModel0;
class vtkVgVideoNode;

namespace TimelineInternal
{
struct SceneVisitor;
struct NodeInfo;
}

class vqTimeline : public QVTKWidget
{
  Q_OBJECT

public:
  vqTimeline(QWidget* parent = NULL);
  virtual ~vqTimeline();

  void SetLookupTable(vtkLookupTable* lut)
    { this->LookupTable = lut; }

  void Initialize(QList<vtkVgVideoNode*> nodes);

  virtual QSize sizeHint() const
    { return QSize(500, 200); }

  virtual QSize minimumSizeHint() const
    { return QSize(300, 100); }

  virtual void showEvent(QShowEvent* event);

signals:
  void ActivatedNode(vtkVgNodeBase& node);
  void SelectedNodes(QList<vtkVgNodeBase*> nodes);

public slots:
  void SelectNodes(QList<vtkVgNodeBase*> node);
  void UpdateColors();
  void Update();
  void ResetView();
  void OnKeyPressed();

private slots:
  void OnFrameChanged(int frame);

private:
  void UpdateTimelineChart();
  void UpdateTimelineTable();
  void UpdateSelection();

  void AddTimelinePlots();

  double GetUnixEpochTime(double useconds);

  int FindFreeY(const TimelineInternal::NodeInfo& info,
                std::vector<std::pair<bool, double> >& used,
                double padding);

  int SetRows(int row,
              const TimelineInternal::NodeInfo& info,
              std::vector<std::pair<bool, double> >& used,
              double padding);

private:
  class TimelineSelectionCallback;

  QPoint LastWindowPosition;

  vtkVgSceneManager* SceneManager;

  TimelineInternal::SceneVisitor* Visitor;
  friend struct TimelineInternal::SceneVisitor;

  bool IsSelecting;
  struct SelectionGuard;

  vtkSmartPointer<vtkContextView> TimelineView;

  vtkSmartPointer<vtkVgChartTimeline> TimelineChart;
  vtkSmartPointer<vtkTable> TimelineTable;

  double MinX, MaxX;
  int    MinY, MaxY;

  vtkLookupTable* LookupTable;
};

#endif
