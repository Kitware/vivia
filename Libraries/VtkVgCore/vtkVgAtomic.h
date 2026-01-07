// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgAtomic_h
#define __vtkVgAtomic_h

#include <vgExport.h>

#if defined(WIN32) || defined(_WIN32)
  #include <vtkWindows.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
  #include <sys/types.h>
#endif

// Description:
// This class provides a thread safe integer that may be accessed and modified
// in an atomic manner. The integer is signed, and is the same size as a
// pointer.
class VTKVG_CORE_EXPORT vtkVgSignedAtomic
{
public:

  // Description:
  // The POD data type this class wraps. This will vary by platform and pointer
  // size.
#if defined(WIN32) || defined(_WIN32)
  #if defined(_WIN64)
  typedef LONGLONG PodType;
  #else
  typedef LONG PodType;
  #endif
#elif defined(__APPLE__) && defined(__MACH__)
  #if __LP64__
  typedef int64_t PodType;
  #else
  typedef int32_t PodType;
  #endif
#else
  typedef long PodType;
#endif

  // Description:
  // Default constructor. Accepts an optional initial value (default == 0).
  vtkVgSignedAtomic(PodType initialValue = 0);

  // Description:
  // Destructor.
  ~vtkVgSignedAtomic();

  // Description:
  // Get the value of the atomic.
  PodType GetValue() const;

  PodType operator=(PodType newValue);
  PodType operator+=(PodType addend);
  PodType operator-=(PodType subtrahend);
  PodType operator++();
  PodType operator--();
  PodType operator++(int);
  PodType operator--(int);

private:
  volatile PodType Value;

  class Internal;
  Internal* Internal;
};

//-----------------------------------------------------------------------------
#define vtkVgAtomic_ImplementPodOperator(_op) \
  inline vtkVgSignedAtomic::PodType \
  operator _op(const vtkVgSignedAtomic& a, vtkVgSignedAtomic::PodType p) \
    { return a.GetValue() _op p; } \
  inline vtkVgSignedAtomic::PodType \
  operator _op(vtkVgSignedAtomic::PodType p, const vtkVgSignedAtomic& a) \
    { return p _op a.GetValue(); }

#define vtkVgAtomic_ImplementBoolOperator(_op) \
  inline bool \
  operator _op(const vtkVgSignedAtomic& a, vtkVgSignedAtomic::PodType p) \
    { return a.GetValue() _op p; } \
  inline bool \
  operator _op(vtkVgSignedAtomic::PodType p, const vtkVgSignedAtomic& a) \
    { return p _op a.GetValue(); }

//-----------------------------------------------------------------------------
vtkVgAtomic_ImplementPodOperator(+)
vtkVgAtomic_ImplementPodOperator(-)
vtkVgAtomic_ImplementPodOperator(*)
vtkVgAtomic_ImplementPodOperator(/)
vtkVgAtomic_ImplementPodOperator(%)

vtkVgAtomic_ImplementBoolOperator(==)
vtkVgAtomic_ImplementBoolOperator(!=)
vtkVgAtomic_ImplementBoolOperator(<)
vtkVgAtomic_ImplementBoolOperator(>)

//-----------------------------------------------------------------------------
#undef vtkVgAtomic_ImplementPodOperator
#undef vtkVgAtomic_ImplementBoolOperator

#endif
