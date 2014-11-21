/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVgAssignCoordinatesLayoutStrategy.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVgAssignCoordinatesLayoutStrategy - uses array values to set vertex locations
//
// .SECTION Description
// Uses vtkAssignCoordinates to use values from arrays as the x, y, and z coordinates.

#ifndef __vtkVgAssignCoordinatesLayoutStrategy_h
#define __vtkVgAssignCoordinatesLayoutStrategy_h

#include "vtkGraphLayoutStrategy.h"
#include "vtkSmartPointer.h" // For SP ivars

#include <vgExport.h>

class vtkVgAssignCoordinates;

class VTKVG_CORE_EXPORT vtkVgAssignCoordinatesLayoutStrategy : public vtkGraphLayoutStrategy
{
public:
  static vtkVgAssignCoordinatesLayoutStrategy *New();
  vtkTypeMacro(vtkVgAssignCoordinatesLayoutStrategy, vtkGraphLayoutStrategy);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The array to use for the xyz coordinate values.
  virtual void SetXYZCoordArrayName(const char* name);
  virtual const char* GetXYZCoordArrayName();

  // Description:
  // Perform the random layout.
  void Layout();

  void SetJitter(bool enable);

protected:
  vtkVgAssignCoordinatesLayoutStrategy();
  ~vtkVgAssignCoordinatesLayoutStrategy();

  //BTX
  vtkSmartPointer<vtkVgAssignCoordinates> AssignCoordinates;
  //ETX

private:
  vtkVgAssignCoordinatesLayoutStrategy(const vtkVgAssignCoordinatesLayoutStrategy&);  // Not implemented.
  void operator=(const vtkVgAssignCoordinatesLayoutStrategy&);  // Not implemented.
};

#endif
