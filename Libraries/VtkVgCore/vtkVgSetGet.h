/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgSetGet_h
#define __vtkVgSetGet_h

#include <vtkSetGet.h>

#define vtkVgSetVector2Macro(name,type) \
  vtkSetVector2Macro(name,type) \
  void Set##name (const type _arg[2]) \
  { this->Set##name (_arg[0], _arg[1]); }

#define vtkVgSetVector3Macro(name,type) \
  vtkSetVector3Macro(name,type) \
  void Set##name (const type _arg[3]) \
  { this->Set##name (_arg[0], _arg[1], _arg[2]); }

#define vtkVgSetVector4Macro(name,type) \
  vtkSetVector4Macro(name,type) \
  void Set##name (const type _arg[4]) \
  { this->Set##name (_arg[0], _arg[1], _arg[2], _arg[3]); }

#define vtkVgSetVector6Macro(name,type) \
  vtkSetVector6Macro(name,type) \
  void Set##name (const type _arg[6]) \
  { this->Set##name (_arg[0], _arg[1], _arg[2], _arg[3], _arg[4], _arg[5]); }

#endif
