// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgPropCollection_h
#define __vtkVgPropCollection_h

// VTK includes.
#include <vtkObject.h>
#include <vtkSmartPointer.h>

// STL includes.
#include <map>
#include <vector>

// VG includes.
#include "vtkVgMacros.h"

#include <vgExport.h>

// Forward declarations.
class vtkProp;
class vtkPropCollection;

class VTKVG_SCENEGRAPH_EXPORT vtkVgPropCollection : public vtkObject
{
public:
  // Description:
  // Define easy to use types.
  vtkVgClassMacro(vtkVgPropCollection);

  // Keys will be sorted if using STL map.
  typedef std::map<int, std::vector< vtkSmartPointer<vtkProp> > > SortedProps;
  typedef SortedProps::iterator                   Itertor;
  typedef SortedProps::const_iterator             ConstItertor;
  typedef std::vector<vtkSmartPointer<vtkProp> > Props;

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgPropCollection, vtkObject);

  static vtkVgPropCollection* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get dirty flag. This indicates state is dirty.
  vtkSetMacro(Dirty, bool);
  vtkGetMacro(Dirty, bool);
  vtkBooleanMacro(Dirty, bool);

  // Description:
  // Set/Get dirty flag. This indicates state is dirty.
  vtkSetMacro(DirtyCache, bool);
  vtkGetMacro(DirtyCache, bool);
  vtkBooleanMacro(DirtyCache, bool);

  // Description:
  // Add new / expired \c vtkPropCollection. \param layerIndex
  // decided which order vtkProps gets rendered.
  void AddNew(vtkPropCollection* collection, int layerIndex = 1000);
  void AddExpired(vtkPropCollection* collection, int layerIndex = 1000);

  // Description:
  // Reset state.
  void Reset();
  void ResetComplete();

  const SortedProps& GetActiveProps() const;
  SortedProps& GetActiveProps();

  const SortedProps& GetExpiredProps() const;
  SortedProps& GetExpiredProps();

protected:
  vtkVgPropCollection();
  virtual ~vtkVgPropCollection();

  bool        Dirty;
  bool        DirtyCache;

  const int   AllocationSize;

  SortedProps ActiveProps;
  SortedProps ExpiredProps;

private:
  vtkVgPropCollection(const vtkVgPropCollection&); // Not implemented.
  void operator= (const vtkVgPropCollection&);     // Not implemented.
};

#endif // __vtkVgPropCollection_h
