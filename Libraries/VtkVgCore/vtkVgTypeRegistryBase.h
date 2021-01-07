// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTypeRegistryBase_h
#define __vtkVgTypeRegistryBase_h

class vgEntityType;

class vtkVgTypeRegistryBase
{
public:
  virtual ~vtkVgTypeRegistryBase() {}

  virtual int GetNumberOfTypes() const = 0;

  // Description:
  // Access base entity type.
  virtual const vgEntityType& GetEntityType(int index) const = 0;

  // Description:
  // Assign base entity type information.
  virtual void SetEntityType(int index, const vgEntityType& type) = 0;

  // Description:
  // Convenience methods to toggle the 'used' bit on registered types.
  virtual void MarkTypeUsed(int index) = 0;
  virtual void MarkAllTypesUnused() = 0;
};

#endif // __vtkVgTypeRegistryBase_h
