/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// .NAME vtkVgTriangulateConcavePolysFilter - triangulate any concave polygons
// .SECTION Description
// This filter takes input polydata and triangulates any concave polygons,
// passing all other data through (cell data for a concave polygon is copied
// to each of the output triangles resulting from that polygon)

#ifndef __vtkVgTriangulateConcavePolysFilter_h
#define __vtkVgTriangulateConcavePolysFilter_h

#include "vtkPolyDataAlgorithm.h"

#include <vgExport.h>

class VTKVG_CORE_EXPORT vtkVgTriangulateConcavePolysFilter
  : public vtkPolyDataAlgorithm
{
public:
  static vtkVgTriangulateConcavePolysFilter* New();
  vtkTypeMacro(vtkVgTriangulateConcavePolysFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Tests whether the cell is concave
  static bool IsPolygonConcave(vtkPoints* points, vtkIdType npts, vtkIdType* pts);

//BTX
protected:
  vtkVgTriangulateConcavePolysFilter() {};
  ~vtkVgTriangulateConcavePolysFilter() {};

  // Description:
  // This is called within ProcessRequest when a request asks the algorithm
  // to do its work. This is the method you should override to do whatever the
  // algorithm is designed to do. This happens during the fourth pass in the
  // pipeline execution process.
  virtual int RequestData(vtkInformation*,
                          vtkInformationVector**,
                          vtkInformationVector*);

private:
  vtkVgTriangulateConcavePolysFilter(const vtkVgTriangulateConcavePolysFilter&); // Not implemented.
  void operator=(const vtkVgTriangulateConcavePolysFilter&); // Not implemented.
//ETX
};

#endif
