// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsTimelineSelectionCallback_h
#define __vsTimelineSelectionCallback_h

#include <vtkCommand.h>

class vtkPlot;

class vsTimelineViewer;

// Selection callback class for the timeline chart
class vsTimelineSelectionCallback : public vtkCommand
{
public:
  static vsTimelineSelectionCallback* New()
    {
    return new vsTimelineSelectionCallback();
    }

  vsTimelineSelectionCallback();

  void SetParent(vsTimelineViewer* parent);

  virtual void Execute(vtkObject* caller, unsigned long eventId,
                       void* callData);

protected:
  friend class vsTimelineViewerPlugin;

  vsTimelineViewer* Parent;

  vtkPlot*     LastPlot;
  vtkIdType    LastSelectedIndex;
  vtkIdType    LastSelectionSize;
  vtkIdType    LastSelectionFirstVal;
  vtkIdType    LastSelectionLastVal;
};

#endif
