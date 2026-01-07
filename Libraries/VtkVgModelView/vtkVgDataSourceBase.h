// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgDataSourceBase_h
#define __vtkVgDataSourceBase_h

// VTK includes.
#include <vtkObject.h>
#include <vtkSmartPointer.h>

// VG includes.
#include "vtkVgMacros.h"

#include <vgExport.h>

class VTKVG_MODELVIEW_EXPORT vtkVgDataSourceBase : public vtkObject
{
public:
  // Description:
  // Define easy to use types.
  vtkVgClassMacro(vtkVgDataSourceBase);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgDataSourceBase, vtkObject);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get data source.
  vtkSetStringMacro(DataSource);
  vtkGetStringMacro(DataSource);
  const char* GetDataSource() const;

  // Description:
  // These parameters could be used to tweak video performance.
  // Default is to do nothing.
  virtual void SetVisibleExtents(const int vtkNotUsed(extents)[4]) {;}
  virtual void SetVisibleScale(const double& vtkNotUsed(scale)) {;}

  // Description:
  // LOD control
  virtual void SetImageLevel(int vtkNotUsed(level)) {;}

  // Description:
  // Update. Does nothing by default.
  virtual void Update() {;}

  virtual void ShallowCopy(vtkVgDataSourceBase& other);
  virtual void DeepCopy(vtkVgDataSourceBase& other);

protected:
  vtkVgDataSourceBase();
  virtual  ~vtkVgDataSourceBase();

  char*     DataSource;

private:
  vtkVgDataSourceBase(const vtkVgDataSourceBase&);  // Not implemened.
  void operator= (const vtkVgDataSourceBase&);      // Not implemented.
};

#endif // __vtkVgDataSourceBase_h
