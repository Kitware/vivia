/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
