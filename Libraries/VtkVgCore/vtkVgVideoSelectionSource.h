/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
