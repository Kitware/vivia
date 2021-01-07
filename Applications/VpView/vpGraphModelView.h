// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpGraphModelView_h
#define __vpGraphModelView_h

#include "vpMultiGraphModel.h"

#include <QObject>

#include <vtkSmartPointer.h>

#include <vgRange.h>

class vpGraphModelRenderCallback;
class vpMultiGraphModel;
class vpMultiGraphRepresentation;
class vtkActor;
class vtkIdTypeArray;
class vtkMatrix3x3;
class vtkPoints;
class vtkRenderer;
class vtkRenderWindow;
class vtkVgEventTypeRegistry;
class vtkVgInteractorStyleRubberBand2D;

//-----------------------------------------------------------------------------
class vpGraphModelView : public QObject
{
  Q_OBJECT

public:
  enum SelectionType
    {
    ST_None,
    ST_Nodes,
    ST_Edges
    };

public:
  vpGraphModelView();

  void initialize(vpMultiGraphModel* model,
                  vtkRenderer* renderer,
                  vtkVgInteractorStyleRubberBand2D* istyle);

  void setEventRegistry(vtkVgEventTypeRegistry* reg);

  void setGraphLayout(int layout,
                      const vgRange<double>& xExtents,
                      const vgRange<double>& yExtents);
  int  getGraphLayout();

  void copyCurrentLayout();
  void refreshGraphLayout();

  void getGraphBounds(vgRange<double>& xExtents, vgRange<double>& yExtents);

  void getWorldPosition(double xDisplay, double yDisplay,
                        double& xWorld, double& yWorld);

  void setZOffset(double zOffset);

  void setVertexSize(double size);
  double getVertexSize();

  void setVertexOpacity(double opacity);
  double getVertexOpacity();

  vpMultiGraphModel::NodePositionType getNodePositionType();
  vtkMatrix3x3* getGraphToSpatialMatrix();

  void setActive(bool active);

  void render();

  void setCreateNodeEnabled(bool enable);

  void setLightForegroundModeEnabled(bool enable);

signals:
  void createNodeRequested(double x, double y);

  void deleteSelectedNodesRequested();
  void deleteSelectedEdgesRequested();

  void nodesLinked(int parentId, int childId);

  void nodesSelected(vtkIdTypeArray* ids, bool append);
  void edgesSelected(vtkIdTypeArray* ids, const QString& domain, bool append);

  void aboutToMoveNode();
  void nodeMoved();

public slots:
  void setCurrentVisibleGraph(const QString& key);

protected slots:
  void leftMousePress();
  void leftMouseRelease();
  void mouseMove();

  void updateSelection();

  void setGraphRepNeedsUpdate()
    {
    this->GraphRepNeedsUpdate = true;
    }

  void forceRender();

  void deleteSelected(vtkObject*, unsigned long, void*, void*,
                      vtkCommand* command);

protected:
  friend class vpGraphModelRenderCallback;

  void update();

  void selectNodes(vtkIdTypeArray* ids, bool append = false);
  void selectEdges(vtkIdTypeArray* ids, const QString& domain,
                   bool append = false);

  void disableRubberBand();
  void enableRubberBand();

protected:
  vtkSmartPointer<vpMultiGraphModel> GraphModel;
  vtkSmartPointer<vpMultiGraphRepresentation> GraphRepresentation;

  vtkSmartPointer<vpGraphModelRenderCallback> RenderCallback;

  vtkSmartPointer<vtkRenderer> Renderer;
  vtkSmartPointer<vtkRenderWindow> RenderWindow;
  vtkSmartPointer<vtkVgInteractorStyleRubberBand2D> InteractorStyle;

  vtkSmartPointer<vtkActor> LineActor;
  vtkSmartPointer<vtkPoints> LinePoints;

  int HeldNode;
  int ConnectParentNode;

  SelectionType CurrentSelectionType;

  bool CreateNodeEnabled;
  bool RenderPending;
  bool DragStarted;
  bool GraphRepNeedsUpdate;

  int PrevRubberBandMode;
};

#endif // __vpGraphModelView_h
