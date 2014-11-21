/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// .NAME vtkVgChartTimeline - Class for drawing timeline charts
//
// .SECTION Description
// This class implements an XY chart.

#ifndef __vtkVgChartTimeline_h
#define __vtkVgChartTimeline_h

#include "vtkChartXY.h"

#include <vgExport.h>

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

  // Description:
  // Sets the selection for a plot and focuses it in the view.
  void SetSelection(int plot, vtkIdTypeArray* selection);

  // Description:
  // Clear the selection for all plots.
  void ClearSelection();

  // Description:
  // Set the min and max x value of the chart.
  void SetXExtents(double min, double max);

  // Description:
  // Reset the chart extents to the default (fit to input).
  void ResetExtents();

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
