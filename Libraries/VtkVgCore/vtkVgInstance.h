// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgInstance_h
#define __vtkVgInstance_h

// Description:
// This class provides an instantiable, copyable and assignable wrapper around
// vtkObject. Copying and assignment require that vtkObject subclass implements
// the DeepCopy method.
template <typename T>
class vtkVgInstance
{
public:
  enum AssignmentMode
    {
    // Description:
    // Set the instance's underlying object to the source vtkObject*, and
    // increment the vtkObject's reference count. Use TakeReference when
    // creating an instance from a smart pointer, or when the pointer will be
    // explicitly released later.
    TakeReference,
    // Description:
    // Set the instance's underlying object to the source vtkObject*. The source
    // vtkObject's reference count is not altered. Use TakeOwnership when the
    // vtkVgInstance may take responsibility for managing the pointer.
    TakeOwnership,
    // Description:
    // Create a new vtkObject and copy the data of the source object using
    // DeepCopy. This is equivalent to constructing the instance with the
    // default constructor and then assigning the existing vtkObject to the
    // instance via ::operator=.
    CopyByAssignment
    };

  // Description:
  // Default constructor. Uses the default constructor of the underlying
  // vtkObject.
  vtkVgInstance();

  // Description:
  // Create an instance from an existing vtkObject. The ::AssignmentMode
  // controls how the instance is created.
  vtkVgInstance(T*, AssignmentMode);

  // Description:
  // "Copy" constructor. Create a new instance with a new underlying vtkObject,
  // and copy the data of the other vtkObject onto the new one using DeepCopy.
  vtkVgInstance(const T&);

  // Description:
  // Copy constructor. Create a new instance with a new underlying vtkObject,
  // and copy the data of the other instance's underlying vtkObject onto the
  // new one using DeepCopy.
  vtkVgInstance(const vtkVgInstance<T>&);

  // Description:
  // Destructor. Release the underlying vtkObject.
  ~vtkVgInstance();

  // Description:
  // Assign the data of another instance to this instance. The other instance's
  // underlying vtkObject is copied onto this instance's underlying vtkObject
  // using DeepCopy.
  vtkVgInstance<T>& operator=(const vtkVgInstance<T>&);

  // Description:
  // Assign the data of another vtkObject to this instance. The object is
  // copied onto this instance's underlying vtkObject using DeepCopy.
  vtkVgInstance<T>& operator=(T*);

  // Description:
  // Cast operator. Return a reference to the underlying vtkObject.
  operator T&();

  // Description:
  // Cast operator. Return a const reference to the underlying vtkObject.
  operator const T&() const;

  // Description:
  // Cast operator. Return a pointer to the underlying vtkObject.
  operator T*();

  // Description:
  // Cast operator. Return a const pointer to the underlying vtkObject.
  operator const T*() const;

  // Description:
  // Dereference accessor operator. Access the underlying vtkObject.
  T* operator->();

  // Description:
  // Dereference accessor operator. Access the underlying vtkObject.
  const T* operator->() const;

private:
  T* Object;
};

//-----------------------------------------------------------------------------
template <typename T>
vtkVgInstance<T>::vtkVgInstance()
  : Object(T::New())
{
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgInstance<T>::vtkVgInstance(T* object,
                                typename vtkVgInstance<T>::AssignmentMode mode)
  : Object(0)
{
  switch (mode)
    {
    case TakeReference:
      object->Register(object);
      // fall through
    case TakeOwnership:
      this->Object = object;
      break;
    default: // CopyByAssignment
      this->Object = T::New();
      *this = object;
    }
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgInstance<T>::vtkVgInstance(const T& object)
  : Object(T::New())
{
  this->Object->DeepCopy(&object);
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgInstance<T>::vtkVgInstance(const vtkVgInstance<T>& other)
  : Object(T::New())
{
  this->Object->DeepCopy(other.Object);
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgInstance<T>::~vtkVgInstance()
{
  this->Object->Delete();
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgInstance<T>& vtkVgInstance<T>::operator=(const vtkVgInstance<T>& other)
{
  this->Object->DeepCopy(other.Object);
  return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgInstance<T>& vtkVgInstance<T>::operator=(T* other)
{
  this->Object->DeepCopy(other);
  return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgInstance<T>::operator T&()
{
  return *this->Object;
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgInstance<T>::operator const T&() const
{
  return *this->Object;
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgInstance<T>::operator T*()
{
  return this->Object;
}

//-----------------------------------------------------------------------------
template <typename T>
vtkVgInstance<T>::operator const T*() const
{
  return this->Object;
}

//-----------------------------------------------------------------------------
template <typename T>
T* vtkVgInstance<T>::operator->()
{
  return this->Object;
}

//-----------------------------------------------------------------------------
template <typename T>
const T* vtkVgInstance<T>::operator->() const
{
  return this->Object;
}

#endif
