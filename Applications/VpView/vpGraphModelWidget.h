// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpGraphModelWidget_h
#define __vpGraphModelWidget_h

#include <QWidget>

// VisGUI includes
#include <qtGlobal.h>

// VTK includes
#include <vtkGraph.h>
#include <vtkSmartPointer.h>

// Forward declarations
class JSONNode;
class QDoubleSpinBox;
class QTreeWidgetItem;
class vpGraphModelWidgetPrivate;
class vpGraphModelView;
class vpViewCore;
class vtkRenderer;
class vtkVgEvent;
class vtkVgEventModel;
class vtkVgEventTypeRegistry;
class vtkVgInteractorStyleRubberBand2D;
class vtkVgRepresentationBase;

class vpGraphModelWidget : public QWidget
{
  Q_OBJECT

public:
  explicit vpGraphModelWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
  ~vpGraphModelWidget();

  void setSpatialOverlayRenderer(vtkRenderer* ren,
                                 vtkVgInteractorStyleRubberBand2D* istyle);

  void setApplicationCore(vpViewCore* core);

  void addEventNodes(const std::vector<vtkVgEvent*>& events);

signals:
  void eventSelected(int id);

  void distanceMeasurementRequested(bool enable);
  void distanceMeasurementComplete();

  void timeIntervalMeasurementRequested();

public slots:
  void initializeUi();
  void loadEventTypes(vtkVgEventTypeRegistry* reg);
  void loadPrimitiveTypes(const QString& filename = QString());
  void loadAttributeTypes(const QString& filename = QString());
  void loadConfig(const QString& filename);

  void updateViews();
  void updateVertexDisplaySize();

  void importJson();
  void exportJson(QString filename=QString(""));

  void clear();

  void selectEvent(int id);

  void showMainWindowOverlay(bool show);

  void setMeasuredDistance(double meters);
  void setMeasuredTimeInterval(double seconds);

  void resetCamera();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpGraphModelWidget)

  void connectViewSignals(vpGraphModelView* view);

  void resetPrimitives();
  void deleteItemsForNode(int nodeId);
  void deleteItemsForEdge(QString domain, int edgeId);

  void updateNodeControls();
  void updateEdgeControls();

  bool importFrom(const JSONNode& root);
  JSONNode exportToJson();

  void saveState(JSONNode& root);

  void getWorldViewport(double (&viewport)[4], double offX, double offY);

  void setParameterValue(double value, QDoubleSpinBox* widget);

protected slots:
  void enableCreateNode(bool flag);
  void enableCreateEdge(bool flag);

  void createNode(double x, double y);
  void createNodeSpatial(double x, double y);

  void createEdge();
  void createEdge(int parentId, int childId);

  void removeNodes();
  void removeEdges();

  void updateNodeListView(QString nodeType, int nodeId);
  void updateEdgeListView(QString domain, int edgeId);

  void updateNodeItem(QTreeWidgetItem* item, int col);
  void updateEdgeItem(QTreeWidgetItem* item, int col);

  void undo();
  void redo();

  void saveUndoState();

  void selectPickedNodes();
  void selectPickedEdges(QString domain);

  void updateNodeSelection();
  void updateEdgeSelection();
  void updateEdgeInterface();
  void updateParameterWidget(int primitive);

  void pruneEdges();

  void changeGraphLayout(int layoutIndex);
  void copyCurrentLayoutToDefault();

  void autoCreateEdges();

  void setVertexOpacity(int value);

  void updateMovedNodeSpatialPositions();

  void setPrimitiveParameter(double value);

  void pickDistance();
  void cancelPickDistance();

  void pickTimeInterval();

private:
  QTE_DECLARE_PRIVATE(vpGraphModelWidget)
};

#endif // __vpGraphModelWidget_h
