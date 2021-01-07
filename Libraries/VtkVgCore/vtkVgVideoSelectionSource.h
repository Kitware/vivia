// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// .NAME vtkVgVideoSelectionSource -
// .SECTION Description

#ifndef __vtkVgVideoSelectionSource_h
#define __vtkVgVideoSelectionSource_h

#include "vtkPolyDataAlgorithm.h"

#include <vgExport.h>

class VTKVG_CORE_EXPORT vtkVgVideoSelectionSource : public vtkPolyDataAlgorithm
{
public:
  static vtkVgVideoSelectionSource* New();
  vtkTypeMacro(vtkVgVideoSelectionSource, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the bounds of the box to be used in Axis Aligned mode.
  vtkSetVector6Macro(Bounds, double);
  vtkGetVectorMacro(Bounds, double, 6);

protected:
  vtkVgVideoSelectionSource();
  ~vtkVgVideoSelectionSource() {}

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
  double Bounds[6];
  double RelativeTriangleSize;

private:
  vtkVgVideoSelectionSource(const vtkVgVideoSelectionSource&);  // Not implemented.
  void operator=(const vtkVgVideoSelectionSource&);  // Not implemented.
};

#endif
