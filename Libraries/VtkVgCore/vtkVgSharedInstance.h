// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgSharedInstance_h
#define __vtkVgSharedInstance_h

#include "vtkVgAtomic.h"

// Description:
// This class provides an instantiable, copyable and assignable wrapper around
// vtkObject. The underlying object is shared to reduce the overhead of making
// copies. Certain types of access will automatically make a deep copy of the
// underlying vtkObject as needed so that each instance appears to have its
// own copy. The sharing is thread safe.
//
// Note:
// The vtkObject subclass must have a DeepCopy method to be shareable. If an
// instantiable wrapper around vtkObject is desired, but the vtkObject subclass
// does not implement DeepCopy, consider using vtkVgInstance instead.
template <typename T>
class vtkVgSharedInstance
{
public:
  // Description:
  // Default constructor. Uses the default constructor of the underlying
  // vtkObject.
  vtkVgSharedInstance();

  // Description:
  // Create an instance from an existing vtkObject and takes ownership of the
  // vtkObject. Once the instance is constructed, the original pointer should
  // be considered invalid.
  explicit vtkVgSharedInstance(T*);

  // Description:
  // Copy constructor. If possible, the underlying vtkObject is reused.
  // Otherwise, takes a DeepCopy of the underlying vtkObject.
  vtkVgSharedInstance(const vtkVgSharedInstance<T>&);

  // Description:
  // Destructor. If no other instances reference the underlying object, it is
  // released.
  ~vtkVgSharedInstance();

  // Description:
  // Assignment operator. If possible, the underlying vtkObject is reused.
  // Otherwise, takes a DeepCopy of the underlying vtkObject.
  vtkVgSharedInstance<T>& operator=(const vtkVgSharedInstance<T>&);

  // Description:
  // Dereference operator. Allows access to the underlying vtkObject. If the
  // underlying vtkObject is shared, a copy is made first so that changes made
  // to this instance do not affect any other instances. If any operations
  // retain a reference to the underlying vtkObject or any of its internal
  // state, you must call GetPointer before making any copies of the instance
  // to ensure that the reference is not invalidated.
  T* operator->();

  // Description:
  // Dereference operator. Allows access to the underlying vtkObject. If any
  // operations retain a reference to the underlying vtkObject or any of its
  // internal state, you must call GetPointer before making any copies of the
  // instance to ensure that the reference is not invalidated.
  //
  // Note:
  // The compiler will only call the const flavor of operator-> if the object
  // being accessed has const decoration. Merely accessing the object in a way
  // that does not require that the object to be mutable will still call the
  // non-const operator->, which may result in a copy being made of the
  // underlying vtkObject.
  const T* operator->() const;

  // Description:
  // Get the pointer to the underlying vtkObject. If the underlying vtkObject
  // is shared, a copy is made first so that changes made to this instance do
  // not affect any other instances. The internal pointer is then poisoned so
  // that copies of the instance will not share the underlying object. This is
  // necessary to ensure that subsequent calls to GetPointer on the same
  // instance will return the same underlying vtkObject.
  T* GetPointer();

  // Description:
  // Get a volatile pointer to the underlying vtkObject. If the underlying
  // vtkObject is shared, a copy is made first so that changes made to this
  // instance do not affect any other instances. The returned pointer is valid
  // when the method is called, but may become invalid by future access to
  // the instance, including making copies of or destroying the instance. By
  // using this method, you warrant that no references to the underlying
  // vtkObject or its internal state will be retained.
  T* GetVolatilePointer();

  // Description:
  // Get a volatile pointer to the underlying vtkObject. The returned pointer
  // may be shared with other instances, including instances in other threads.
  // The returned pointer is valid when the method is called, but may become
  // invalid by future access to the instance, including making copies of or
  // destroying the instance. By using this method, you warrant that no
  // references to the underlying vtkObject or its internal state will be
  // retained.
  const T* GetVolatileConstPointer() const;

  // Description:
  // Un-poison the internal pointer, allowing copies made of this instance to
  // share the underlying object. By calling this method, you warrant that no
  // references to the underlying vtkObject exist. After calling this method,
  // the underlying vtkObject may be modified or deleted by other instances.
  void ReleasePointer();

  // Description:
  // Assigns a different vtkObject to the instance. This is equivalent to
  // assigning a new instance constructed with the vtkObject pointer to this
  // instance. The instance takes ownership of the vtkObject. After calling
  // this method, the original pointer should be considered invalid.
  void Reset(T*);

private:
  void Copy(const vtkVgSharedInstance<T>&);
  void Detach(bool poison = false);
  void ResetInternal(T*);
  void Release();

  T* Object;
  vtkVgSignedAtomic* RefCount;
};

//-----------------------------------------------------------------------------
template <typename T>
vtkVgSharedInstance<T>::vtkVgSharedInstance()
  : Object(0), RefCount(0)
{
  this->ResetInternal(T::New());
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgSharedInstance<T>::vtkVgSharedInstance(T* object)
  : Object(0), RefCount(0)
{
  this->ResetInternal(object);
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgSharedInstance<T>::vtkVgSharedInstance(
  const vtkVgSharedInstance<T>& other
)
  : Object(0), RefCount(0)
{
  this->Copy(other);
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgSharedInstance<T>::~vtkVgSharedInstance()
{
  this->Release();
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgSharedInstance<T>& vtkVgSharedInstance<T>::operator=(
  const vtkVgSharedInstance<T>& other
)
{
  this->Release();
  this->Copy(other);
  return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
T* vtkVgSharedInstance<T>::operator->()
{
  this->Detach();
  return this->Object;
}

//-----------------------------------------------------------------------------
template <typename T>
const T* vtkVgSharedInstance<T>::operator->() const
{
  return this->Object;
}

//-----------------------------------------------------------------------------
template <typename T>
T* vtkVgSharedInstance<T>::GetPointer()
{
  this->Detach(true);
  return this->Object;
}

//-----------------------------------------------------------------------------
template <typename T>
T* vtkVgSharedInstance<T>::GetVolatilePointer()
{
  this->Detach();
  return this->Object;
}

//-----------------------------------------------------------------------------
template <typename T>
const T* vtkVgSharedInstance<T>::GetVolatileConstPointer() const
{
  return this->Object;
}

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgSharedInstance<T>::ReleasePointer()
{
  if ((*this->RefCount) < 0)
    {
    (*this->RefCount) = 1;
    }
}

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgSharedInstance<T>::Reset(T* object)
{
  this->Release();
  this->ResetInternal(object);
}

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgSharedInstance<T>::Copy(const vtkVgSharedInstance<T>& other)
{
  if (other.Object)
    {
    // Reuse instance if not poisoned
    if ((*other.RefCount) > 0)
      {
      // Not poisoned; increment reference count and share
      this->RefCount = other.RefCount;
      this->Object = other.Object;
      ++(*this->RefCount);
      }
    else
      {
      // Poisoned; make a deep copy
      T* newObject = T::New();
      newObject->DeepCopy(other.Object);
      this->ResetInternal(newObject);
      }
    }
}

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgSharedInstance<T>::Detach(bool poison)
{
  if (this->Object)
    {
    // Ensure unique reference
    if ((*this->RefCount) > 1)
      {
      // If other references are held, replace our object with new deep copy
      T* newObject = T::New();
      newObject->DeepCopy(this->Object);
      --(*this->RefCount);
      this->ResetInternal(newObject);
      }

    // Mark poisoned if requested
    if (poison)
      {
      (*this->RefCount) = -1;
      }
    }
}

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgSharedInstance<T>::ResetInternal(T* object)
{
  if (object)
    {
    this->Object = object;
    this->RefCount = new vtkVgSignedAtomic(1);
    }
  else
    {
    this->Object = 0;
    this->RefCount = 0;
    }
}

//-----------------------------------------------------------------------------
template <typename T>
void vtkVgSharedInstance<T>::Release()
{
  if (this->Object)
    {
    // Release reference and check result
    if (--(*this->RefCount) <= 0)
      {
      // No other references; release reference counter and internal object
      delete this->RefCount;
      this->Object->Delete();
      this->RefCount = 0;
      this->Object = 0;
      }
    }
}

#endif
