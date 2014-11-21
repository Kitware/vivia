/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// .NAME vtkVgIcon - represent a single in the display
// .SECTION Description
// This class represents an icon. It defines a type, and whether it is
// visible. It also requires representational information to display
// including its size (in screen coordinates), an image, and a position.
//
#ifndef __vtkVgIcon_h
#define __vtkVgIcon_h

#include "vtkObject.h"
#include "vtkVgEvent.h"
#include "vtkVgTypeDefs.h"

#include <vgExport.h>

class VTKVG_CORE_EXPORT vtkVgIcon : public vtkObject
{
public:

  // Description:
  // Standard VTK functions.
  static vtkVgIcon* New();
  vtkTypeMacro(vtkVgIcon, vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the type of the icon.
  vtkSetMacro(Category, int);
  vtkGetMacro(Category, int);
  void SetCategoryToEvent()
    {this->SetCategory(vgEventTypeDefinitions::Event);}
  void SetCategoryToActivity()
    {this->SetCategory(vgEventTypeDefinitions::Activity);}
  void SetCategoryToIntelligence()
    {this->SetCategory(vgEventTypeDefinitions::Intel);}
  void SetCategoryToUnknown()
    {this->SetCategory(vgEventTypeDefinitions::CategoryUnknown);}

  // Description:
  // Specify the type of the icon.
  vtkSetMacro(Type, int);
  vtkGetMacro(Type, int);

  // Description:
  // Specify whether the icon is visible.
  vtkSetMacro(Visibility, int);
  vtkGetMacro(Visibility, int);
  vtkBooleanMacro(Visibility, int);

  // Description:
  // Set/get the icon position.
  vtkSetVector2Macro(Position, int);
  vtkGetVector2Macro(Position, int);

  // Description:
  // Set/get the icon scale.
  vtkSetMacro(Scale, double);
  vtkGetMacro(Scale, double);

  // Description:
  // Constructor / Destructor made public.
  vtkVgIcon();
  ~vtkVgIcon() {}

private:
  vtkVgIcon(const vtkVgIcon&); // Not implemented.
  void operator=(const vtkVgIcon&);  // Not implemented.

  int Category;
  int Type;
  int Visibility;
  int Position[2];
  double Scale;

};

#endif // __vtkVgIcon_h
