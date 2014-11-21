/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
