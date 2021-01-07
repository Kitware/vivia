// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsTimelineSelectionCallback.h"

#include "vsTimelineViewer.h"

#include <vtkVgChartTimeline.h>

#include <vtkIdTypeArray.h>
#include <vtkPlot.h>

//----------------------------------------------------------------------------
vsTimelineSelectionCallback::vsTimelineSelectionCallback() :
  Parent(0),
  LastPlot(0),
  LastSelectedIndex(-1),
  LastSelectionSize(0),
  LastSelectionFirstVal(-1),
  LastSelectionLastVal(-1)
{
}

//----------------------------------------------------------------------------
void vsTimelineSelectionCallback::SetParent(vsTimelineViewer* parent)
{
  this->Parent = parent;
}

//----------------------------------------------------------------------------
void vsTimelineSelectionCallback::Execute(vtkObject*, unsigned long,
                                          void* callData)
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
    this->Parent->setSelectedEntity(NULL, -1);
    this->LastPlot = 0;
    return;
    }

  vtkIdTypeArray* selection = plot->GetSelection();
  vtkIdType size = selection->GetNumberOfTuples();

  if (size == 0)
    {
    this->Parent->setSelectedEntity(NULL, -1);
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

  this->Parent->setSelectedEntity(plot, selectedVal);
  if (sd->IsActivation)
    {
    this->Parent->activateEntity(plot, selectedVal);
    }
}
