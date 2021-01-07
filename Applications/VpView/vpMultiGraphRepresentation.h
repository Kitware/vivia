// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpMultiGraphRepresentation_h
#define __vpMultiGraphRepresentation_h

#include "vpMultiGraphModel.h"
#include "vpPrimitiveConfig.h"

// VisGUI includes
#include <vgRange.h>
#include <vtkVgRepresentationBase.h>
#include <vtkVgPickData.h>

// VTK includes
#include <vtkActor.h>
#include <vtkSmartPointer.h>

// C++ includes
#include <map>

class vtkVgEventTypeRegistry;
class vtkVgTimeStamp;

class vtkGraph;
class vtkGraphActor;
class vtkHardwareSelector;
class vtkIdTypeArray;
class vtkMatrix3x3;
class vtkPropCollection;
class vtkRenderer;
class vtkSelection;

class vpMultiGraphRepresentation : public vtkVgRepresentationBase
{
public:
  enum LayoutMode
    {
    Default = 0,
    SortByStartTime,
    Spatial,
    RawSpatial
    };

  /// \TODO Use vtkVgClassMacro as well
  vtkTypeMacro(vpMultiGraphRepresentation, vtkVgRepresentationBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vpMultiGraphRepresentation* New();

  static std::string GetGraphEdgeColorModeString(int mode);
  static std::string GetGraphEdgeThicknessModeString(int mode);

  /// Return all the objects that can be rendered.
  virtual const vtkPropCollection* GetNewRenderObjects() const;
  virtual const vtkPropCollection* GetActiveRenderObjects() const;
  virtual const vtkPropCollection* GetExpiredRenderObjects() const;

  virtual vtkPropCollection* GetNewRenderObjects();
  virtual vtkPropCollection* GetActiveRenderObjects();
  virtual vtkPropCollection* GetExpiredRenderObjects();

  virtual void ResetTemporaryRenderObjects();

  /// Control visibility.
  virtual void SetVisible(int flag);
  virtual int  GetVisible() const { return this->Visible; }

  /// Control visibility by domain
  void SetCurrentVisibleGraph(const std::string& key);
  const std::string& GetCurrentVisibleGraph() const;

  /// Build and initialize representation structures.
  void Initialize();

  /// Set the item on the representation.
  void SetGraphModel(vpMultiGraphModel* graphModel);
  vpMultiGraphModel* GetGraphModel() const;

  /// Update all the event actors. Generally called by the application layer.
  virtual void Update();

  /// Pick operation from display (i.e., pixel) coordinates in the current
  /// renderer. Return the picked eventId if a successful pick occurs,
  /// otherwise return -1. Note that the pick operation takes into account
  /// whether the event is currently visible or not.
  virtual vtkIdType Pick(double renX, double renY,
                         vtkRenderer* ren, vtkIdType& pickType);

  // Pick within bounding rectangle
  vtkIdType Pick(double x1, double y1, double x2, double y2,
                 vtkRenderer* ren, vtkIdType& pickType);

  std::string GetPickedDomain()
    {
    return this->PickedGraphDomain;
    }

  vtkIdTypeArray* GetPickedIds();

  void SetEventRegistry(vtkVgEventTypeRegistry* reg);
  vtkVgEventTypeRegistry* GetEventRegistry() const;

  void SetLabelsVisible(bool visible);
  void SetEdgeGlyphsVisible(bool visible);

  // Reset cached positions to current positions
  void ResetCachedNodePositions();

  /// Set graph layout given specific ranges
  void SetLayoutMode(int mode, vtkRenderer *ren,
    const vgRange<double>& widthRange, const vgRange<double>& heightRange);
  int GetLayoutMode();

  void RefreshLayout();

  vpMultiGraphModel::NodePositionType GetNodePositionType();

  /// Set vertex size
  void SetVertexSize(double size);
  double GetVertexSize();

  void SetVertexOpacity(double opacity);
  double GetVertexOpacity() { return this->VertexOpacity; }

  // Description:
  // Set/Get the Z offset to be applied to the actor
  // Note: the Z_OFFSET of the prop will be added to get final Z offset
  vtkSetMacro(ZOffset, double);
  vtkGetMacro(ZOffset, double);

  vtkMatrix3x3* GetGraphToSpatialMatrix();

  void SetForegroundColor(double color[3]);

  // Description:
  // Compute event time using node's position and timeline view
  vtkVgTimeStamp ComputeNodeStartTime(double x, double y, double z);

  // Description:
  // Return current layout extents
  void GetCurrentLayoutExtents(vgRange<double>& xext, vgRange<double>& yext);

protected:
  /// Add graph to the representation
  void AddGraph(vtkGraph* graph, const std::string& domain);

  vtkIdType PickNodes(double x1, double y1, double x2, double y2,
                      vtkRenderer* ren);
  vtkIdType PickEdges(double x1, double y1, double x2, double y2,
                      vtkRenderer* ren);

  /// Layout implementations
  void  UseSortByStartTimeLayout(vtkRenderer* ren,
    const vgRange<double>& widthRange, const vgRange<double>& heightRange,
    int prevLayoutMode);
  void  UseSpatialLayout(vtkRenderer* ren,
    const vgRange<double>& widthRange, const vgRange<double>& heightRange,
    int prevLayoutMode);
  void  UseRandomLayout(vtkRenderer* ren,
    const vgRange<double>& widthRange, const vgRange<double>& heightRange,
    int prevLayoutMode);

private:
  /// Not implemented
  vpMultiGraphRepresentation(const vpMultiGraphRepresentation&);

  /// Not implemented
  void operator=(const vpMultiGraphRepresentation&);

  vpMultiGraphRepresentation();
  virtual ~vpMultiGraphRepresentation();

  /// Reinitialize rendering related data
  void ReinitializeRenderData();

  typedef vtkSmartPointer<vtkPropCollection> vtkPropCollectionRef;

  vtkPropCollectionRef  NewPropCollection;
  vtkPropCollectionRef  ActivePropCollection;
  vtkPropCollectionRef  ExpirePropCollection;

  int Visible;
  std::string CurrentVisibleGraphKey;

  vtkSmartPointer<vpMultiGraphModel> GraphModel;

  class vtkInternal;
  vtkInternal* Internal;

  vtkSmartPointer<vtkHardwareSelector> Picker;
  std::string PickedGraphDomain;

  vtkSmartPointer<vtkVgEventTypeRegistry> EventTypeRegisty;

  vpPrimitiveConfig PrimitiveConfig;

  vgRange<double> CurrentLayoutExtentsX;
  vgRange<double> CurrentLayoutExtentsY;

  vtkSmartPointer<vtkRenderer> CurrentLayoutRenderer;

  int GraphLayoutMode;
  double VertexSize;
  double VertexOpacity;
  double ZOffset;

  double ForegroundColor[3];
};

#endif // __vpMultiGraphRepresentation_h
