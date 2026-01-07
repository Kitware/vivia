// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgLeafNodeBase_h
#define __vtkVgLeafNodeBase_h

// VG includes.
#include "vtkVgMacros.h"
#include "vtkVgNodeBase.h"

#include <vgExport.h>

// Forward declarations.
class vtkVgPropCollection;
class vtkVgRepresentationBase;

class VTKVG_SCENEGRAPH_EXPORT vtkVgLeafNodeBase : public vtkVgNodeBase
{
public:

  // Description:
  // Easy to use types.
  vtkVgClassMacro(vtkVgLeafNodeBase);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgLeafNodeBase, vtkVgNodeBase);

  virtual void PrintSelf(ostream& os, vtkIndent indent)
    {this->Superclass::PrintSelf(os, indent);}

  // Description:
  // Update \c vtkVgPropCollection with new and expired
  // \c vtkPropCollection 's.
  // Derived classes need to override this.
  virtual void UpdateRenderObjects(
    vtkVgPropCollection* vtkNotUsed(propCollection)) {;}

protected:
  vtkVgLeafNodeBase() {;}
  virtual ~vtkVgLeafNodeBase() {;}

  void     PrepareForAddition(vtkVgPropCollection* propCollection,
                              vtkVgRepresentationBase* representation);
  void     PrepareForRemoval(vtkVgPropCollection* propCollection,
                             vtkVgRepresentationBase* representation);

private:
  vtkVgLeafNodeBase(const vtkVgLeafNodeBase&);
  void operator= (const vtkVgLeafNodeBase&);
};

#endif // __vtkVgLeafNodeBase_h
