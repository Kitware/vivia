/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgContourOperatorManager_h
#define __vtkVgContourOperatorManager_h

#include "vtkObject.h"

#include <vgExport.h>

class vtkIdList;
class vtkImplicitBoolean;
class vtkImplicitSelectionLoop;
class vtkPoints;
class vtkPolyLine;

class VTKVG_CORE_EXPORT vtkVgContourOperatorManager : public vtkObject
{
public:
  static vtkVgContourOperatorManager* New();
  vtkTypeMacro(vtkVgContourOperatorManager, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Remove all operators, regardless of type
  bool RemoveAllOperators();

  // Description:
  // If not a filter, then it is a selector
  bool AddFilter(vtkPoints* loopPts);
  bool RemoveFilter(vtkPoints* loopPts);
  bool RemoveAllFilters();
  int GetNumberOfFilters();
  int GetNumberOfEnabledFilters();

  // Description:
  // If not a filter, then it is a selector
  bool AddSelector(vtkPoints* loopPts);
  bool RemoveSelector(vtkPoints* loopPts);
  bool RemoveAllSelectors();
  int GetNumberOfSelectors();
  int GetNumberOfEnabledSelectors();

  void SetContourEnabled(vtkPoints* loopPts, bool state);
  bool GetContourEnabled(vtkPoints* loopPts);

  vtkGetObjectMacro(FilterBoolean, vtkImplicitBoolean);
  vtkGetObjectMacro(SelectorBoolean, vtkImplicitBoolean);

  vtkImplicitSelectionLoop* GetContourLoop(vtkPoints* loopPts);

  // Description:
  // Return true if any point in the "path" passes current set
  // of filters and selectors
  bool EvaluatePath(vtkPoints* points, vtkIdList* ptIds);

  // Description:
  // Return true if the passes current set of filters and selectors
  bool EvaluatePoint(double testPt[3]);

  // Description:
  // Return the Modified tim accounting for changes to actual contour objects
  virtual vtkMTimeType GetMTime();

//BTX
protected:
  vtkVgContourOperatorManager();
  ~vtkVgContourOperatorManager();

  vtkImplicitBoolean* FilterBoolean;
  vtkImplicitBoolean* SelectorBoolean;

  class vtkInternal;
  vtkInternal* Internals;

//ETX
};

#endif
