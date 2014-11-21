/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVgArcParallelEdgeStrategy.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*----------------------------------------------------------------------------
 Copyright (c) Sandia Corporation
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
----------------------------------------------------------------------------*/
// .NAME vtkVgArcParallelEdgeStrategy - routes parallel edges as arcs
//
// .SECTION Description
// Parallel edges are drawn as arcs, and self-loops are drawn as ovals.
// When only one edge connects two vertices it is drawn as a straight line.

#ifndef __vtkVgArcParallelEdgeStrategy_h
#define __vtkVgArcParallelEdgeStrategy_h

#include "vtkEdgeLayoutStrategy.h"

#include <vgExport.h>

class vtkGraph;

class VTKVG_CORE_EXPORT vtkVgArcParallelEdgeStrategy
  : public vtkEdgeLayoutStrategy
{
public:
  static vtkVgArcParallelEdgeStrategy* New();
  vtkTypeMacro(vtkVgArcParallelEdgeStrategy, vtkEdgeLayoutStrategy);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This is the layout method where the graph that was
  // set in SetGraph() is laid out.
  virtual void Layout();

  // Description:
  // Get/Set the number of subdivisions on each edge.
  vtkGetMacro(NumberOfSubdivisions, int);
  vtkSetMacro(NumberOfSubdivisions, int);

  // Description:
  // Get/Set the height of self-loops.
  vtkGetMacro(MinLoopHeight, double);
  vtkSetMacro(MinLoopHeight, double);

  // Description:
  // Get/Set the height of self-loops.
  vtkGetMacro(MaxLoopHeight, double);
  vtkSetMacro(MaxLoopHeight, double);

protected:
  vtkVgArcParallelEdgeStrategy();
  ~vtkVgArcParallelEdgeStrategy();

  int NumberOfSubdivisions;

  double MinLoopHeight;
  double MaxLoopHeight;

private:
  vtkVgArcParallelEdgeStrategy(const vtkVgArcParallelEdgeStrategy&);  // Not implemented.
  void operator=(const vtkVgArcParallelEdgeStrategy&);  // Not implemented.
};

#endif
