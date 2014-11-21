/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVgAssignCoordinates.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVgAssignCoordinates - Given two(or three) arrays take the values
// in those arrays and simply assign them to the coordinates of the vertices.
//
// .SECTION Description
// Given two(or three) arrays take the values in those arrays and simply assign
// them to the coordinates of the vertices. Yes you could do this with the array
// calculator, but your mom wears army boots so we're not going to.

#ifndef __vtkVgAssignCoordinates_h
#define __vtkVgAssignCoordinates_h

#include "vtkPassInputTypeAlgorithm.h"

#include <vgExport.h>

class VTKVG_CORE_EXPORT vtkVgAssignCoordinates : public vtkPassInputTypeAlgorithm
{
public:
  static vtkVgAssignCoordinates *New();

  vtkTypeMacro(vtkVgAssignCoordinates, vtkPassInputTypeAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the xyz coordinate array name.
  vtkSetStringMacro(XYZCoordArrayName);
  vtkGetStringMacro(XYZCoordArrayName);

  // Description:
  // Set if you want a random jitter
  vtkSetMacro(Jitter,bool);

protected:
  vtkVgAssignCoordinates();
  ~vtkVgAssignCoordinates();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int FillInputPortInformation(int port, vtkInformation* info);

private:

  char* XYZCoordArrayName;
  bool Jitter;

  vtkVgAssignCoordinates(const vtkVgAssignCoordinates&);  // Not implemented.
  void operator=(const vtkVgAssignCoordinates&);  // Not implemented.
};

#endif
