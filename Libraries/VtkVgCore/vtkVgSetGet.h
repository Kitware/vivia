// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
