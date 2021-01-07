// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// .NAME vtkVgPlotTimeline - Class for drawing an XY line plot given two columns from
// a vtkTable.
//
// .SECTION Description
//

#ifndef __vtkVgPlotTimeline_h
#define __vtkVgPlotTimeline_h

#include "vtkPlotLine.h"

#include <vgExport.h>

#include <map>

class VTKVG_CORE_EXPORT vtkVgPlotTimeline : public vtkPlotLine
{
public:
  vtkTypeMacro(vtkVgPlotTimeline, vtkPlotLine);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Creates a 2D Chart object.
  static vtkVgPlotTimeline* New();

  // Description:
  // Paint event for the XY plot, called whenever the chart needs to be drawn
  virtual bool Paint(vtkContext2D* painter);

  // Description:
  // Get an array containing the ids of all points in a given area
  vtkIdTypeArray* GetPointIdsInArea(const vtkVector2f& center,
                                    const vtkVector2f& tol);

  vtkIdTypeArray* GetIntervalIdsInArea(const vtkVector2f& point,
                                       const vtkVector2f& tol);

  bool SelectIntervals(const vtkVector2f& point,
                       const vtkVector2f& tol);

  virtual bool SelectPoints(const vtkVector2f& min, const vtkVector2f& max);

  // TODO: We shouldn't need this function, but it seems to prevent a memory
  // leak in vtkCharts.
  virtual void SetSelection(vtkIdTypeArray* id);

  // Description:
  // An interval plot will draw lines between every adjacent pair in the
  // input table.
  vtkSetMacro(IsIntervalPlot, bool);
  vtkGetMacro(IsIntervalPlot, bool);

  // Description:
  // A General setter/getter
  virtual void SetProperty(const vtkStdString& property,
                           const vtkVariant& var);
  virtual vtkVariant GetProperty(const vtkStdString& property);

//BTX
protected:
  vtkVgPlotTimeline();
  ~vtkVgPlotTimeline();

  bool InRange(const vtkVector2f& point, const vtkVector2f& tol,
               const vtkVector2f& current);

  bool InRange(const vtkVector2f& point, const vtkVector2f& tol,
               const vtkVector2f& p1, const vtkVector2f& p2);

  vtkIdType* SortedIds;
  vtkIdTypeArray* AreaPointIds;

  bool IsIntervalPlot;

  vtkTimeStamp SortTime;

  std::map<vtkStdString, vtkVariant> Properties;

private:
  vtkVgPlotTimeline(const vtkVgPlotTimeline&);  // Not implemented.
  void operator=(const vtkVgPlotTimeline&);  // Not implemented.

//ETX
};

#endif //__vtkVgPlotTimeline_h
