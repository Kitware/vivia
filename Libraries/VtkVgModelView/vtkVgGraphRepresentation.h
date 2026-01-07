// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgGraphRepresentation_h
#define __vtkVgGraphRepresentation_h

// VisGUI includes.
#include "vtkVgRepresentationBase.h"

#include "vtkVgPickData.h"

// VTK includes.
#include <vtkSmartPointer.h>

#include <vgExport.h>

class vtkVgEventFilter;
class vtkVgEventTypeRegistry;
class vtkVgGraphModel;

class vtkVgCellPicker;
class vtkPropCollection;
class vtkRenderer;
class vtkSelection;

class VTKVG_MODELVIEW_EXPORT vtkVgGraphRepresentation
  : public vtkVgRepresentationBase
{
public:
  enum GraphEdgeColorMode
    {
    DefaultEdgeColor = 0,
    RoleBasedEdgeColor = 1,
    CountEdgeColorModes
    };

  enum GraphEdgeThicknessMode
    {
    ThinEdge      = 0,
    ThickEdge     = 1,
    ThickerEdge   = 2,
    CountEdgeThicknessModes
    };

  vtkTypeMacro(vtkVgGraphRepresentation, vtkVgRepresentationBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgGraphRepresentation* New();

  static std::string GetGraphEdgeColorModeString(int mode);
  static std::string GetGraphEdgeThicknessModeString(int mode);

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
  // Control visibility.
  virtual void SetVisible(int flag);
  virtual int  GetVisible() const { return this->Visible; }

  // Description:
  // Build and initialize representation structures.
  void Initialize();

  // Description:
  // Set the item on the representation.
  void SetGraphModel(vtkVgGraphModel* modelItem);
  vtkGetObjectMacro(GraphModel, vtkVgGraphModel);

  // Description:
  // Update all the event actors. Generally called by the application layer.
  virtual void Update();

  // Description:
  // Pick operation from display (i.e., pixel) coordinates in the current
  // renderer. Return the picked eventId if a successful pick occurs,
  // otherwise return -1. Note that the pick operation takes into account
  // whether the event is currently visible or not.
  virtual vtkIdType Pick(double renX, double renY, vtkRenderer* ren, vtkIdType& pickType);

  // Description:
  // After a successful pick occurs, return the position of the pick (in
  // world coordinates).
  vtkGetVector3Macro(PickPosition, double);

  // Description:
  // Assign an event type registry. Must be done before initialization.
  void SetEventTypeRegistry(vtkVgEventTypeRegistry* registry);
  vtkGetObjectMacro(EventTypeRegistry, vtkVgEventTypeRegistry);

  // Description:
  // Set edge colors based on some mode.
  void SetGraphEdgeColorMode(int mode);
  int  GetGraphEdgeColorMode();

  // Description:
  // Set edge colors based on some mode.
  void SetGraphEdgeThicknessMode(int mode);
  int  GetGraphEdgeThicknessMode();

  // Description:
  // Set edge opacity for all the edges.
  void SetGraphEdgeOpacity(double value);

  // Description:
  vtkVgPickData::PickData Pick(vtkSelection* selection);

private:
  vtkVgGraphRepresentation(const vtkVgGraphRepresentation&); // Not implemented.
  void operator=(const vtkVgGraphRepresentation&);           // Not implemented.

  vtkVgGraphRepresentation();
  virtual ~vtkVgGraphRepresentation();

  typedef vtkSmartPointer<vtkPropCollection> vtkPropCollectionRef;

  vtkPropCollectionRef  NewPropCollection;
  vtkPropCollectionRef  ActivePropCollection;
  vtkPropCollectionRef  ExpirePropCollection;

  int Visible;

  int GraphEdgeColorMode;
  int GraphEdgeThicknessMode;

  vtkVgGraphModel* GraphModel;

  struct vtkInternal;
  vtkInternal* Internal;

  vtkSmartPointer<vtkVgCellPicker> Picker;
  double PickPosition[3];

  vtkVgEventTypeRegistry* EventTypeRegistry;
};

#endif // __vtkVgGraphRepresentation_h
