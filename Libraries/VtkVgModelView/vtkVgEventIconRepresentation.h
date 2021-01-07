// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgEventIconRepresentation_h
#define __vtkVgEventIconRepresentation_h

#include "vtkSmartPointer.h"
#include "vtkTimeStamp.h"
#include "vtkVgEventRepresentationBase.h"

#include <vgExport.h>

class vtkPropCollection;
class vtkRenderer;

class vtkVgEvent;
class vtkVgIconManager;

class VTKVG_MODELVIEW_EXPORT vtkVgEventIconRepresentation
  : public vtkVgEventRepresentationBase
{
public:
  vtkTypeMacro(vtkVgEventIconRepresentation, vtkVgEventRepresentationBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgEventIconRepresentation* New();

  // Description:
  // Return all the objects that can be rendered.
  virtual const vtkPropCollection* GetNewRenderObjects() const;
  virtual const vtkPropCollection* GetActiveRenderObjects() const;
  virtual const vtkPropCollection* GetExpiredRenderObjects() const;

  virtual vtkPropCollection* GetNewRenderObjects();
  virtual vtkPropCollection* GetActiveRenderObjects();
  virtual vtkPropCollection* GetExpiredRenderObjects();

  virtual void ResetTemporaryRenderObjects();

  // Description:
  virtual void SetVisible(int flag);
  virtual int GetVisible() const;

  // Description:
  // Update all the event actors. Generally called by the application layer.
  virtual void Update();

  // Description:
  // Set the icon manager to coordinate with.
  void SetIconManager(vtkVgIconManager* im);
  vtkGetObjectMacro(IconManager, vtkVgIconManager);

private:
  vtkVgEventIconRepresentation(const vtkVgEventIconRepresentation&);
  void operator=(const vtkVgEventIconRepresentation&);

  vtkVgEventIconRepresentation();
  virtual ~vtkVgEventIconRepresentation();

  void UpdateEventIcons(vtkVgEvent* event);
  void HideEventIcons(vtkVgEvent* event);

  typedef vtkSmartPointer<vtkPropCollection> vtkPropCollectionRef;

  vtkPropCollectionRef  NewPropCollection;
  vtkPropCollectionRef  ActivePropCollection;
  vtkPropCollectionRef  ExpirePropCollection;

  vtkVgIconManager* IconManager;
};

#endif // __vtkVgEventIconRepresentation_h
