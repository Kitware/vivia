// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
