// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// .NAME vtkVgChartTimeline - Class for drawing timeline charts
//
// .SECTION Description
// This class implements an XY chart. This class expects that data (vtkTable)
// will have columns with very specific names such as 'Id' for object ids and
// 'XD' for duration of objects.

#ifndef __vtkVgChartTimeline_h
#define __vtkVgChartTimeline_h

#include <vgRange.h>

#include <vgExport.h>

#include <vtkChartXY.h>

class vtkIdTypeArray;
class vtkPlot;

class VTKVG_CORE_EXPORT vtkVgChartTimeline : public vtkChartXY
{
public:
  struct SelectionData
    {
    vtkPlot* Plot;
    bool IsActivation;
    };

public:
  vtkTypeMacro(vtkVgChartTimeline, vtkChartXY);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  enum {
    BOTTOM_MAJOR = 100
  };

  // Description:
  // Creates a 2D Chart object.
  static vtkVgChartTimeline* New();

  virtual void Update();

  // Description:
  // Mouse move event.
  virtual bool MouseMoveEvent(const vtkContextMouseEvent& mouse);

  // Description:
  // Mouse button double click event.
  virtual bool MouseDoubleClickEvent(const vtkContextMouseEvent& mouse);

  // Description:
  // Mouse button release event.
  virtual bool MouseButtonReleaseEvent(const vtkContextMouseEvent& mouse);

  // Description:
  // Mouse wheel event, positive delta indicates forward movement of the wheel.
  virtual bool MouseWheelEvent(const vtkContextMouseEvent& mouse, int delta);

  virtual bool Hit(const vtkContextMouseEvent& mouse);

  virtual vtkAxis* GetAxis(int axisIndex);

  // Description:
  // Sets the selection for a plot and focuses it in the view.
  void SetSelection(int plot, vtkIdTypeArray* selection);

  // Description:
  // Clear the selection for all plots.
  void ClearSelection();

  // Description:
  // Get/Set the min and max x value of the chart.
  vgRange<double> GetXExtents() const;
  void SetXExtents(double min, double max);
  void SetXExtents(vgRange<double> x)
    { this->SetXExtents(x.lower, x.upper); }

  // Description:
  // Set the allowed range of X extents.
  vgRange<double> GetXExtentLimits() const;
  void SetXExtentLimits(double min, double max);
  void SetXExtentLimits(vgRange<double> x)
    { this->SetXExtentLimits(x.lower, x.upper); }

  // Description:
  // Reset the chart extents to the default (fit to input).
  void ResetExtents();

  // Description:
  // Set the position of the current time indicator.
  void SetCurrentTime(double);

  // Description:
  // Whether to use supplemental x column to generate normalized input.
  vtkSetMacro(NormalizeInput, bool);
  vtkGetMacro(NormalizeInput, bool);

protected:
  vtkVgChartTimeline();
  ~vtkVgChartTimeline();

  void BuildAxis();
  void FixZoomLevel();

  void DoSelect(const vtkContextMouseEvent& mouse, bool activate);

  virtual bool UpdateLayout(vtkContext2D* painter);

private:
  void MakePointVisible(float x, float y);
  bool UpdateTooltip(const vtkContextMouseEvent& mouse);
  void BuildXColumn();
  void NormalizeXAxis();

  double MinX, MaxX;
  double MinXLimit, MaxXLimit;

  bool NormalizeInput;
  bool ExtentsAreValid;

  vtkTimeStamp DataBuildTime;
  vtkTimeStamp AxisBuildTime;

  vtkAxis* xAxis2;
};

#endif //__vtkVgChartTimeline_h
