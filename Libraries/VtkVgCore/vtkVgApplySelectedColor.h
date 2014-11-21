/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVgApplySelectedColor.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/
// .NAME vtkVgApplySelectedColor - apply colors to a data set.
//
// .SECTION Description
// vtkVgApplySelectedColor performs a coloring of the dataset using default colors,
// lookup tables, annotations, and/or a selection. The output is a
// four-component vtkUnsignedCharArray containing RGBA tuples for each
// element in the dataset. The first input is the dataset to be colored, which
// may be a vtkTable, vtkGraph subclass, or vtkDataSet subclass. The API
// of this algorithm refers to "points" and "cells". For vtkGraph, the
// "points" refer to the graph vertices and "cells" refer to graph edges.
// For vtkTable, "points" refer to table rows. For vtkDataSet subclasses, the
// meaning is obvious.
//
// The algorithm takes four input arrays, specified with
// SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, name) // boolean array with selected = true
// SetInputArrayToProcess(1, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, name) // color scalars (optional)
// and
// SetInputArrayToProcess(2, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, name). // boolean array with selected = true
// SetInputArrayToProcess(3, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, name). // color scalars (optional)
// These set the point and cell data arrays to use to color the data with
// according to the 'selected' bit in array 0 and 2. For vtkGraph,
// vtkTable inputs, you would use FIELD_ASSOCIATION_VERTICES,
// FIELD_ASSOCIATION_EDGES, or FIELD_ASSOCIATION_ROWS as appropriate.
//
// To use the color array generated here, you should do the following:
//
//  mapper->SetScalarModeToUseCellFieldData();
//  mapper->SelectColorArray("vtkVgApplySelectedColor color");
//  mapper->SetScalarVisibility(true);
//
// Colors are assigned with the following priorities:
// <ol>
// <li> If an item is part of the selection, it is colored with that color.
// <li> Otherwise, if the item is part of an annotation, it is colored
//      with the color of the final (top) annotation in the set of layers.
// <li> Otherwise, if the lookup table is used, it is colored using the
//      lookup table color for the data value of the element.
// <li> Otherwise it will be colored with the default color.
// </ol>
//
// Note: The opacity of an unselected item is defined by the multiplication
// of default opacity, lookup table opacity, and annotation opacity, where
// opacity is taken as a number from 0 to 1. So items will never be more opaque
// than any of these three opacities. Selected items are always given the
// selection opacity directly.

#ifndef __vtkVgApplySelectedColor_h
#define __vtkVgApplySelectedColor_h

#include "vtkPassInputTypeAlgorithm.h"

#include <vgExport.h>

class vtkScalarsToColors;
class vtkUnsignedCharArray;

class VTKVG_CORE_EXPORT vtkVgApplySelectedColor
  : public vtkPassInputTypeAlgorithm
{
public:
  static vtkVgApplySelectedColor* New();
  vtkTypeMacro(vtkVgApplySelectedColor, vtkPassInputTypeAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The default point color for all unannotated, unselected elements
  // of the data. This is used if UsePointLookupTable is off.
  vtkSetVector3Macro(DefaultPointColor, double);
  vtkGetVector3Macro(DefaultPointColor, double);

  // Description:
  // The default point opacity for all unannotated, unselected elements
  // of the data. This is used if UsePointLookupTable is off.
  vtkSetMacro(DefaultPointOpacity, double);
  vtkGetMacro(DefaultPointOpacity, double);

  // Description:
  // The point color for all selected elements of the data.
  // This is used if the selection input is available.
  vtkSetVector3Macro(SelectedPointColor, double);
  vtkGetVector3Macro(SelectedPointColor, double);

  // Description:
  // The point opacity for all selected elements of the data.
  // This is used if the selection input is available.
  vtkSetMacro(SelectedPointOpacity, double);
  vtkGetMacro(SelectedPointOpacity, double);

  // Description:
  // The output array name for the point color RGBA array.
  // Default is "vtkVgApplySelectedColor color".
  vtkSetStringMacro(PointColorOutputArrayName);
  vtkGetStringMacro(PointColorOutputArrayName);

  // Description:
  // The default cell color for all unannotated, unselected elements
  // of the data. This is used if UseCellLookupTable is off.
  vtkSetVector3Macro(DefaultCellColor, double);
  vtkGetVector3Macro(DefaultCellColor, double);

  // Description:
  // The default cell opacity for all unannotated, unselected elements
  // of the data. This is used if UseCellLookupTable is off.
  vtkSetMacro(DefaultCellOpacity, double);
  vtkGetMacro(DefaultCellOpacity, double);

  // Description:
  // The cell color for all selected elements of the data.
  // This is used if the selection input is available.
  vtkSetVector3Macro(SelectedCellColor, double);
  vtkGetVector3Macro(SelectedCellColor, double);

  // Description:
  // The cell opacity for all selected elements of the data.
  // This is used if the selection input is available.
  vtkSetMacro(SelectedCellOpacity, double);
  vtkGetMacro(SelectedCellOpacity, double);

  // Description:
  // The output array name for the cell color RGBA array.
  // Default is "vtkVgApplySelectedColor color".
  vtkSetStringMacro(CellColorOutputArrayName);
  vtkGetStringMacro(CellColorOutputArrayName);

  // Description:
  // If on, uses the input scalar array to set the colors of unannotated,
  // unselected elements of the data.
  vtkSetMacro(UsePointScalars, bool);
  vtkGetMacro(UsePointScalars, bool);
  vtkBooleanMacro(UsePointScalars, bool);

  // Description:
  // If on, uses the input scalar array to set the colors of unannotated,
  // unselected elements of the data.
  vtkSetMacro(UseCellScalars, bool);
  vtkGetMacro(UseCellScalars, bool);
  vtkBooleanMacro(UseCellScalars, bool);

  // Description:
  // If on, uses the point selection array to determine selection of points
  vtkSetMacro(UsePointSelection, bool);
  vtkGetMacro(UsePointSelection, bool);
  vtkBooleanMacro(UsePointSelection, bool);

  // Description:
  // If on, uses the cell selection array to determine selection of cells
  vtkSetMacro(UseCellSelection, bool);
  vtkGetMacro(UseCellSelection, bool);
  vtkBooleanMacro(UseCellSelection, bool);

protected:
  vtkVgApplySelectedColor();
  ~vtkVgApplySelectedColor();

  // Description:
  // Convert the vtkGraph into vtkPolyData.
  int RequestData(
    vtkInformation*, vtkInformationVector**, vtkInformationVector*);

  // Description:
  // Set the input type of the algorithm to vtkGraph.
  int FillInputPortInformation(int port, vtkInformation* info);

  void ProcessColorArray(
    vtkAbstractArray* selectedArr,
    vtkUnsignedCharArray* inputColorArr,
    vtkUnsignedCharArray* outputColorArr,
    unsigned char selectedColor[4],
    unsigned char defaultColor[4]);

  double DefaultPointColor[3];
  double DefaultPointOpacity;
  double DefaultCellColor[3];
  double DefaultCellOpacity;
  double SelectedPointColor[3];
  double SelectedPointOpacity;
  double SelectedCellColor[3];
  double SelectedCellOpacity;
  char* PointColorOutputArrayName;
  char* CellColorOutputArrayName;

  bool UsePointScalars;
  bool UsePointSelection;
  bool UseCellScalars;
  bool UseCellSelection;

private:
  vtkVgApplySelectedColor(const vtkVgApplySelectedColor&);  // Not implemented.
  void operator=(const vtkVgApplySelectedColor&);  // Not implemented.
};

#endif
