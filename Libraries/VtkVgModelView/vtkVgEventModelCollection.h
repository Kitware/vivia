// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgEventModelCollection_h
#define __vtkVgEventModelCollection_h

// VisGUI includes.
#include <vtkVgMacros.h>

// VTK includes.
#include <vtkObject.h>

#include <vgExport.h>

// Forward declarations.
class vtkVgEvent;
class vtkVgEventBase;
class vtkVgEventModel;

class VTKVG_MODELVIEW_EXPORT vtkVgEventModelCollection : public vtkObject
{
public:
  // Description:
  // Macro defined for SmartPtr.
  vtkVgClassMacro(vtkVgEventModelCollection);

  // Description:
  // Macro defined for object hierarchy and internal management.
  vtkTypeMacro(vtkVgEventModelCollection, vtkObject);

  // Description:
  // Print information about data members.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a new instance of this class.
  static vtkVgEventModelCollection* New();

  // Description:
  // Update modified time on contained models
  virtual void Modified();

  // Description:
  // Add/Remove \c vtkVgEventBase to/from known event models.
  vtkVgEvent* AddEvent(vtkVgEventBase* event);
  void        RemoveEvent(vtkIdType id);

  // Description:
  // Get a event give a event id.
  vtkVgEvent* GetEvent(vtkIdType id);

  // Description:
  // Provide access to the internal model.
  vtkVgEventModel* GetInternalEventModel();

  // Description:
  // Add/Remove \c vtkVgEventModel to/from internal store of event models.
  void AddEventModel(vtkVgEventModel* eventModel);
  void RemoveEventModel(vtkVgEventModel* eventModel);

protected:
  vtkVgEventModelCollection();
  virtual ~vtkVgEventModelCollection();

private:

  class Internal;
  Internal* Implementation;

  vtkVgEventModelCollection(const vtkVgEventModelCollection&); // Not implemented.
  void operator= (const vtkVgEventModelCollection&);           // Not implemented.
};

#endif // __vtkVgEventModelCollection_h
