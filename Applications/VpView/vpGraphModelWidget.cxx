/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpGraphModelWidget.h"

#include "ui_vpGraphModelWidget.h"

// VpView includes
#include "vpAttributeConfig.h"
#include "vpEventConfig.h"
#include "vpGraphModelHelper.h"
#include "vpGraphModelView.h"
#include "vpMultiGraphModel.h"
#include "vpMultiGraphRepresentation.h"
#include "vpPrimitiveConfig.h"
#include "vpViewCore.h"

// VisGUI includes
#include <qtComboBoxDelegate.h>
#include <qtStlUtil.h>
#include <vgFileDialog.h>
#include <vtkVgEvent.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVgInteractorStyleRubberBand2D.h>
#include <vtkVgSpaceConversion.h>

// JSON includes
#include <json.h>

// VTK includes
#include <QVTKWidget.h>
#include <vtkCamera.h>
#include <vtkDataSetAttributes.h>
#include <vtkDoubleArray.h>
#include <vtkEdgeListIterator.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkMatrix3x3.h>
#include <vtkMutableDirectedGraph.h>
#include <vtkMutableUndirectedGraph.h>
#include <vtkPoints.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkStringArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkVariantArray.h>
#include <vtkVertexListIterator.h>

// Qt includes
#include <QApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QList>
#include <QMessageBox>
#include <QShortcut>

#include <QtCore/qmath.h>

enum
{
  NodeIdRole = Qt::UserRole + 1,
  EdgeIdRole,
  EdgeDomainRole
};

//-----------------------------------------------------------------------------
static void setComboItemText(QComboBox* cb, int id, const QString& displayId)
{
  for (int i = 0, end = cb->count(); i != end; ++i)
    {
    if (cb->itemData(i).toInt() == id)
      {
      cb->setItemText(i, displayId);
      break;
      }
    }
}

//-----------------------------------------------------------------------------
static void removeComboItem(QComboBox* cb, int id)
{
  for (int i = 0, end = cb->count(); i != end; ++i)
    {
    if (cb->itemData(i).toInt() == id)
      {
      cb->removeItem(i);
      break;
      }
    }
}

//-----------------------------------------------------------------------------
class vpNoEditDelegate : public QStyledItemDelegate
{
public:
  vpNoEditDelegate(QObject* parent = 0)
    : QStyledItemDelegate(parent)
    {
    }

  // Hack to prevent editing of a column
  virtual QWidget* createEditor(QWidget* /*parent*/,
                                const QStyleOptionViewItem& /*option*/,
                                const QModelIndex& /*index*/) const
    {
    return 0;
    }

private:
  Q_DISABLE_COPY(vpNoEditDelegate)
};

//-----------------------------------------------------------------------------
class vpNodeTypeDelegate : public qtComboBoxDelegate
{
public:
  vpNodeTypeDelegate(vtkVgEventTypeRegistry* types, QObject* parent = 0)
    : qtComboBoxDelegate(parent)
    {
    QVariantList vl;
    QStringList sl;

    for (int i = 0, end = types->GetNumberOfTypes(); i != end; ++i)
      {
      const vgEventType& type = types->GetType(i);
      vl << type.GetId();
      sl << vpEventConfig::GetStringFromId(type.GetId());
      }
    this->setMapping(sl, vl);
    }

  virtual ~vpNodeTypeDelegate()
    {
    }

private:
  Q_DISABLE_COPY(vpNodeTypeDelegate)
};

//-----------------------------------------------------------------------------
class vpAttributeDelegate : public qtComboBoxDelegate
{
public:
  vpAttributeDelegate(vpAttributeConfig& config, QObject* parent = 0)
    : qtComboBoxDelegate(parent)
    {
    QVariantList vl;
    QStringList sl;

    vl << -1;
    sl << "";

    for (int i = 0, end = config.getNumberOfTypes(); i != end; ++i)
      {
      const vpAttributeConfig::vpAttributeType& type
        = config.getAttributeTypeByIndex(i);
      vl << type.Id;
      sl << type.Name;
      }
    this->setMapping(sl, vl);
    }

  virtual ~vpAttributeDelegate()
    {
    }

private:
  Q_DISABLE_COPY(vpAttributeDelegate)
};

//-----------------------------------------------------------------------------
class vpGraphModelWidgetPrivate
{
public:
  vpGraphModelWidgetPrivate(vpGraphModelWidget* parent) :
    GraphModel(vtkSmartPointer<vpMultiGraphModel>::New()), Helper(GraphModel),
    Selecting(false), PickingMaxSpatialWindow(false),
    PickingMaxTemporalWindow(false), q_ptr(parent)
    {
    this->Renderer = vtkSmartPointer<vtkRenderer>::New();
    this->RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    this->InteractorStyle =
      vtkSmartPointer<vtkVgInteractorStyleRubberBand2D>::New();

    this->Renderer->SetBackground(1.0, 1.0, 1.0);
    this->Renderer->GetActiveCamera()->ParallelProjectionOn();
    this->RenderWindow->AddRenderer(this->Renderer);
    this->InteractorStyle->SetRenderer(this->Renderer);
    this->InteractorStyle->SetRubberBandModeToSelection();
    }

public:
  typedef QHash<QString, int> NameToIdMap;

  void enableGraphControls();
  void disableGraphControls();

  void addEdgeAttributes(JSONNode& node,
                         vtkVariantArray* parentAttrArrays,
                         vtkVariantArray* childAttrArrays,
                         int edgeId);

  QString getEdgeAttributeName(int id);
  int getEdgeAttributeId(const QString& name);

  vtkSmartPointer<vpMultiGraphModel> GraphModel;

  vtkSmartPointer<vtkRenderer> Renderer;
  vtkSmartPointer<vtkRenderWindow> RenderWindow;
  vtkSmartPointer<vtkVgInteractorStyleRubberBand2D> InteractorStyle;

  vtkSmartPointer<vtkIntArray> attributeJsonToArray(const JSONNode& node);

  Ui_GraphModelWidget UI;

  vpGraphModelHelper Helper;

  vpGraphModelView View;
  vpGraphModelView SpatialView;

  vpPrimitiveConfig PrimitiveConfig;
  vpAttributeConfig AttributeConfig;

  vpViewCore* AppCore;

  bool Selecting;
  bool PickingMaxSpatialWindow;
  bool PickingMaxTemporalWindow;

  JSONNode UndoState;
  JSONNode RedoState;

protected:
  QTE_DECLARE_PUBLIC_PTR(vpGraphModelWidget)

  void addEdgeAttributesNode(JSONNode& node, const char* name,
                             vtkIntArray* array);

private:
  QTE_DECLARE_PUBLIC(vpGraphModelWidget)
};

//-----------------------------------------------------------------------------
class SelectionGuard
{
  vpGraphModelWidgetPrivate* P;
  bool Prev;

public:
  SelectionGuard(vpGraphModelWidgetPrivate* p)
    : P(p), Prev(p->Selecting)
    {
    this->P->Selecting = true;
    }

  ~SelectionGuard()
    {
    this->P->Selecting = this->Prev;
    }
};

//-----------------------------------------------------------------------------
void vpGraphModelWidgetPrivate::enableGraphControls()
{
  this->UI.addNodeButton->setEnabled(true);
  this->UI.eventTypesComboBox->setEnabled(true);

  this->UI.primitivesLabel->setEnabled(true);
  this->UI.primitivesComboBox->setEnabled(true);

  this->UI.parentNodeLabel->setEnabled(true);
  this->UI.childNodeLabel->setEnabled(true);
  this->UI.addEdgeButton->setEnabled(true);
  this->UI.parentNodesComboBox->setEnabled(true);
  this->UI.childNodesComboBox->setEnabled(true);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidgetPrivate::disableGraphControls()
{
  this->UI.addNodeButton->setDisabled(true);
  this->UI.eventTypesComboBox->setDisabled(true);

  this->UI.primitivesLabel->setDisabled(true);
  this->UI.primitivesComboBox->setDisabled(true);

  this->UI.parentNodeLabel->setDisabled(true);
  this->UI.childNodeLabel->setDisabled(true);
  this->UI.addEdgeButton->setDisabled(true);
  this->UI.parentNodesComboBox->setDisabled(true);
  this->UI.childNodesComboBox->setDisabled(true);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidgetPrivate::addEdgeAttributesNode(
  JSONNode& node, const char* name, vtkIntArray* array)
{
  JSONNode attrs(JSON_ARRAY);
  attrs.set_name(name);

  if (array)
    {
    for (int i = 0, end = array->GetNumberOfTuples(); i < end; ++i)
      {
      QString name = this->getEdgeAttributeName(array->GetValue(i));
      if (!name.isEmpty())
        {
        attrs.push_back(JSONNode("value", qPrintable(name)));
        }
      }
    }

  node.push_back(attrs);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidgetPrivate::addEdgeAttributes(
  JSONNode& node,
  vtkVariantArray* parentAttrArrays,
  vtkVariantArray* childAttrArrays,
  int edgeId)
{
  vtkIntArray* parentAttrs
    = vtkIntArray::SafeDownCast(
        parentAttrArrays->GetValue(edgeId).ToArray());

  vtkIntArray* childAttrs
    = vtkIntArray::SafeDownCast(
        childAttrArrays->GetValue(edgeId).ToArray());

  this->addEdgeAttributesNode(node, "parent_attributes", parentAttrs);
  this->addEdgeAttributesNode(node, "child_attributes", childAttrs);
}

//-----------------------------------------------------------------------------
QString vpGraphModelWidgetPrivate::getEdgeAttributeName(int id)
{
  int idx = this->AttributeConfig.getAttributeTypeIndex(id);
  if (idx != -1)
    {
    return this->AttributeConfig.getAttributeTypeByIndex(idx).Name;
    }
  return QString();
}

//-----------------------------------------------------------------------------
int vpGraphModelWidgetPrivate::getEdgeAttributeId(const QString& name)
{
  return this->AttributeConfig.getAttributeTypeByName(name).Id;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkIntArray>
vpGraphModelWidgetPrivate::attributeJsonToArray(const JSONNode& node)
{
  vtkSmartPointer<vtkIntArray> attrArray;
  for (unsigned int i = 0, end = node.size(); i != end; ++i)
    {
    std::string attr = node[i].as_string();
    int id = this->getEdgeAttributeId(qtString(attr));
    if (id == -1)
      {
      qDebug() << "Skipping unrecognized attribute type:" << qtString(attr);
      continue;
      }
    if (!attrArray)
      {
      attrArray = vtkSmartPointer<vtkIntArray>::New();
      }
    attrArray->InsertNextValue(id);
    }
  return attrArray;
}

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(vpGraphModelWidget)

//-----------------------------------------------------------------------------
vpGraphModelWidget::vpGraphModelWidget(QWidget* parent, Qt::WindowFlags flags) :
  QWidget(parent, flags),
  d_ptr(new vpGraphModelWidgetPrivate(this))
{
  QTE_D(vpGraphModelWidget);

  d->UI.setupUi(this);

  d->RenderWindow->SetInteractor(d->UI.renderWidget->GetInteractor());
  d->UI.renderWidget->SetRenderWindow(d->RenderWindow);
  d->RenderWindow->GetInteractor()->SetInteractorStyle(d->InteractorStyle);

  d->View.initialize(d->GraphModel, d->Renderer, d->InteractorStyle);
  d->View.setActive(true);

  d->SpatialView.setZOffset(0.5);
  d->SpatialView.setLightForegroundModeEnabled(true);

  d->Helper.setNewNodeDefaultEdgeAttr(
    d->AttributeConfig.getAttributeTypeByIndex(0).Id);

  // TODO: Make these configurable
  d->Helper.setStartAttr(
    d->AttributeConfig.getAttributeTypeByName("Beginning").Id);

  d->Helper.setStopAttr(
    d->AttributeConfig.getAttributeTypeByName("end").Id);

  QStringList nodeCols;
  nodeCols << "Label" << "Type" << "Default Edge Attr";
  d->UI.nodeTreeWidget->setHeaderLabels(nodeCols);

  vpAttributeDelegate* defaultAttrDg =
    new vpAttributeDelegate(d->AttributeConfig, d->UI.nodeTreeWidget);

  d->UI.nodeTreeWidget->setItemDelegateForColumn(2, defaultAttrDg);

  QStringList edgeCols;
  edgeCols << "Id" << "Parent Attr" << "Child Attr";
  d->UI.edgeTreeWidget->setHeaderLabels(edgeCols);

  vpNoEditDelegate* ndg = new vpNoEditDelegate(d->UI.edgeTreeWidget);
  vpAttributeDelegate* adg = new vpAttributeDelegate(d->AttributeConfig,
      d->UI.edgeTreeWidget);

  d->UI.edgeTreeWidget->setItemDelegateForColumn(0, ndg);
  d->UI.edgeTreeWidget->setItemDelegateForColumn(1, adg);
  d->UI.edgeTreeWidget->setItemDelegateForColumn(2, adg);

  QShortcut* delNodes =
    new QShortcut(QKeySequence(QKeySequence::Delete), d->UI.nodeTreeWidget,
                  0, 0, Qt::WidgetShortcut);

  QShortcut* delEdges =
    new QShortcut(QKeySequence(QKeySequence::Delete), d->UI.edgeTreeWidget,
                  0, 0, Qt::WidgetShortcut);

  QShortcut* resetCamera =
    new QShortcut(QKeySequence(Qt::Key_R), d->UI.renderWidget,
                  0, 0, Qt::WidgetShortcut);

  connect(delNodes, SIGNAL(activated()), this, SLOT(removeNodes()));
  connect(delEdges, SIGNAL(activated()), this, SLOT(removeEdges()));
  connect(resetCamera, SIGNAL(activated()), this, SLOT(resetCamera()));

  QShortcut* undo = new QShortcut(QKeySequence(QKeySequence::Undo), this);
  connect(undo, SIGNAL(activated()), this, SLOT(undo()));

  QShortcut* redo = new QShortcut(QKeySequence(QKeySequence::Redo), this);
  connect(redo, SIGNAL(activated()), this, SLOT(redo()));

  d->UI.graphLayoutComboBox->addItem("User Layout");
  d->UI.graphLayoutComboBox->addItem("Temporal Layout");
  d->UI.graphLayoutComboBox->addItem("Spatial Layout");
  connect(d->UI.graphLayoutComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(changeGraphLayout(int)));

  d->disableGraphControls();

  this->connectViewSignals(&d->View);
  this->connectViewSignals(&d->SpatialView);

  connect(&d->View, SIGNAL(createNodeRequested(double, double)), this,
          SLOT(createNode(double, double)));

  connect(&d->SpatialView, SIGNAL(createNodeRequested(double, double)), this,
          SLOT(createNodeSpatial(double, double)));

  connect(&d->SpatialView, SIGNAL(nodeMoved()), this,
          SLOT(updateMovedNodeSpatialPositions()));

  connect(&d->View, SIGNAL(nodeMoved()), this, SLOT(updateViews()));
  connect(&d->SpatialView, SIGNAL(nodeMoved()), this, SLOT(updateViews()));

  connect(&d->Helper, SIGNAL(nodeCreated(QString, int)), this,
          SLOT(updateNodeListView(QString, int)));

  connect(&d->Helper, SIGNAL(edgeCreated(QString, int)), this,
          SLOT(updateEdgeListView(QString, int)));

  connect(&d->Helper, SIGNAL(nodesDeleted()), this,
          SLOT(pruneEdges()));

  connect(&d->Helper, SIGNAL(selectedNodesChanged()), this,
          SLOT(selectPickedNodes()));

  connect(&d->Helper, SIGNAL(selectedEdgesChanged(QString)), this,
          SLOT(selectPickedEdges(QString)));

  connect(d->UI.addNodeButton, SIGNAL(toggled(bool)), this,
          SLOT(enableCreateNode(bool)));

  connect(d->UI.addEdgeButton, SIGNAL(clicked()), this,
          SLOT(createEdge()));

  connect(d->UI.removeNodeButton, SIGNAL(clicked()), this,
          SLOT(removeNodes()));

  connect(d->UI.removeEdgeButton, SIGNAL(clicked()), this,
          SLOT(removeEdges()));

  connect(d->UI.eventTypesComboBox, SIGNAL(currentIndexChanged(QString)),
          &d->Helper, SLOT(setNewNodeType(QString)));

  connect(d->UI.primitivesComboBox, SIGNAL(currentIndexChanged(QString)),
          &d->View, SLOT(setCurrentVisibleGraph(QString)));

  connect(d->UI.primitivesComboBox, SIGNAL(currentIndexChanged(QString)),
          &d->SpatialView, SLOT(setCurrentVisibleGraph(QString)));

  connect(d->UI.primitivesComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(updateEdgeSelection()));

  connect(d->UI.primitivesComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(updateEdgeInterface()));

  connect(d->UI.primitivesComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(updateParameterWidget(int)));

  connect(d->UI.distanceParameter, SIGNAL(valueChanged(double)),
          this, SLOT(setPrimitiveParameter(double)));

  connect(d->UI.pickMaxSpatialWindow, SIGNAL(clicked()),
          this, SLOT(pickDistance()));

  connect(d->UI.pickRadius, SIGNAL(clicked()),
          this, SLOT(pickDistance()));

  connect(d->UI.pickMaxTemporalWindow, SIGNAL(clicked()),
          this, SLOT(pickTimeInterval()));

  connect(d->UI.pickTimeInterval, SIGNAL(clicked()),
          this, SLOT(pickTimeInterval()));

  connect(d->UI.timeParameter, SIGNAL(valueChanged(double)),
          this, SLOT(setPrimitiveParameter(double)));

  connect(d->UI.nodeTreeWidget, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateNodeSelection()));

  connect(d->UI.edgeTreeWidget, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateEdgeSelection()));

  connect(d->UI.nodeTreeWidget,
          SIGNAL(itemChanged(QTreeWidgetItem*, int)),
          this, SLOT(updateNodeItem(QTreeWidgetItem*, int)));

  connect(d->UI.edgeTreeWidget,
          SIGNAL(itemChanged(QTreeWidgetItem*, int)),
          this, SLOT(updateEdgeItem(QTreeWidgetItem*, int)));

  connect(d->UI.copyCurrentLayoutButton, SIGNAL(clicked()),
          this, SLOT(copyCurrentLayoutToDefault()));

  connect(d->UI.resetView, SIGNAL(clicked()),
          this, SLOT(resetCamera()));

  connect(d->UI.vertexSize, SIGNAL(valueChanged(int)),
          this, SLOT(updateVertexDisplaySize()));

  connect(d->UI.autoEdgesButton, SIGNAL(clicked()), this,
          SLOT(autoCreateEdges()));

  connect(d->UI.showMainWindowOverlay, SIGNAL(toggled(bool)),
          this, SLOT(showMainWindowOverlay(bool)));

  connect(d->UI.vertexOpacity, SIGNAL(valueChanged(int)),
          this, SLOT(setVertexOpacity(int)));

  d->UI.vertexOpacity->setValue(d->UI.vertexOpacity->maximum());
}

//-----------------------------------------------------------------------------
vpGraphModelWidget::~vpGraphModelWidget()
{
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::connectViewSignals(vpGraphModelView* view)
{
  QTE_D(vpGraphModelWidget);

  connect(view, SIGNAL(deleteSelectedNodesRequested()), this,
          SLOT(removeNodes()));

  connect(view, SIGNAL(deleteSelectedEdgesRequested()), this,
          SLOT(removeEdges()));

  connect(view, SIGNAL(nodesLinked(int, int)), this,
          SLOT(createEdge(int, int)));

  connect(view, SIGNAL(aboutToMoveNode()), this,
          SLOT(saveUndoState()));

  connect(view, SIGNAL(nodesSelected(vtkIdTypeArray*, bool)),
          &d->Helper, SLOT(selectNodes(vtkIdTypeArray*, bool)));

  connect(view, SIGNAL(edgesSelected(vtkIdTypeArray*, QString, bool)),
          &d->Helper, SLOT(selectEdges(vtkIdTypeArray*, QString, bool)));
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::setSpatialOverlayRenderer(
  vtkRenderer* ren, vtkVgInteractorStyleRubberBand2D* istyle)
{
  QTE_D(vpGraphModelWidget);

  d->SpatialView.initialize(d->GraphModel, ren, istyle);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::setApplicationCore(vpViewCore *core)
{
  QTE_D(vpGraphModelWidget);
  if (core)
    {
    d->AppCore = core;
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::addEventNodes(const std::vector<vtkVgEvent*>& events)
{
  QTE_D(vpGraphModelWidget);

  if (events.empty())
    {
    return;
    }

  this->saveUndoState();

  double viewport[4];
  this->getWorldViewport(viewport, 0.20, 0.20);
  vgRange<double> width(viewport[0], viewport[2]);
  vgRange<double> height(viewport[1], viewport[3]);

  // Add nodes in default layout
  int mode = d->View.getGraphLayout();
  if (mode != vpMultiGraphRepresentation::Default)
    {
    d->View.setGraphLayout(
      vpMultiGraphRepresentation::Default, width, height);
    }

  vgRange<double> xExtents, yExtents;
  d->View.getGraphBounds(xExtents, yExtents);
  d->Helper.createEventNodes(events, 0.5 * (xExtents.lower + xExtents.upper),
                             yExtents.lower - 0.3);

  // Return to previous layout
  if (d->View.getGraphLayout() != mode)
    {
    d->View.setGraphLayout(mode, width, height);
    }

  this->resetCamera();
  this->updateViews();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::initializeUi()
{
  QTE_D(vpGraphModelWidget);

  d->UI.vertexSize->setValue(static_cast<int>(d->View.getVertexSize()));

  // NOTE Hiding the add edge interface for now
  d->UI.frame_3->setVisible(false);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::loadEventTypes(
  vtkVgEventTypeRegistry* reg)
{
  if (!reg)
    {
    return;
    }

  // Not using event model for now
  QTE_D(vpGraphModelWidget);

  d->enableGraphControls();

  // \TODO Cache color as well
  int count = reg->GetNumberOfTypes();
  for (int i = 0; i < count; ++i)
    {
    const vgEventType& eventType = reg->GetType(i);
    d->UI.eventTypesComboBox->addItem(
      vpEventConfig::GetStringFromId(eventType.GetId()));
    }
  d->View.setEventRegistry(reg);
  d->SpatialView.setEventRegistry(reg);

  vpNodeTypeDelegate* ntd = new vpNodeTypeDelegate(reg, d->UI.nodeTreeWidget);
  d->UI.nodeTreeWidget->setItemDelegateForColumn(1, ntd);

  this->resetPrimitives();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::loadPrimitiveTypes(const QString& filename)
{
  QTE_D(vpGraphModelWidget);

  if (!filename.isEmpty())
    {
    d->PrimitiveConfig.loadFromFile(filename);
    }

  this->resetPrimitives();

  int count = d->PrimitiveConfig.getNumberOfTypes();
  for (int i = 0; i < count; ++i)
    {
    const vpPrimitiveConfig::vpPrimitiveType& type
      = d->PrimitiveConfig.getPrimitiveTypeByIndex(i);
    d->UI.primitivesComboBox->addItem(type.Name, type.ParamDefaultValue);
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::loadAttributeTypes(const QString& filename)
{
  QTE_D(vpGraphModelWidget);

  if (!filename.isEmpty())
    {
    d->AttributeConfig.loadFromFile(filename);

    vpAttributeDelegate* adg =
      new vpAttributeDelegate(d->AttributeConfig, d->UI.edgeTreeWidget);
    d->UI.edgeTreeWidget->setItemDelegateForColumn(1, adg);
    d->UI.edgeTreeWidget->setItemDelegateForColumn(2, adg);
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::loadConfig(const QString& filename)
{
  if (filename.isEmpty())
    {
    return;
    }

  this->loadPrimitiveTypes(filename);
  this->loadAttributeTypes(filename);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::updateViews()
{
  QTE_D(vpGraphModelWidget);

  d->View.render();

  if (d->UI.showMainWindowOverlay->isChecked())
    {
    d->SpatialView.render();
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::updateVertexDisplaySize()
{
  QTE_D(vpGraphModelWidget);
  double size = static_cast<double>(d->UI.vertexSize->value());
  d->View.setVertexSize(size);
  d->SpatialView.setVertexSize(size);
  this->updateViews();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::importJson()
{
  QTE_D(vpGraphModelWidget);

  QString path = vgFileDialog::getOpenFileName(
    this, "Import Graph Model", QString(), "Graph Model (*.json);;");

  if (path.isEmpty())
    {
    return;
    }

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly))
    {
    qDebug() << "Error opening file" << path << "for reading:"
             << qPrintable(file.errorString());
    return;
    }

  QTextStream in(&file);
  QString str = in.readAll();

  JSONNode root;
  try
    {
    root = libjson::parse(qPrintable(str));
    }
  catch (...)
    {
    qDebug() << "Malformed JSON";
    return;
    }

  // Add nodes in default layout
  d->UI.graphLayoutComboBox->setCurrentIndex(
    vpMultiGraphRepresentation::Default);

  if (this->importFrom(root))
    {
    d->UndoState.clear();
    d->RedoState.clear();
    this->resetCamera();
    this->updateViews();
    }
}

//-----------------------------------------------------------------------------
bool vpGraphModelWidget::importFrom(const JSONNode& root)
{
  QTE_D(vpGraphModelWidget);

  JSONNode nodes;
  try
    {
    nodes = root.at("nodes");
    }
  catch (...)
    {
    qDebug() << "No \"nodes\" element found";
    return false;
    }

  // Clear out any previously constructed graphs
  this->clear();

  try
    {
    JSONNode name = root.at("name");
    d->UI.nameLineEdit->setText(qtString(name.as_string()));
    }
  catch (...)
    {
    // No name
    d->UI.nameLineEdit->setText(QString());
    }

  try
    {
    JSONNode desc = root.at("description");
    d->UI.descriptionLineEdit->setText(qtString(desc.as_string()));
    }
  catch (...)
    {
    // No description
    d->UI.descriptionLineEdit->setText(QString());
    }

  try
    {
    JSONNode id = root.at("activity_id");
    d->UI.activityId->setValue(id.as_int());
    }
  catch (...)
    {
    d->UI.activityId->setValue(0);
    }

  try
    {
    JSONNode maxSpatial = root.at("max_spatial_window");
    d->UI.maxSpatialWindow->setValue(maxSpatial.as_float());
    }
  catch (...)
    {
    d->UI.maxSpatialWindow->setValue(0.0);
    }

  try
    {
    JSONNode maxTemporal= root.at("max_temporal_window");
    d->UI.maxTemporalWindow->setValue(maxTemporal.as_int());
    }
  catch (...)
    {
    d->UI.maxTemporalWindow->setValue(0);
    }

  try
    {
    JSONNode numCommonEvents = root.at("num_common_events");
    d->UI.numCommonEvents->setValue(numCommonEvents.as_int());
    }
  catch (...)
    {
    d->UI.numCommonEvents->setValue(0);
    }

  try
    {
    JSONNode minNumEvents = root.at("min_num_events");
    d->UI.minNumEvents->setValue(minNumEvents.as_int());
    }
  catch (...)
    {
    d->UI.minNumEvents->setValue(0);
    }

  JSONNode params;
  try
    {
    params = root.at("primitive_params");
    }
  catch (...)
    {}

  // Read primitive params
  try
    {
    for (unsigned int i = 0, end = params.size(); i != end; ++i)
      {
      const JSONNode& param = params[i];

      QString domain = qtString(param.at("name").as_string());
      vpPrimitiveConfig::vpPrimitiveType pt =
        d->PrimitiveConfig.getPrimitiveTypeByName(domain);

      if (pt.Id == -1)
        {
        qDebug() << "Skipping param of unrecognized primitive type:"
                 << domain;
        continue;
        }

      double val = param.at("value").as_float();
      d->UI.primitivesComboBox->setItemData(
        d->PrimitiveConfig.getPrimitiveTypeIndex(pt.Id) + 1, val);
      }
    }
  catch (...)
    {
    qDebug() << "Malformed primitive param element";
    return false;
    }

  // Compute spatial to graph transform in case we need to transform positions
  vtkSmartPointer<vtkMatrix3x3> spatialToGraph =
    vtkSmartPointer<vtkMatrix3x3>::New();

  vtkMatrix3x3::Invert(d->View.getGraphToSpatialMatrix(), spatialToGraph);

  try
    {
    for (unsigned int i = 0, end = nodes.size(); i != end; ++i)
      {
      const JSONNode& node = nodes[i];
      int id = node.at("id").as_int();
      std::string label = node.at("label").as_string();
      std::string type = node.at("event_type").as_string();
      double x = node.at("x").as_float();
      double y = node.at("y").as_float();

      try
        {
        double spatialX = node.at("spatial_x").as_float();
        double spatialY = node.at("spatial_y").as_float();
        int eventId = node.at("event_id").as_int();

        vtkVgTimeStamp startTime, endTime;

        JSONNode::const_iterator itr = node.find("event_start_time");
        if (itr != node.end())
          {
          startTime.SetTime(itr->as_float() * 1e6);
          }
        itr = node.find("event_start_frame");
        if (itr != node.end())
          {
          startTime.SetFrameNumber(itr->as_int());
          }

        itr = node.find("event_end_time");
        if (itr != node.end())
          {
          endTime.SetTime(itr->as_float() * 1e6);
          }
        itr = node.find("event_end_frame");
        if (itr != node.end())
          {
          endTime.SetFrameNumber(itr->as_int());
          }

        double startX = node.at("event_start_position_x").as_float();
        double startY = node.at("event_start_position_y").as_float();
        double endX = node.at("event_end_position_x").as_float();
        double endY = node.at("event_end_position_y").as_float();
        double eventPositions[4] = {startX, startY, endX, endY};
        if (d->AppCore)
          {
            d->AppCore->toGraphicsCoordinates(eventPositions[0], eventPositions[1]);
            d->AppCore->toGraphicsCoordinates(eventPositions[2], eventPositions[3]);
          }

        int groupId = node.at("group_id").as_int();

        QString attrName = qtString(node.at("default_edge_attr").as_string());
        int defaultEdgeAttr = d->getEdgeAttributeId(attrName);

        d->Helper.createNodeExternal(id, x, y, type, label,
                                     eventPositions, defaultEdgeAttr, groupId,
                                     startTime.GetTime(), endTime.GetTime(),
                                     startTime.GetFrameNumber(),
                                     endTime.GetFrameNumber(),
                                     spatialX, spatialY, eventId);

        // Compute the spatial layout position if we're in that layout mode
        if (d->View.getGraphLayout() == vpMultiGraphRepresentation::Spatial)
          {
          double pt[3] = { spatialX, spatialY, 1.0 };
          spatialToGraph->MultiplyPoint(pt, pt);

          d->Helper.setNodePosition(id, vpMultiGraphModel::NPT_NormalizedSpatial,
                                    pt[0], pt[1]);
          }
        }
      catch (...)
        {
        d->Helper.createNodeExternal(id, x, y, type, label);
        }
      }
    }
  catch (...)
    {
    qDebug() << "Malformed node element";
    return false;
    }

  try
    {
    JSONNode primitives = root.at("primitives");
    try
      {
      for (unsigned int i = 0, end = primitives.size(); i != end; ++i)
        {
        const JSONNode& prim = primitives[i];
        std::string domain = prim.at("name").as_string();

        if (d->PrimitiveConfig.getPrimitiveTypeByName(
              qtString(domain)).Id == -1)
          {
          qDebug() << "Skipping unrecognized primitive type:"
                   << qtString(domain);
          continue;
          }

        const JSONNode& links = prim.at("links");
        try
          {
          for (unsigned int i = 0, end = links.size(); i != end; ++i)
            {
            const JSONNode& link = links[i];
            int id = link.at("id").as_int();
            int parentId = link.at("parent_id").as_int();
            int childId = link.at("child_id").as_int();

            vtkSmartPointer<vtkIntArray> parentAttrArray
              = d->attributeJsonToArray(link.at("parent_attributes"));

            vtkSmartPointer<vtkIntArray> childAttrArray
              = d->attributeJsonToArray(link.at("child_attributes"));

            d->Helper.createEdgeExternal(id, parentId, childId, domain,
                                         parentAttrArray, childAttrArray);
            d->UI.primitivesComboBox->setCurrentIndex(
              d->UI.primitivesComboBox->findText(qtString(domain)));
            }
          }
        catch (...)
          {
          qDebug() << "Malformed link element";
          return false;
          }
        }
      }
    catch (...)
      {
      qDebug() << "Malformed primitive element";
      return false;
      }
    }
  catch (...)
    {
    // No primitives
    }

  return true;
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::exportJson(QString filepath)
{
  if (filepath.isEmpty())
    {
    filepath = vgFileDialog::getSaveFileName(
      this, "Export Graph Model", QString(), "Graph Model (*.json);;");
    }

  if (filepath.isEmpty())
    {
    return;
    }

  QFile file(filepath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
    qDebug() << "Error opening file" << filepath << "for writing:"
             << qPrintable(file.errorString());
    return;
    }

  try
    {
    JSONNode root = this->exportToJson();

    QString output = qtString(root.write_formatted());
    QTextStream out(&file);
    out << output;

    file.close();
    }
  catch (const std::exception& e)
    {
    qDebug() << "Failed to export graph:" << e.what();
    }
}

//-----------------------------------------------------------------------------
JSONNode vpGraphModelWidget::exportToJson()
{
  QTE_D(vpGraphModelWidget);

  JSONNode root(JSON_NODE);
  root.push_back(JSONNode("description",
                          stdString(d->UI.descriptionLineEdit->text())));
  root.push_back(JSONNode("file", "amg"));
  root.push_back(JSONNode("name",
                          stdString(d->UI.nameLineEdit->text())));

  root.push_back(JSONNode("activity_id", d->UI.activityId->value()));
  root.push_back(JSONNode("max_spatial_window",
                          d->UI.maxSpatialWindow->value()));
  root.push_back(JSONNode("max_temporal_window",
                          d->UI.maxTemporalWindow->value()));
  root.push_back(JSONNode("num_common_events", d->UI.numCommonEvents->value()));
  root.push_back(JSONNode("min_num_events", d->UI.minNumEvents->value()));

  JSONNode params(JSON_ARRAY);
  params.set_name("primitive_params");

  // Write the parameter values for primitives that have them
  for (int i = 1; i < d->UI.primitivesComboBox->count(); ++i)
    {
    const vpPrimitiveConfig::vpPrimitiveType& pt =
      d->PrimitiveConfig.getPrimitiveTypeByIndex(i - 1);

    if (pt.ParamType != vpPrimitiveConfig::PPT_None)
      {
      JSONNode primitive(JSON_NODE);
      primitive.push_back(
        JSONNode("name", stdString(d->UI.primitivesComboBox->itemText(i))));
      primitive.push_back(
        JSONNode("value", d->UI.primitivesComboBox->itemData(i).toDouble()));
      params.push_back(primitive);
      }
    }

  root.push_back(params);

  // First get nodes of the graph model
  vpMultiGraphModel* graphModel = d->GraphModel;

  // Master does not belong to any domain
  vtkGraph* masterGraph = graphModel->GetGraph(vpMultiGraphModel::NoneDomain);

  // Nodes section
  JSONNode nodes(JSON_ARRAY);
  nodes.set_name("nodes");

  vtkDoubleArray* nodePositions = vtkDoubleArray::SafeDownCast(
    masterGraph->GetVertexData()->GetAbstractArray("CachedPositions"));

  vtkStringArray* nodeLabels = vtkStringArray::SafeDownCast(
    masterGraph->GetVertexData()->GetAbstractArray("NodeLabels"));
  vtkStringArray* nodeTypes = vtkStringArray::SafeDownCast(
    masterGraph->GetVertexData()->GetAbstractArray("NodeTypes"));

  vtkDoubleArray* startTime = vtkDoubleArray::SafeDownCast(
    masterGraph->GetVertexData()->GetAbstractArray("StartTime"));
  vtkDoubleArray* endTime = vtkDoubleArray::SafeDownCast(
    masterGraph->GetVertexData()->GetAbstractArray("EndTime"));

  vtkUnsignedIntArray* startFrame = vtkUnsignedIntArray::SafeDownCast(
    masterGraph->GetVertexData()->GetAbstractArray("StartFrame"));
  vtkUnsignedIntArray* endFrame = vtkUnsignedIntArray::SafeDownCast(
    masterGraph->GetVertexData()->GetAbstractArray("EndFrame"));

  vtkSmartPointer<vtkVertexListIterator> vItr
    = vtkSmartPointer<vtkVertexListIterator>::New();
  masterGraph->GetVertices(vItr);

  vtkAbstractArray* nodeIds = masterGraph->GetVertexData()->GetPedigreeIds();

  while (vItr->HasNext())
    {
    JSONNode node(JSON_NODE);

    vtkIdType id = vItr->Next();
    int nodeId = nodeIds->GetVariantValue(id).ToInt();
    node.push_back(JSONNode("id", nodeId));
    node.push_back(JSONNode("virtual",  graphModel->GetNodeVirtualType(nodeId)));
    node.push_back(JSONNode("label", nodeLabels->GetValue(id)));
    node.push_back(JSONNode("event_type", nodeTypes->GetValue(id)));
    node.push_back(JSONNode("event_type_id",
                            vpEventConfig::GetIdFromString(
                              nodeTypes->GetValue(id).c_str())));

    double *pos = nodePositions->GetTuple3(id);
    node.push_back(JSONNode("x", pos[0]));
    node.push_back(JSONNode("y", pos[1]));

    double spatial[2];
    graphModel->GetSpatialPosition(nodeId, spatial[0], spatial[1]);

    node.push_back(JSONNode("spatial_x", spatial[0]));
    node.push_back(JSONNode("spatial_y", spatial[1]));

    node.push_back(JSONNode("event_id", graphModel->GetNodeEventId(nodeId)));
    node.push_back(JSONNode("group_id", graphModel->GetGroupId(nodeId)));

    double st = startTime->GetValue(id);
    if (st != vgTimeStamp::InvalidTime())
      {
      node.push_back(JSONNode("event_start_time", st * 1e-6));
      }
    unsigned int sf = startFrame->GetValue(id);
    if (sf != vgTimeStamp::InvalidFrameNumber())
      {
      node.push_back(JSONNode("event_start_frame", sf));
      }

    double et = endTime->GetValue(id);
    if (et != vgTimeStamp::InvalidTime())
      {
      node.push_back(JSONNode("event_end_time", et * 1e-6));
      }
    unsigned int ef = endFrame->GetValue(id);
    if (ef != vgTimeStamp::InvalidFrameNumber())
      {
      node.push_back(JSONNode("event_end_frame", ef));
      }

    double eventStartPosition[2];
    graphModel->GetStartPosition(nodeId, eventStartPosition[0], eventStartPosition[1]);
    if (d->AppCore)
      {
      d->AppCore->toWindowCoordinates(eventStartPosition);
      }

    double eventEndPosition[2];
    graphModel->GetEndPosition(nodeId, eventEndPosition[0], eventEndPosition[1]);
    if (d->AppCore)
      {
      d->AppCore->toWindowCoordinates(eventEndPosition);
      }

    node.push_back(JSONNode("event_start_position_x", eventStartPosition[0]));
    node.push_back(JSONNode("event_start_position_y", eventStartPosition[1]));
    node.push_back(JSONNode("event_end_position_x", eventEndPosition[0]));
    node.push_back(JSONNode("event_end_position_y", eventEndPosition[1]));

    QString edgeAttr =
      d->getEdgeAttributeName(graphModel->GetNodeDefaultEdgeAttr(nodeId));

    node.push_back(JSONNode("default_edge_attr", qPrintable(edgeAttr)));

    nodes.push_back(node);
    }
  root.push_back(nodes);

  // Now primitives and edges
  JSONNode primitives(JSON_ARRAY);
  primitives.set_name("primitives");

  // Easy to use types
  typedef std::map<std::string, vtkSmartPointer<vtkMutableUndirectedGraph> >
  UnGraphs;
  typedef UnGraphs::const_iterator UnConstItr;

  typedef std::map<std::string, vtkSmartPointer<vtkMutableDirectedGraph> >
  DGraphs;
  typedef DGraphs::const_iterator DConstItr;

  vtkSmartPointer<vtkEdgeListIterator> egItr
    = vtkSmartPointer<vtkEdgeListIterator>::New();

  // Undirected graphs
  UnGraphs unGraphs = graphModel->GetAllUndirectedGraphsWithEdges();
  UnConstItr unConstItr = unGraphs.begin();
  for (; unConstItr != unGraphs.end(); ++unConstItr)
    {
    JSONNode edges(JSON_NODE);
    edges.push_back(JSONNode("directed", "false"));
    edges.push_back(JSONNode("name", unConstItr->first));
    JSONNode links(JSON_ARRAY);
    links.set_name("links");

    // Skip master graph
    if (unConstItr->first.compare(vpMultiGraphModel::NoneDomain) == 0)
      {
      continue;
      }

    vtkAbstractArray* ids = unConstItr->second->GetEdgeData()->GetPedigreeIds();

    vtkVariantArray* parentAttrArrays
      = vtkVariantArray::SafeDownCast(
          unConstItr->second->GetEdgeData()->GetAbstractArray(
            "EdgeParentAttrs"));

    vtkVariantArray* childAttrArrays
      = vtkVariantArray::SafeDownCast(
          unConstItr->second->GetEdgeData()->GetAbstractArray(
            "EdgeChildAttrs"));

    unConstItr->second->GetEdges(egItr);
    while (egItr->HasNext())
      {
      JSONNode link(JSON_NODE);
      vtkEdgeType edgeType = egItr->Next();

      int edgeId = ids->GetVariantValue(edgeType.Id).ToInt();
      int parentId = nodeIds->GetVariantValue(edgeType.Source).ToInt();
      int childId = nodeIds->GetVariantValue(edgeType.Target).ToInt();

      link.push_back(JSONNode("id", edgeId));
      link.push_back(JSONNode("parent_id", parentId));
      // \TODO What is parent subtype??
      link.push_back(JSONNode("parent_subtype", ""));
      link.push_back(JSONNode("child_id", childId));
      // \TODO What is child subtype??
      link.push_back(JSONNode("child_subtype", ""));

      d->addEdgeAttributes(link, parentAttrArrays, childAttrArrays,
                           edgeType.Id);

      links.push_back(link);
      }

    edges.push_back(links);
    primitives.push_back(edges);
    }

  // Directed graphs
  DGraphs dGraphs = graphModel->GetAllDirectedGraphs();
  DConstItr dConstItr = dGraphs.begin();
  for (; dConstItr != dGraphs.end(); ++dConstItr)
    {
    JSONNode edges(JSON_NODE);
    edges.push_back(JSONNode("directed", "true"));
    edges.push_back(JSONNode("name", dConstItr->first));
    JSONNode links(JSON_ARRAY);
    links.set_name("links");

    vtkAbstractArray* ids = dConstItr->second->GetEdgeData()->GetPedigreeIds();

    vtkVariantArray* parentAttrArrays
      = vtkVariantArray::SafeDownCast(
          dConstItr->second->GetEdgeData()->GetAbstractArray(
            "EdgeParentAttrs"));

    vtkVariantArray* childAttrArrays
      = vtkVariantArray::SafeDownCast(
          dConstItr->second->GetEdgeData()->GetAbstractArray(
            "EdgeChildAttrs"));

    dConstItr->second->GetEdges(egItr);
    while (egItr->HasNext())
      {
      JSONNode link(JSON_NODE);
      vtkEdgeType edgeType = egItr->Next();

      int edgeId = ids->GetVariantValue(edgeType.Id).ToInt();
      int parentId = nodeIds->GetVariantValue(edgeType.Source).ToInt();
      int childId = nodeIds->GetVariantValue(edgeType.Target).ToInt();

      link.push_back(JSONNode("id", edgeId));
      link.push_back(JSONNode("parent_id", parentId));
      // \TODO What is parent subtype??
      link.push_back(JSONNode("parent_subtype", ""));
      link.push_back(JSONNode("child_id", childId));
      // \TODO What is child subtype??
      link.push_back(JSONNode("child_subtype", ""));

      d->addEdgeAttributes(link, parentAttrArrays, childAttrArrays,
                           edgeType.Id);

      links.push_back(link);
      }

    edges.push_back(links);
    primitives.push_back(edges);
    }

  root.push_back(primitives);
  return root;
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::saveState(JSONNode& root)
{
  try
    {
    root = this->exportToJson();
    }
  catch (const std::exception& e)
    {
    qDebug() << "Failed to save state:" << e.what();
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::saveUndoState()
{
  QTE_D(vpGraphModelWidget);

  this->saveState(d->UndoState);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::getWorldViewport(double (&viewport)[4],
                                          double offX, double offY)
{
  QTE_D(vpGraphModelWidget);

  int* size = d->RenderWindow->GetSize();
  double dsize[2] =
    { static_cast<double>(size[0]), static_cast<double>(size[1]) };

  double offsetX = dsize[0] * offX;
  double offsetY = dsize[1] * offY;

  double minPt[3] = { offsetX, offsetY, 0.0 };
  double maxPt[3] = { dsize[0] - offsetX, dsize[1] - offsetY, 0.0 };

  vtkVgSpaceConversion::DisplayToWorldNormalized(d->Renderer, minPt, minPt);
  vtkVgSpaceConversion::DisplayToWorldNormalized(d->Renderer, maxPt, maxPt);

  viewport[0] = minPt[0];
  viewport[1] = minPt[1];
  viewport[2] = maxPt[0];
  viewport[3] = maxPt[1];
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::resetCamera()
{
  QTE_D(vpGraphModelWidget);

  // Because the graph mapper uses vtkDistanceToCamera, there is a cyclic
  // dependency between the bounds of the graph and the zoom level of the
  // camera when performing a reset. It takes a few iterations for the camera
  // to converge to the right zoom level.
  vgRange<double> xExtents, yExtents;
  for (int i = 0; i < 5; ++i)
    {
    d->View.getGraphBounds(xExtents, yExtents);
    d->Renderer->ResetCamera(xExtents.lower, xExtents.upper,
                             yExtents.lower, yExtents.upper,
                             -1.0, 1.0);
    }
  d->View.render();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::clear()
{
  QTE_D(vpGraphModelWidget);

  d->Helper.clear();

  d->UI.nodeTreeWidget->clear();
  d->UI.edgeTreeWidget->clear();
  d->UI.childNodesComboBox->clear();
  d->UI.parentNodesComboBox->clear();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::selectEvent(int id)
{
  QTE_D(vpGraphModelWidget);

  if (d->Selecting)
    {
    return;
    }

  if (id == -1)
    {
    d->UI.nodeTreeWidget->clearSelection();
    return;
    }

  // Find the tree item corresponding to the given event, if any
  for (int i = 0, end = d->UI.nodeTreeWidget->topLevelItemCount(); i != end;
       ++i)
    {
    QTreeWidgetItem* item = d->UI.nodeTreeWidget->topLevelItem(i);
    if (d->Helper.getNodeEventId(item->data(0, NodeIdRole).toInt()) ==
        id)
      {
      d->UI.nodeTreeWidget->setCurrentItem(item);
      d->UI.nodeTreeWidget->scrollTo(d->UI.nodeTreeWidget->currentIndex());
      return;
      }
    }

  // No item found
  d->UI.nodeTreeWidget->clearSelection();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::resetPrimitives()
{
  QTE_D(vpGraphModelWidget);

  d->UI.primitivesComboBox->clear();

  // "None" is required to see nodes
  d->UI.primitivesComboBox->addItem("None");
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::enableCreateNode(bool flag)
{
  QTE_D(vpGraphModelWidget);

  if (flag && d->UI.addNodeButton->isEnabled())
    {
    // Switch to user-positioned layout if can't place user nodes in this layout
    if (d->UI.graphLayoutComboBox->currentIndex() ==
        vpMultiGraphRepresentation::SortByStartTime)
      {
      d->UI.graphLayoutComboBox->setCurrentIndex(
        vpMultiGraphRepresentation::Default);
      }
    d->View.setCreateNodeEnabled(flag);
    d->SpatialView.setCreateNodeEnabled(flag);
    return;
    }

  d->View.setCreateNodeEnabled(flag);
  d->SpatialView.setCreateNodeEnabled(flag);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::enableCreateEdge(bool /*flag*/)
{
  //QTE_D(vpGraphModelWidget);

  // FIXME
  //if (flag && d->UI.addEdgeButton->isEnabled())
  //  {
  //  d->Viewer.enableCreateEdge(flag);

  //  return;
  //  }

  //d->Viewer.enableCreateEdge(flag);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::createEdge()
{
  QTE_D(vpGraphModelWidget);

  int parentId
    = d->UI.parentNodesComboBox->itemData(
        d->UI.parentNodesComboBox->currentIndex()).toInt();

  int childId
    = d->UI.childNodesComboBox->itemData(
        d->UI.childNodesComboBox->currentIndex()).toInt();

  this->createEdge(parentId, childId);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::createNode(double x, double y)
{
  QTE_D(vpGraphModelWidget);

  this->saveUndoState();
  int id = d->Helper.createNode(x, y);

  // Compute and set spatial positions if in that layout
  if (d->View.getGraphLayout() == vpMultiGraphRepresentation::Spatial)
    {
    d->Helper.setNodePosition(id, vpMultiGraphModel::NPT_NormalizedSpatial,
                              x, y);

    double pt[3] = { x, y, 1.0 };
    d->View.getGraphToSpatialMatrix()->MultiplyPoint(pt, pt);
    d->Helper.setNodePosition(id, vpMultiGraphModel::NPT_Spatial, pt[0], pt[1]);
    }

  this->updateViews();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::createNodeSpatial(double x, double y)
{
  QTE_D(vpGraphModelWidget);

  this->saveUndoState();

  double wx, wy;
  d->SpatialView.getWorldPosition(x, y, wx, wy);

  // TODO: Give the node a sensible position for user layout
  int id = d->Helper.createNode(x, y);

  d->Helper.setNodePosition(id, vpMultiGraphModel::NPT_Spatial, x, y);

  if (d->View.getGraphLayout() == vpMultiGraphRepresentation::Spatial)
    {
    vtkSmartPointer<vtkMatrix3x3> spatialToGraph =
      vtkSmartPointer<vtkMatrix3x3>::New();;
    vtkMatrix3x3::Invert(d->View.getGraphToSpatialMatrix(), spatialToGraph);

    double pt[3] = { x, y, 1.0 };
    spatialToGraph->MultiplyPoint(pt, pt);

    d->Helper.setNodePosition(id, vpMultiGraphModel::NPT_NormalizedSpatial,
                              pt[0], pt[1]);
    }

  this->updateViews();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::createEdge(int parentId, int childId)
{
  QTE_D(vpGraphModelWidget);

  this->saveUndoState();
  const QString& domain = d->UI.primitivesComboBox->currentText();
  d->Helper.createEdge(parentId, childId, stdString(domain), 0, 0);
  this->updateViews();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::removeNodes()
{
  QTE_D(vpGraphModelWidget);

  vtkSmartPointer<vtkIdTypeArray> ids
    = vtkSmartPointer<vtkIdTypeArray>::New();

  foreach (QTreeWidgetItem* item, d->UI.nodeTreeWidget->selectedItems())
    {
    int id = item->data(0, NodeIdRole).toInt();
    this->deleteItemsForNode(id);
    ids->InsertNextValue(id);
    }

  if (ids->GetNumberOfTuples() != 0)
    {
    this->saveUndoState();
    d->Helper.deleteNodes(ids);
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::removeEdges()
{
  QTE_D(vpGraphModelWidget);

  vtkSmartPointer<vtkIdTypeArray> ids
    = vtkSmartPointer<vtkIdTypeArray>::New();

  QString curDomain;
  foreach (QTreeWidgetItem* item, d->UI.edgeTreeWidget->selectedItems())
    {
    int id = item->data(0, EdgeIdRole).toInt();
    QString domain = item->data(0, EdgeDomainRole).toString();

    if (curDomain.isNull())
      {
      curDomain = domain;
      }
    else if (domain != curDomain)
      {
      d->Helper.deleteEdges(ids, curDomain);
      curDomain = domain;
      ids->Reset();
      }

    this->deleteItemsForEdge(domain, id);
    ids->InsertNextValue(id);
    }

  if (ids->GetNumberOfTuples() != 0)
    {
    this->saveUndoState();
    d->Helper.deleteEdges(ids, curDomain);
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::updateNodeListView(
  QString nodeType, int nodeId)
{
  QTE_D(vpGraphModelWidget);

  QString nodeLabel =
    qtString(d->GraphModel->GetNodeLabel(nodeId));

  vpAttributeConfig::vpAttributeType defaultAttrType;

  int attr =
    d->AttributeConfig.getAttributeTypeIndex(
      d->GraphModel->GetNodeDefaultEdgeAttr(nodeId));

  if (attr != -1)
    {
    defaultAttrType = d->AttributeConfig.getAttributeTypeByIndex(attr);
    }

  QStringList cols;
  cols << nodeLabel;
  cols << nodeType;
  cols << defaultAttrType.Name;

  QTreeWidgetItem* item = new QTreeWidgetItem(cols);
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  item->setData(0, NodeIdRole, nodeId);
  item->setData(1, Qt::UserRole,
                vpEventConfig::GetIdFromString(qPrintable(nodeType)));
  item->setData(2, Qt::UserRole, defaultAttrType.Id);

  d->UI.nodeTreeWidget->addTopLevelItem(item);

  d->UI.parentNodesComboBox->addItem(nodeLabel, nodeId);
  d->UI.childNodesComboBox->addItem(nodeLabel, nodeId);

  if (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
    {
    d->UI.addNodeButton->setChecked(false);
    }

  // Since we cannot hide any item, hence safe to use count
  this->enableCreateEdge((d->UI.nodeTreeWidget->topLevelItemCount() > 1));
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::updateEdgeListView(
  QString domain, int edgeId)
{
  QTE_D(vpGraphModelWidget);

  vtkIntArray* parentAttrs =
      d->GraphModel->GetEdgeParentAttributes(edgeId, stdString(domain));
  vtkIntArray* childAttrs =
      d->GraphModel->GetEdgeChildAttributes(edgeId, stdString(domain));

  int parentAttrId = -1;
  QString parentAttribute;
  if (parentAttrs && parentAttrs->GetNumberOfTuples() != 0)
    {
    int idx
      = d->AttributeConfig.getAttributeTypeIndex(parentAttrs->GetValue(0));
    if (idx != -1)
      {
      vpAttributeConfig::vpAttributeType type
        = d->AttributeConfig.getAttributeTypeByIndex(idx);
      parentAttrId = type.Id;
      parentAttribute = type.Name;
      }
    }

  int childAttrId = -1;
  QString childAttribute;
  if (childAttrs && childAttrs->GetNumberOfTuples() != 0)
    {
    int idx
      = d->AttributeConfig.getAttributeTypeIndex(childAttrs->GetValue(0));
    if (idx != -1)
      {
      vpAttributeConfig::vpAttributeType type
        = d->AttributeConfig.getAttributeTypeByIndex(idx);
      childAttrId = type.Id;
      childAttribute = type.Name;
      }
    }

  QStringList cols;
  // \TODO Revisit this string formatting
  cols << QString(domain + "_%1").arg(edgeId);
  cols << parentAttribute;
  cols << childAttribute;
  QTreeWidgetItem* item = new QTreeWidgetItem(cols);
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  item->setData(0, EdgeIdRole, edgeId);
  item->setData(0, EdgeDomainRole, domain);
  item->setData(1, Qt::UserRole, parentAttrId);
  item->setData(2, Qt::UserRole, childAttrId);
  d->UI.edgeTreeWidget->addTopLevelItem(item);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::updateNodeItem(QTreeWidgetItem* item, int col)
{
  QTE_D(vpGraphModelWidget);

  if (col > 2)
    {
    return;
    }

  QString label = item->text(0);
  QString type = item->text(1);

  int id = item->data(0, NodeIdRole).toInt();
  int defaultEdgeAttr = item->data(2, Qt::UserRole).toInt();

  this->saveUndoState();
  d->Helper.setNodeType(id, stdString(type), stdString(label), defaultEdgeAttr);

  setComboItemText(d->UI.parentNodesComboBox, id, label);
  setComboItemText(d->UI.childNodesComboBox, id, label);

  // Update layouts in case node positions have changed due to a change in
  // default edge attribute.
  d->View.refreshGraphLayout();
  this->updateViews();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::updateEdgeItem(QTreeWidgetItem* item, int col)
{
  QTE_D(vpGraphModelWidget);

  if (col < 1 || col > 2)
    {
    return;
    }

  int attr = item->data(col, Qt::UserRole).toInt();

  vtkSmartPointer<vtkIntArray> attrs;
  if (attr != -1)
    {
    attrs = vtkSmartPointer<vtkIntArray>::New();
    attrs->InsertNextValue(attr);
    }

  std::string domain = stdString(item->data(0, EdgeDomainRole).toString());
  int id = item->data(0, EdgeIdRole).toInt();

  this->saveUndoState();
  if (col == 1)
    {
    d->GraphModel->SetEdgeParentAttributes(id, domain, attrs);
    }
  else
    {
    d->GraphModel->SetEdgeChildAttributes(id, domain, attrs);
    }

  this->updateViews();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::undo()
{
  QTE_D(vpGraphModelWidget);

  if (!d->UndoState.empty())
    {
    this->saveState(d->RedoState);
    if (this->importFrom(d->UndoState))
      {
      this->updateViews();
      }
    else
      {
      d->RedoState.clear();
      }
    d->UndoState.clear();
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::redo()
{
  QTE_D(vpGraphModelWidget);

  if (!d->RedoState.empty())
    {
    this->saveState(d->UndoState);
    if (this->importFrom(d->RedoState))
      {
      this->updateViews();
      }
    else
      {
      d->UndoState.clear();
      }
    d->RedoState.clear();
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::deleteItemsForNode(int id)
{
  QTE_D(vpGraphModelWidget);

  for (int i = 0, end = d->UI.nodeTreeWidget->topLevelItemCount(); i != end;
       ++i)
    {
    QTreeWidgetItem* item = d->UI.nodeTreeWidget->topLevelItem(i);
    if (id == item->data(0, NodeIdRole).toInt())
      {
      d->UI.nodeTreeWidget->takeTopLevelItem(i);
      delete item;
      break;
      }
    }

  removeComboItem(d->UI.parentNodesComboBox, id);
  removeComboItem(d->UI.childNodesComboBox, id);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::deleteItemsForEdge(QString domain, int id)
{
  QTE_D(vpGraphModelWidget);

  for (int i = 0, end = d->UI.edgeTreeWidget->topLevelItemCount(); i != end;
       ++i)
    {
    QTreeWidgetItem* item = d->UI.edgeTreeWidget->topLevelItem(i);
    if (id == item->data(0, EdgeIdRole).toInt() &&
        domain == item->data(0, EdgeDomainRole).toString())
      {
      d->UI.edgeTreeWidget->takeTopLevelItem(i);
      delete item;
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::updateNodeControls()
{
  QTE_D(vpGraphModelWidget);

  int count = d->UI.nodeTreeWidget->selectedItems().count();
  d->UI.removeNodeButton->setEnabled(count != 0);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::updateEdgeControls()
{
  QTE_D(vpGraphModelWidget);

  int count = d->UI.edgeTreeWidget->selectedItems().count();
  d->UI.removeEdgeButton->setEnabled(count != 0);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::selectPickedNodes()
{
  QTE_D(vpGraphModelWidget);

  if (d->Selecting)
    {
    return;
    }

  vtkIdTypeArray* pickedIds = d->GraphModel->GetSelectedNodes();
  QItemSelection selection;
  for (int i = 0, end = d->UI.nodeTreeWidget->topLevelItemCount(); i != end;
       ++i)
    {
    QTreeWidgetItem* item = d->UI.nodeTreeWidget->topLevelItem(i);
    for (vtkIdType j = 0, end = pickedIds->GetNumberOfTuples(); j != end; ++j)
      {
      if (pickedIds->GetValue(j) == item->data(0, NodeIdRole).toInt())
        {
        QModelIndex idx = d->UI.nodeTreeWidget->model()->index(i, 0);
        selection.select(idx, idx);
        break;
        }
      }
    }

  d->UI.nodeTreeWidget->selectionModel()->select(
    selection,
    QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

  if (!selection.isEmpty())
    {
    d->UI.nodeTreeWidget->selectionModel()->setCurrentIndex(
      selection.front().topLeft(),
      QItemSelectionModel::Rows | QItemSelectionModel::NoUpdate);
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::selectPickedEdges(QString domain)
{
  QTE_D(vpGraphModelWidget);

  if (d->Selecting)
    {
    return;
    }

  vtkIdTypeArray* pickedIds =
    d->GraphModel->GetSelectedEdges(stdString(domain));

  QItemSelection selection;
  for (int i = 0, end = d->UI.edgeTreeWidget->topLevelItemCount(); i != end;
       ++i)
    {
    QTreeWidgetItem* item = d->UI.edgeTreeWidget->topLevelItem(i);
    for (vtkIdType j = 0, end = pickedIds->GetNumberOfTuples(); j != end; ++j)
      {
      if (pickedIds->GetValue(j) ==
          item->data(0, EdgeIdRole).toInt() &&
          domain == item->data(0, EdgeDomainRole).toString())
        {
        QModelIndex idx = d->UI.edgeTreeWidget->model()->index(i, 0);
        selection.select(idx, idx);
        break;
        }
      }
    }

  d->UI.edgeTreeWidget->selectionModel()->select(
    selection,
    QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

  if (!selection.isEmpty())
    {
    d->UI.edgeTreeWidget->selectionModel()->setCurrentIndex(
      selection.front().topLeft(),
      QItemSelectionModel::Rows | QItemSelectionModel::NoUpdate);
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::updateNodeSelection()
{
  QTE_D(vpGraphModelWidget);

  SelectionGuard sg(d);

  vtkSmartPointer<vtkIdTypeArray> ids
    = vtkSmartPointer<vtkIdTypeArray>::New();

  int eventId = -1;

  int count = 0;
  foreach (QTreeWidgetItem* item, d->UI.nodeTreeWidget->selectedItems())
    {
    ++count;
    int id = item->data(0, NodeIdRole).toInt();
    ids->InsertNextValue(id);
    if (eventId == -1)
      {
      eventId = d->Helper.getNodeEventId(id);
      }
    }

  if (count != 0)
    {
    d->UI.edgeTreeWidget->clearSelection();
    }

  this->updateNodeControls();

  d->Helper.selectNodes(ids);
  this->updateViews();

  // Emit a signal if a node linked to an event was selected
  if (eventId != -1)
    {
    emit this->eventSelected(eventId);
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::updateEdgeSelection()
{
  QTE_D(vpGraphModelWidget);

  SelectionGuard sg(d);

  vtkSmartPointer<vtkIdTypeArray> ids
    = vtkSmartPointer<vtkIdTypeArray>::New();

  QString curDomain = d->UI.primitivesComboBox->currentText();

  int count = 0;
  foreach (QTreeWidgetItem* item, d->UI.edgeTreeWidget->selectedItems())
    {
    if (item->data(0, EdgeDomainRole).toString() == curDomain)
      {
      ++count;
      int id = item->data(0, EdgeIdRole).toInt();
      ids->InsertNextValue(id);
      }
    }

  if (count != 0)
    {
    d->UI.nodeTreeWidget->clearSelection();
    }

  this->updateEdgeControls();

  d->Helper.selectEdges(ids, curDomain);
  this->updateViews();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::updateEdgeInterface()
{
  QTE_D(vpGraphModelWidget);

  QString curDomain = d->UI.primitivesComboBox->currentText();

  if ((curDomain.compare("before") == 0) ||
      (curDomain.compare("adjacent") == 0))
    {
    d->UI.autoEdgesButton->setEnabled(true);
    d->UI.autoEdgesCheckBox->setEnabled(true);
    }
  else
    {
    d->UI.autoEdgesButton->setEnabled(false);
    d->UI.autoEdgesCheckBox->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::updateParameterWidget(int primitive)
{
  QTE_D(vpGraphModelWidget);

  if (--primitive < 0)
    {
    d->UI.parameterWidget->setVisible(false);
    return;
    }

  int prevType = d->PrimitiveConfig.getPreviousOrderedTypeIndex(primitive);
  double minimum =
    prevType != -1
      ? d->UI.primitivesComboBox->itemData(prevType + 1).toDouble()
      : 0.0;

  int nextType = d->PrimitiveConfig.getNextOrderedTypeIndex(primitive);
  double maximum =
    nextType != -1
      ? d->UI.primitivesComboBox->itemData(nextType + 1).toDouble()
      : 1e9;

  vpPrimitiveConfig::vpPrimitiveType pt =
    d->PrimitiveConfig.getPrimitiveTypeByIndex(primitive);
  switch (pt.ParamType)
    {
    case vpPrimitiveConfig::PPT_Distance:
      d->UI.parameterWidget->setCurrentIndex(0);
      d->UI.parameterWidget->setVisible(true);
      d->UI.distanceParameter->setValue(
        d->UI.primitivesComboBox->itemData(primitive + 1).toDouble());
      d->UI.distanceParameter->setRange(minimum, maximum);
      break;
    case vpPrimitiveConfig::PPT_Time:
      d->UI.parameterWidget->setCurrentIndex(1);
      d->UI.parameterWidget->setVisible(true);
      d->UI.timeParameter->setValue(
        d->UI.primitivesComboBox->itemData(primitive + 1).toDouble());
      d->UI.timeParameter->setRange(minimum, maximum);
      break;
    default:
      d->UI.parameterWidget->setVisible(false);
      break;
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::pruneEdges()
{
  QTE_D(vpGraphModelWidget);

  vpMultiGraphModel* gm = d->GraphModel;
  for (int i = 0; i != d->UI.edgeTreeWidget->topLevelItemCount();)
    {
    QTreeWidgetItem* item = d->UI.edgeTreeWidget->topLevelItem(i);
    int id = item->data(0, EdgeIdRole).toInt();
    QString domain = item->data(0, EdgeDomainRole).toString();
    if (!gm->HasEdge(id, stdString(domain)))
      {
      delete d->UI.edgeTreeWidget->takeTopLevelItem(i);
      }
    else
      {
      ++i;
      }
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::changeGraphLayout(int layoutIndex)
{
  QTE_D(vpGraphModelWidget);
  double viewport[4];
  this->getWorldViewport(viewport, 0.20, 0.20);

  vgRange<double> width(viewport[0], viewport[2]);
  vgRange<double> height(viewport[1], viewport[3]);
  d->View.setGraphLayout(layoutIndex, width, height);
  d->View.render();

  if (layoutIndex == vpMultiGraphRepresentation::SortByStartTime)
    {
    d->UI.addNodeButton->setChecked(false);
    }
  d->UI.copyCurrentLayoutButton->setEnabled(
    layoutIndex != vpMultiGraphRepresentation::Default);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::copyCurrentLayoutToDefault()
{
  QTE_D(vpGraphModelWidget);

  d->View.copyCurrentLayout();
  d->UI.graphLayoutComboBox->setCurrentIndex(
    vpMultiGraphRepresentation::Default);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::autoCreateEdges()
{
  QTE_D(vpGraphModelWidget);

  QString curDomain = d->UI.primitivesComboBox->currentText();
  d->Helper.autoCreateEdges(curDomain, d->UI.autoEdgesCheckBox->isChecked(),
                            d->View.getNodePositionType());
  this->updateViews();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::setVertexOpacity(int value)
{
  QTE_D(vpGraphModelWidget);

  double opacity = 0.25 + 0.75 * static_cast<double>(value) /
                                 d->UI.vertexOpacity->maximum();
  d->View.setVertexOpacity(opacity);
  d->SpatialView.setVertexOpacity(opacity);
  this->updateViews();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::showMainWindowOverlay(bool show)
{
  QTE_D(vpGraphModelWidget);

  if (show)
    {
    d->SpatialView.setGraphLayout(vpMultiGraphRepresentation::RawSpatial,
                                  vgRange<double>(), vgRange<double>());
    }
  d->SpatialView.setActive(show);
  d->SpatialView.render();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::updateMovedNodeSpatialPositions()
{
  QTE_D(vpGraphModelWidget);

  if (d->View.getGraphLayout() != vpMultiGraphRepresentation::Spatial)
    {
    return;
    }

  vtkSmartPointer<vtkMatrix3x3> spatialToGraph =
    vtkSmartPointer<vtkMatrix3x3>::New();
  vtkMatrix3x3::Invert(d->View.getGraphToSpatialMatrix(), spatialToGraph);

  // Here we assume that only selected nodes have been moved
  vtkIdTypeArray* nodes = d->GraphModel->GetSelectedNodes();
  for (vtkIdType i = 0, end = nodes->GetNumberOfTuples(); i < end; ++i)
    {
    int id = static_cast<int>(nodes->GetValue(i));
    double pt[3];
    d->GraphModel->GetNodePosition(vpMultiGraphModel::NPT_Spatial, id,
                                   pt[0], pt[1], pt[2]);
    pt[2] = 1.0;

    // Set the "normalized" spatial position for the widget graph view based on
    // the world position of the node that was just set via a drag in the graph
    // overlay.
    spatialToGraph->MultiplyPoint(pt, pt);
    d->Helper.setNodePosition(id, vpMultiGraphModel::NPT_NormalizedSpatial,
                              pt[0], pt[1]);
    }

  this->updateViews();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::setPrimitiveParameter(double value)
{
  QTE_D(vpGraphModelWidget);

  d->UI.primitivesComboBox->setItemData(
    d->UI.primitivesComboBox->currentIndex(), value);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::pickDistance()
{
  QTE_D(vpGraphModelWidget);

  if (this->sender() == d->UI.pickMaxSpatialWindow)
    {
    d->PickingMaxSpatialWindow = true;
    }

  QMessageBox *mb =
    new QMessageBox(QMessageBox::NoIcon, QString(),
                    "Select desired distance by clicking and dragging in the "
                    "main window.", QMessageBox::Cancel);

  connect(mb, SIGNAL(finished(int)), this, SLOT(cancelPickDistance()));
  connect(this, SIGNAL(distanceMeasurementComplete()), mb, SLOT(accept()));

  mb->setAttribute(Qt::WA_DeleteOnClose);
  mb->setModal(false);
  mb->show();

  // Disable input to the rest of the widget. Normally we would just use a
  // modal dialog, but can't because there is no such thing as "widget modal".
  this->setEnabled(false);

  emit this->distanceMeasurementRequested(true);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::cancelPickDistance()
{
  QTE_D(vpGraphModelWidget);

  d->PickingMaxSpatialWindow = false;
  this->setEnabled(true);

  emit this->distanceMeasurementRequested(false);
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::setMeasuredDistance(double meters)
{
  QTE_D(vpGraphModelWidget);

  if (meters < 0.0)
    {
    QMessageBox::warning(this, QString(),
                         "Failed to get valid distance. Make sure the project "
                         "is loaded with geographic coordinates enabled.");

    emit this->distanceMeasurementComplete();
    return;
    }

  if (d->PickingMaxSpatialWindow)
    {
    d->UI.maxSpatialWindow->setValue(meters);

    emit this->distanceMeasurementComplete();
    return;
    }

  this->setParameterValue(meters, d->UI.distanceParameter);

  emit this->distanceMeasurementComplete();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::pickTimeInterval()
{
  QTE_D(vpGraphModelWidget);

  this->setEnabled(false);

  if (this->sender() == d->UI.pickMaxTemporalWindow)
    {
    d->PickingMaxTemporalWindow = true;
    }

  emit this->timeIntervalMeasurementRequested();
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::setMeasuredTimeInterval(double seconds)
{
  QTE_D(vpGraphModelWidget);

  this->setEnabled(true);
  if (seconds >= 0.0)
    {
    if (d->PickingMaxTemporalWindow)
      {
      d->UI.maxTemporalWindow->setValue(qCeil(seconds));
      }
    else
      {
      this->setParameterValue(seconds, d->UI.timeParameter);
      }
    }
  d->PickingMaxTemporalWindow = false;
}

//-----------------------------------------------------------------------------
void vpGraphModelWidget::setParameterValue(double value, QDoubleSpinBox* widget)
{
  QTE_D(vpGraphModelWidget);

  // Sanity check the new value against constraints imposed by other types
  int currentType = d->UI.primitivesComboBox->currentIndex() - 1;
  int prevType = d->PrimitiveConfig.getPreviousOrderedTypeIndex(currentType);
  int nextType = d->PrimitiveConfig.getNextOrderedTypeIndex(currentType);

  if (prevType >= 0 &&
      value < d->UI.primitivesComboBox->itemData(prevType + 1).toDouble())
    {
    double prevVal =
      d->UI.primitivesComboBox->itemData(prevType + 1).toDouble();

    QMessageBox::warning(
      this, QString(),
      QString("Attempted to set parameter to a lower value than that of "
              "the previous ordered primitive, \"%1\" ( %2 ). Parameter "
              "value will be clamped.")
        .arg(d->PrimitiveConfig.getPrimitiveTypeByIndex(prevType).Name)
        .arg(prevVal));
    value = prevVal;
    }

  if (nextType >= 0 &&
      value > d->UI.primitivesComboBox->itemData(nextType + 1).toDouble())
    {
    double nextVal =
      d->UI.primitivesComboBox->itemData(nextType + 1).toDouble();

    QMessageBox::warning(
      this, QString(),
      QString("Attempted to set parameter to a higher value than that of "
              "the next ordered primitive, \"%1\" ( %2 ). Parameter value "
              "will be clamped.")
        .arg(d->PrimitiveConfig.getPrimitiveTypeByIndex(nextType).Name)
        .arg(nextVal));
    value = nextVal;
    }

  widget->setValue(value);
}
