/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgPythonUtil_h
#define __vtkVgPythonUtil_h

#define vtkVgPythonAttribute(type, name) \
  vtkVgPythonGetSet(type, name) \
  type name

#define vtkVgPythonGetSet(type, name) \
  type& Get##name() { return this->name; } \
  type const& Get##name() const { return this->name; } \
  void Set##name(type const& value) { this->name = value; }

#endif
