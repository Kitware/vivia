// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgGeode_h
#define __vtkVgGeode_h

// VG includes.
#include "vtkVgMacros.h"
#include "vtkVgLeafNodeBase.h"

// VTK includes.
#include <vtkSmartPointer.h>

#include <vgExport.h>

// Forward declarations.
class vtkProp;
class vtkPropCollection;
class vtkVgPropCollection;
class vtkVgTimeStamp;

class VTKVG_SCENEGRAPH_EXPORT vtkVgGeode : public vtkVgLeafNodeBase
{
public:
  // Description:
  // Easy to use types.
  vtkVgClassMacro(vtkVgGeode);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgGeode, vtkVgLeafNodeBase);

  static vtkVgGeode* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Overridden function.
  virtual int SetVisible(int flag);

  // Description:
  // Add/Remove drawables \c vtkProp 's from the collection.
  void AddDrawable(vtkProp* prop);
  void RemoveDrawable(vtkProp* prop);

  // Description:
  // Get active drawables.
  vtkPropCollection*       GetActiveDrawables();
  const vtkPropCollection* GetActiveDrawables() const;

  // Description:
  // Get new drawables.
  vtkPropCollection*       GetNewDrawables();
  const vtkPropCollection* GetNewDrawables() const;

  // Description:
  // Get expired drawables.
  vtkPropCollection*       GetExpiredDrawables();
  const vtkPropCollection* GetExpiredDrawables() const;

  // Description:
  // Overridden functions.
  virtual void Update(vtkVgNodeVisitorBase& nodeVisitor);

  virtual void UpdateRenderObjects(vtkVgPropCollection* propCollection);

  // Description
  virtual void Accept(vtkVgNodeVisitorBase&  nodeVisitor);

  virtual void Traverse(vtkVgNodeVisitorBase& nodeVisitor);

  virtual void ComputeBounds();

protected:
  vtkVgGeode();
  virtual  ~vtkVgGeode();

  vtkSmartPointer<vtkPropCollection> ActiveDrawables;
  vtkSmartPointer<vtkPropCollection> NewDrawables;
  vtkSmartPointer<vtkPropCollection> ExpiredDrawables;

private:

  vtkVgGeode(const vtkVgGeode&);
  void operator=(vtkVgGeode&);
};

#endif // __vtkVgGeode_h
