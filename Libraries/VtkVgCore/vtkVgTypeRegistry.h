// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTypeRegistry_h
#define __vtkVgTypeRegistry_h

#include "vtkVgTypeRegistryBase.h"

#include <vtkObject.h>

#include <vector>

template <typename T>
class vtkVgTypeRegistry : public vtkObject, vtkVgTypeRegistryBase
{
public:
  vtkTypeMacro(vtkVgTypeRegistry<T>, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Current number of registered types.
  virtual int GetNumberOfTypes() const;

  // Description:
  // Access to registered types.
  const T& GetType(int index) const;
  virtual const vgEntityType& GetEntityType(int index) const;

  // Description:
  // Add to registered types. The supplied type should have a unique id.
  void SetType(int index, const T& type);
  virtual void SetEntityType(int index, const vgEntityType& type);

  // Description:
  // Add to registered types. The supplied type should have a unique id.
  void AddType(const T& type);

  // Description:
  // Remove registered type.
  void RemoveType(int index);

  // Description:
  // Remove all registered types.
  void Clear();

  // Description:
  // Convenience methods to toggle the 'used' bit on registered types.
  virtual void MarkTypeUsed(int index);
  virtual void MarkAllTypesUnused();

private:
  std::vector<T> Types;
};

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgTypeRegistry<T>::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Types:\n";
  for (int i = 0, end = this->GetNumberOfTypes(); i < end; ++i)
    {
    os << indent << this->Types[i].GetName()
       << " (" << this->Types[i].GetId() << ")\n";
    }
}

//-----------------------------------------------------------------------------
template <typename T>
int vtkVgTypeRegistry<T>::GetNumberOfTypes() const
{
  return static_cast<int>(this->Types.size());
}

//-----------------------------------------------------------------------------
template <typename T>
const T& vtkVgTypeRegistry<T>::GetType(int index) const
{
  return this->Types[index];
}

//-----------------------------------------------------------------------------
template <typename T>
const vgEntityType& vtkVgTypeRegistry<T>::GetEntityType(int index) const
{
  return this->GetType(index);
}

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgTypeRegistry<T>::SetType(int index, const T& type)
{
  this->Types[index] = type;
  this->Modified();
}

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgTypeRegistry<T>::SetEntityType(int index, const vgEntityType& type)
{
  this->Types[index].SetEntityTypeInfo(type);
  this->Modified();
}

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgTypeRegistry<T>::AddType(const T& type)
{
  this->Types.push_back(type);
  this->Modified();
}

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgTypeRegistry<T>::RemoveType(int index)
{
  this->Types.erase(this->Types.begin() + index);
  this->Modified();
}

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgTypeRegistry<T>::Clear()
{
  this->Types.clear();
  this->Modified();
}

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgTypeRegistry<T>::MarkTypeUsed(int index)
{
  this->Types[index].SetIsUsed(true);
  this->Modified();
}

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgTypeRegistry<T>::MarkAllTypesUnused()
{
  for (size_t i = 0, end = this->Types.size(); i < end; ++i)
    {
    this->Types[i].SetIsUsed(false);
    }
  this->Modified();
}

#endif // __vtkVgTypeRegistry_h
