// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgAtomic.h"

// Windows
#if defined(WIN32) || defined(_WIN32)
  #include <vtkWindows.h>
  #define VTK_HAVE_PURE_ATOMICS

  #if defined(_WIN64)
    #define _atomicAdd(_v, _a) (InterlockedExchangeAdd64(_v, _a) + _a)
    #define _atomicIncr InterlockedIncrement64
    #define _atomicDecr InterlockedDecrement64
  #else
    #define _atomicAdd(_v, _a) (InterlockedExchangeAdd(_v, _a) + _a)
    #define _atomicIncr InterlockedIncrement
    #define _atomicDecr InterlockedDecrement
  #endif

// OS/X
#elif defined(__APPLE__) && defined(__MACH__)
  #include <libkern/OSAtomic.h>
  #define VTK_HAVE_PURE_ATOMICS

  #if __LP64__
    #define _atomicAdd(_v, _a) OSAtomicAdd64(_a, _v)
    #define _atomicIncr OSAtomicIncrement64
    #define _atomicDecr OSAtomicDecrement64
  #else
    #define _atomicAdd(_v, _a) OSAtomicAdd32(_a, _v)
    #define _atomicIncr OSAtomicIncrement32
    #define _atomicDecr OSAtomicDecrement32
  #endif

// gcc
#elif defined(VTK_HAVE_SYNC_BUILTINS)
  #define VTK_HAVE_PURE_ATOMICS

  #define _atomicAdd __sync_add_and_fetch
  #define _atomicIncr(_v) __sync_add_and_fetch(_v, 1)
  #define _atomicDecr(_v) __sync_add_and_fetch(_v, -1)
#endif

// Everyone else
#if defined(VTK_HAVE_PURE_ATOMICS)
  #define THIS_ARGS &this->Value
#else

  #include <vtkCriticalSection.h>
  class vtkVgSignedAtomic::Internal : public vtkSimpleCriticalSection {};

  #define THIS_ARGS &this->Value, this->Internal

  namespace // <anonymous>
    {

    typedef vtkVgSignedAtomic::PodType satom_t;
    typedef volatile satom_t* satom_p;

    class Lock
      {
      public:
        Lock(vtkSimpleCriticalSection* mutex) : Mutex(mutex)
          { this->Mutex->Lock(); }
        ~Lock() { this->Mutex->Unlock(); }
      private:
        vtkSimpleCriticalSection* Mutex;
      };

    satom_t _atomicAdd(satom_p v, vtkSimpleCriticalSection* m, satom_t n)
      {
      Lock l(m);
      return (*v) += n;
      }

    satom_t _atomicIncr(satom_p v, vtkSimpleCriticalSection* m)
      {
      Lock l(m);
      return ++(*v);
      }

    satom_t _atomicDecr(satom_p v, vtkSimpleCriticalSection* m)
      {
      Lock l(m);
      return --(*v);
      }

    } // end anonymous namespace

#endif

//-----------------------------------------------------------------------------
vtkVgSignedAtomic::vtkVgSignedAtomic(vtkVgSignedAtomic::PodType initialValue)
  : Value(initialValue)
#if !defined(VTK_HAVE_PURE_ATOMICS)
  , Internal(new class Internal)
#endif
{
}

//-----------------------------------------------------------------------------
vtkVgSignedAtomic::~vtkVgSignedAtomic()
{
#if !defined(VTK_HAVE_PURE_ATOMICS)
  delete this->Internal;
#endif
}

//-----------------------------------------------------------------------------
vtkVgSignedAtomic::PodType vtkVgSignedAtomic::GetValue() const
{
  return this->Value;
}

//-----------------------------------------------------------------------------
vtkVgSignedAtomic::PodType vtkVgSignedAtomic::operator=(
  vtkVgSignedAtomic::PodType newValue)
{
  return this->Value = newValue;
}

//-----------------------------------------------------------------------------
vtkVgSignedAtomic::PodType vtkVgSignedAtomic::operator+=(
  vtkVgSignedAtomic::PodType addend)
{
  return _atomicAdd(THIS_ARGS, addend);
}

//-----------------------------------------------------------------------------
vtkVgSignedAtomic::PodType vtkVgSignedAtomic::operator-=(
  vtkVgSignedAtomic::PodType subtrahend)
{
  return *this += (-subtrahend);
}

//-----------------------------------------------------------------------------
vtkVgSignedAtomic::PodType vtkVgSignedAtomic::operator++()
{
  return _atomicIncr(THIS_ARGS);
}

//-----------------------------------------------------------------------------
vtkVgSignedAtomic::PodType vtkVgSignedAtomic::operator--()
{
  return _atomicDecr(THIS_ARGS);
}

//-----------------------------------------------------------------------------
vtkVgSignedAtomic::PodType vtkVgSignedAtomic::operator++(int)
{
  return _atomicIncr(THIS_ARGS) - 1;
}

//-----------------------------------------------------------------------------
vtkVgSignedAtomic::PodType vtkVgSignedAtomic::operator--(int)
{
  return _atomicDecr(THIS_ARGS) + 1;
}
