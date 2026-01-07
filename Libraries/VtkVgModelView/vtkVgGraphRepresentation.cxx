// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgGraphRepresentation.h"

#include <vtkVgActivity.h>
#include <vtkVgActivityTypeRegistry.h>

#include "vtkVgActivityManager.h"
#include "vtkVgEvent.h"
#include "vtkVgEventFilter.h"
#include "vtkVgEventModel.h"
#include "vtkVgEventTypeRegistry.h"
#include "vtkVgGraphModel.h"
#include "vtkVgPicker.h"

#include <vgActivityType.h>
#include <vgEventType.h>

#include <vtkVgCellPicker.h>
#include <vtkVgColorUtil.h>
#include <vtkVgGraphMapper.h>

#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkCubeSource.h>
#include <vtkDataSet.h>
#include <vtkDataSetAttributes.h>
#include <vtkDoubleArray.h>
#include <vtkEdgeListIterator.h>
#include <vtkGlyph3D.h>
#include <vtkGlyphSource2D.h>
#include <vtkGraph.h>
#include <vtkGraphToPolyData.h>
#include <vtkInEdgeIterator.h>
#include <vtkIdListCollection.h>
#include <vtkIdTypeArray.h>
#include <vtkObjectFactory.h>
#include <vtkOutEdgeIterator.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkSphereSource.h>

// C++ includes
#include <cassert>
#include <map>

vtkStandardNewMacro(vtkVgGraphRepresentation);
vtkCxxSetObjectMacro(vtkVgGraphRepresentation, EventTypeRegistry,
                     vtkVgEventTypeRegistry);

#define vtkCreateMacro(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//----------------------------------------------------------------------------
struct vtkVgGraphRepresentation::vtkInternal
{
  vtkInternal();
  ~vtkInternal();

  vtkSmartPointer<vtkActor>       GraphActor;

  vtkSmartPointer<vtkDoubleArray> ScaleArray;
  vtkSmartPointer<vtkUnsignedCharArray>
  VertexColorArray;

  vtkSmartPointer<vtkUnsignedCharArray>
  EdgeColorArray;
  vtkSmartPointer<vtkIdTypeArray> GeometryIndexArray;

  vtkSmartPointer<vtkPolyData>    TrackGeometry;
  vtkSmartPointer<vtkPolyData>    EventGeometry;
  vtkSmartPointer<vtkPolyData>    ActivityGeometry;

  static std::string              EdgeColorModeStrings
  [vtkVgGraphRepresentation::CountEdgeColorModes];

  static std::string              EdgeThicknessModeStrings
  [vtkVgGraphRepresentation::CountEdgeThicknessModes];

  inline void SetColor(vtkUnsignedCharArray* array,
                       vtkIdType id, double rgba[4]);

  inline void SetVertexColor(vtkIdType id, double rgba[4]);
  inline void SetVertexScale(vtkIdType id, double scale);
  inline void SetVertexGeometryIndex(vtkIdType id, vtkIdType value);

  inline void SetEdgeColor(vtkIdType id, double rgba[4]);

  static std::string GetGraphEdgeColorModeString
  (vtkIdType index);
  static std::string GetGraphEdgeThicknessModeString
  (vtkIdType index);
};

//-----------------------------------------------------------------------------
std::string vtkVgGraphRepresentation::vtkInternal::EdgeColorModeStrings
[vtkVgGraphRepresentation::CountEdgeColorModes] =
{
  "Default",
  "RoleBased"
};

//-----------------------------------------------------------------------------
std::string vtkVgGraphRepresentation::vtkInternal::EdgeThicknessModeStrings
[vtkVgGraphRepresentation::CountEdgeThicknessModes] =
{
  "Thin",
  "Thick",
  "Thicker"
};

//-----------------------------------------------------------------------------
vtkVgGraphRepresentation::vtkInternal::vtkInternal()
{
  this->TrackGeometry     = vtkSmartPointer<vtkPolyData>::New();
  this->EventGeometry     = vtkSmartPointer<vtkPolyData>::New();
  this->ActivityGeometry  = vtkSmartPointer<vtkPolyData>::New();

  // Currently track and event are represented as spheres.
  // Create geometric representation of nodes.
  vtkSmartPointer<vtkSphereSource> sphereSource(vtkSmartPointer<vtkSphereSource>::New());
  sphereSource->SetRadius(10.0);
  sphereSource->Update();

  this->TrackGeometry->DeepCopy(sphereSource->GetOutput());
  this->EventGeometry->DeepCopy(sphereSource->GetOutput());

  // Activities are represented as cones.
  vtkSmartPointer<vtkCubeSource> cubeSource(vtkSmartPointer<vtkCubeSource>::New());
  cubeSource->SetXLength(20.0);
  cubeSource->SetYLength(20.0);
  cubeSource->SetZLength(20.0);
  cubeSource->Update();

  this->ActivityGeometry->DeepCopy(cubeSource->GetOutput());
}

//-----------------------------------------------------------------------------
vtkVgGraphRepresentation::vtkInternal::~vtkInternal()
{
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::vtkInternal::SetColor(vtkUnsignedCharArray* array,
    vtkIdType id, double rgba[4])
{
  unsigned char c[4];
  vtkVgColorUtil::convert(rgba, c);
  array->SetTypedTuple(id, c);
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::vtkInternal::SetVertexColor(vtkIdType id, double rgba[4])
{
  this->SetColor(this->VertexColorArray, id, rgba);
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::vtkInternal::SetVertexScale(vtkIdType id, double scale)
{
  // \note: No null pointer check for performance reasons.
  this->ScaleArray->SetValue(id, scale);
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::vtkInternal::SetVertexGeometryIndex(vtkIdType id, vtkIdType value)
{
  this->GeometryIndexArray->SetValue(id, value);
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::vtkInternal::SetEdgeColor(vtkIdType id, double rgba[4])
{
  this->SetColor(this->EdgeColorArray, id, rgba);
}

//-----------------------------------------------------------------------------
std::string vtkVgGraphRepresentation::vtkInternal::GetGraphEdgeColorModeString(
  vtkIdType index)
{
  const std::string emptyString("");
  if (index < 0 || index >= vtkVgGraphRepresentation::CountEdgeColorModes)
    {
    return emptyString;
    }
  else
    {
    return vtkInternal::EdgeColorModeStrings[index];
    }
}

//-----------------------------------------------------------------------------
std::string vtkVgGraphRepresentation::vtkInternal::GetGraphEdgeThicknessModeString(
  vtkIdType index)
{
  const std::string emptyString("");
  if (index < 0 || index >= vtkVgGraphRepresentation::CountEdgeThicknessModes)
    {
    return emptyString;
    }
  else
    {
    return vtkInternal::EdgeThicknessModeStrings[index];
    }
}

//-----------------------------------------------------------------------------
vtkVgGraphRepresentation::vtkVgGraphRepresentation()
{
  this->Visible = 1;

  this->GraphEdgeColorMode      = DefaultEdgeColor;

  this->GraphEdgeThicknessMode  = ThinEdge;

  this->GraphModel = 0;

  this->Internal = new vtkInternal;

  this->Internal->GraphActor  = vtkSmartPointer<vtkActor>::New();

  this->NewPropCollection    = vtkPropCollectionRef::New();
  this->ActivePropCollection = vtkPropCollectionRef::New();
  this->ExpirePropCollection = vtkPropCollectionRef::New();

  this->NewPropCollection->AddItem(this->Internal->GraphActor);
  this->ActivePropCollection->AddItem(this->Internal->GraphActor);

  this->Picker = vtkSmartPointer<vtkVgCellPicker>::New();
  this->Picker->PickFromListOn();
  this->Picker->SetTolerance(0.01);
  this->PickPosition[0] = this->PickPosition[1] = this->PickPosition[2] = 0.0;

  this->EventTypeRegistry = 0;
}

//-----------------------------------------------------------------------------
vtkVgGraphRepresentation::~vtkVgGraphRepresentation()
{
  this->SetGraphModel(0);

  this->SetEventTypeRegistry(0);

  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::SetGraphModel(vtkVgGraphModel* graphModel)
{
  if (this->GraphModel != graphModel)
    {
    vtkVgGraphModel* temp = this->GraphModel;
    this->GraphModel = graphModel;
    if (this->GraphModel)
      {
      this->GraphModel->Register(this);
      }
    if (temp)
      {
      temp->UnRegister(this);
      }

    if (this->GraphModel)
      {
      if (this->UseAutoUpdate)
        {
        this->GraphModel->AddObserver(vtkCommand::UpdateDataEvent, this,
                                      &vtkVgGraphRepresentation::Update);
        }
      }

    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgGraphRepresentation::GetNewRenderObjects() const
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgGraphRepresentation::GetActiveRenderObjects() const
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgGraphRepresentation::GetExpiredRenderObjects() const
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgGraphRepresentation::GetNewRenderObjects()
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgGraphRepresentation::GetActiveRenderObjects()
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgGraphRepresentation::GetExpiredRenderObjects()
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::ResetTemporaryRenderObjects()
{
  this->NewPropCollection->RemoveAllItems();
  this->ExpirePropCollection->RemoveAllItems();
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::SetVisible(int flag)
{
  if (flag == this->Visible)
    {
    return;
    }

  this->Visible = flag;
  this->ActivePropCollection->InitTraversal();
  while (vtkProp* prop = this->ActivePropCollection->GetNextProp())
    {
    prop->SetVisibility(flag);
    }

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::Initialize()
{
  if (!this->GraphModel)
    {
    vtkErrorMacro("Event model must be set before initialization.");
    return;
    }

  vtkGraph* graph = this->GraphModel->GetGraph();
  vtkIdType numVerts = graph->GetNumberOfVertices();
  vtkIdType numEdges = graph->GetNumberOfEdges();

  this->Internal->ScaleArray  = vtkSmartPointer<vtkDoubleArray>::New();
  this->Internal->ScaleArray->SetName("vertex_scale");
  this->Internal->ScaleArray->SetNumberOfTuples(numVerts);
  this->Internal->ScaleArray->SetNumberOfComponents(1);

  this->Internal->VertexColorArray = vtkSmartPointer<vtkUnsignedCharArray>::New();
  this->Internal->VertexColorArray->SetName("vertex_color");
  this->Internal->VertexColorArray->SetNumberOfComponents(4);
  this->Internal->VertexColorArray->SetNumberOfTuples(numVerts);

  this->Internal->GeometryIndexArray = vtkSmartPointer<vtkIdTypeArray>::New();
  this->Internal->GeometryIndexArray->SetName("geometry_index");
  this->Internal->GeometryIndexArray->SetNumberOfComponents(1);
  this->Internal->GeometryIndexArray->SetNumberOfTuples(numVerts);

  this->Internal->EdgeColorArray = vtkSmartPointer<vtkUnsignedCharArray>::New();
  this->Internal->EdgeColorArray->SetName("edge_color");
  this->Internal->EdgeColorArray->SetNumberOfComponents(4);
  this->Internal->EdgeColorArray->SetNumberOfTuples(numEdges);

  // Color
  double rgba[4] = {0.7, 0.7, 0.7, 0.7};

  // Tracks.
  const std::map<vtkIdType, vtkIdType> tracksVerticesMap =
    this->GraphModel->GetTracksVerticesMap();
  std::map<vtkIdType, vtkIdType>::const_iterator tvConstItr =
    tracksVerticesMap.begin();

  while (tvConstItr != tracksVerticesMap.end())
    {
    this->Internal->SetVertexColor(tvConstItr->second, rgba);
    this->Internal->SetVertexScale(tvConstItr->second, 0.01);
    this->Internal->SetVertexGeometryIndex(tvConstItr->second, 0);

    ++tvConstItr;
    }

  // Events.
  const std::map<vtkIdType, vtkIdType> eventsVerticesMap =
    this->GraphModel->GetEventsVerticesMap();
  std::map<vtkIdType, vtkIdType>::const_iterator evConstItr =
    eventsVerticesMap.begin();

  while (evConstItr != eventsVerticesMap.end())
    {
    vtkVgEvent* event =
      this->GraphModel->GetEventModel()->GetEvent(evConstItr->first);

    if (event && this->EventTypeRegistry)
      {
      const vgEventType& et = this->EventTypeRegistry->GetTypeById(
                                event->GetActiveClassifierType());

      et.GetTrackColor(0, rgba[0], rgba[1], rgba[2]);

      double normalcyScale = 0.25 +
        (1 - event->GetNormalcy(event->GetActiveClassifierType())) * 0.75;

      this->Internal->SetVertexColor(evConstItr->second, rgba);
      this->Internal->SetVertexScale(evConstItr->second, normalcyScale);
      this->Internal->SetVertexGeometryIndex(evConstItr->second, 1);
      }

    ++evConstItr;
    }

  // Activities.
  vtkVgActivityTypeRegistry* activityTypeRegistry
  (this->GraphModel->GetActivityManager()->GetActivityTypeRegistry());
  const std::map<vtkIdType, vtkIdType> activitiesVerticesMap =
    this->GraphModel->GetActivitiesVerticesMap();
  std::map<vtkIdType, vtkIdType>::const_iterator avConstItr =
    activitiesVerticesMap.begin();

  // get range of saliency scores to compute a scale factor
  double maxSaliency = 0;
  while (avConstItr != activitiesVerticesMap.end())
    {
    vtkVgActivity* activity =
      this->GraphModel->GetActivityManager()->GetActivity(avConstItr->first);

    if (activity && activityTypeRegistry &&
        activity->GetSaliency() > maxSaliency)
      {
      maxSaliency = activity->GetSaliency();
      }

    ++avConstItr;
    }
  double scaleFactor = 2;
  if (maxSaliency > 1)
    {
    scaleFactor /= maxSaliency;
    }

  avConstItr = activitiesVerticesMap.begin();
  while (avConstItr != activitiesVerticesMap.end())
    {
    vtkVgActivity* activity =
      this->GraphModel->GetActivityManager()->GetActivity(avConstItr->first);

    if (activity && activityTypeRegistry)
      {
      int type = activity->GetType();
      const vgActivityType& at = activityTypeRegistry->GetType(type);
      at.GetColor(rgba[0], rgba[1], rgba[2]);

      this->Internal->SetVertexColor(avConstItr->second, rgba);
      this->Internal->SetVertexScale(avConstItr->second, 1.25 +
                                     scaleFactor * activity->GetSaliency());
      this->Internal->SetVertexGeometryIndex(avConstItr->second, 2);
      }

    ++avConstItr;
    }

  // Edges
  const std::map<vtkIdType, vtkIdType> edgeIdToEdgeTypeMap =
    this->GraphModel->GetEdgeIdToEdgeTypeMap();
  const std::map<vtkIdType, double> edgeProbabilityMap =
    this->GraphModel->GetEdgeProbabilityMap();
  std::map<vtkIdType, vtkIdType>::const_iterator edgeConstItr =
    edgeIdToEdgeTypeMap.begin();
  while (edgeConstItr != edgeIdToEdgeTypeMap.end())
    {
    if (edgeConstItr->second == 0)
      {
      double rgba[4] = {0.3, 0.6, 0.3, 0.5};
      this->Internal->SetEdgeColor(edgeConstItr->first, rgba);
      }
    else if (edgeConstItr->second == 1)
      {
      double rgba[4] = {0.3, 0.8, 0.8, 0.5};
      this->Internal->SetEdgeColor(edgeConstItr->first, rgba);
      }
    else if (edgeConstItr->second == 2)
      {
      // Look up probability value of link edge.
      const std::map<vtkIdType, double>::const_iterator itr
        = edgeProbabilityMap.find(edgeConstItr->first);
      double p = itr->second;
      p = p > 1.0 ? 1.0 : (p < 0.0 ? 0.0 : p);

      // Make low probability links darker than more likely ones.
      double rgba[] = { p, p, p, 0.5 };
      this->Internal->SetEdgeColor(edgeConstItr->first, rgba);
      }
    else
      {
      vtkErrorMacro("ERROR: Bad edge type " << edgeConstItr->second);
      }
    ++edgeConstItr;
    }

  assert(this->Internal->ScaleArray->GetNumberOfTuples()         == numVerts);
  assert(this->Internal->VertexColorArray->GetNumberOfTuples()   == numVerts);
  assert(this->Internal->GeometryIndexArray->GetNumberOfTuples() == numVerts);
  assert(this->Internal->EdgeColorArray->GetNumberOfTuples()     == numEdges);

  // Add scalar data to the graph.
  graph->GetVertexData()->AddArray(this->Internal->ScaleArray);
  graph->GetVertexData()->AddArray(this->Internal->VertexColorArray);
  graph->GetVertexData()->AddArray(this->Internal->GeometryIndexArray);
  graph->GetEdgeData()->AddArray(this->Internal->EdgeColorArray);

  vtkCreateMacro(vtkVgGraphMapper, mapper);
  mapper->SetInputData(graph);
  mapper->SetVertexGeometryIndexing(1);
  mapper->SetNumberOfVertexGeometrySources(3);
  mapper->SetVertexGeometry(0, this->Internal->TrackGeometry);
  mapper->SetVertexGeometry(1, this->Internal->EventGeometry);
  mapper->SetVertexGeometry(2, this->Internal->ActivityGeometry);
  mapper->SetVertexGeometryIndexArray(this->Internal->GeometryIndexArray->GetName());
  mapper->SetScalingArrayName(this->Internal->ScaleArray->GetName());
  mapper->SetScaledGlyphs(1);
  mapper->SetVertexPointSize(1.0f);
  mapper->SetVertexColorArrayName(this->Internal->VertexColorArray->GetName());
  mapper->SetColorVertices(true);

  this->ActivePropCollection->RemoveItem(this->Internal->GraphActor);
  this->NewPropCollection->RemoveAllItems();
  this->ExpirePropCollection->AddItem(this->Internal->GraphActor);

  this->Internal->GraphActor = vtkSmartPointer<vtkActor>::New();
  this->NewPropCollection->AddItem(this->Internal->GraphActor);

  this->Internal->GraphActor->SetMapper(mapper);
  this->Internal->GraphActor->PickableOn();

  if (numEdges)
    {
    this->SetGraphEdgeColorMode(this->GraphEdgeColorMode);
    this->SetGraphEdgeThicknessMode(this->GraphEdgeThicknessMode);
    }
  mapper->Update();
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::Update()
{

  if (!this->GetVisible())
    {
    return;
    }

  // \note: Currently call initialize every update as the graph model has
  // changed.
  this->Initialize();

  this->Internal->GraphActor->GetMapper()->Update();

  this->UpdateTime.Modified();
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgGraphRepresentation::
Pick(double renX, double renY, vtkRenderer* ren, vtkIdType& pickType)
{
  pickType = vtkVgPickData::EmptyPick;

  return -1;

  if (!this->GetVisible())
    {
    return -1;
    }

  // Set up the pick list and add the actors to it
  this->Picker->InitializePickList();

  this->ActivePropCollection->InitTraversal();
  while (vtkProp* prop = this->ActivePropCollection->GetNextProp())
    {
    this->Picker->AddPickList(prop);
    }

  if (this->Picker->Pick(renX, renY, 0.0, ren))
    {
    vtkIdType cellId = this->Picker->GetCellId();
    this->Picker->GetPickPosition(this->PickPosition);

    if (vtkIdTypeArray* da =
          dynamic_cast<vtkIdTypeArray*>(
            this->Picker->GetDataSet()->GetCellData()->GetArray("Event Ids")))
      {
      pickType = vtkVgPickData::PickedGraph;
      return da->GetValue(cellId);
      }
    }

  return -1;
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::SetGraphEdgeColorMode(int mode)
{
  if (mode < CountEdgeColorModes)
    {
    this->GraphEdgeColorMode = mode;
    }

  vtkVgGraphMapper* graphMapper =
    vtkVgGraphMapper::SafeDownCast(this->Internal->GraphActor->GetMapper());
  if (!graphMapper)
    {
    vtkErrorMacro("ERROR: Invalid graph mapper\n");
    return;
    }

  switch (mode)
    {
    case DefaultEdgeColor:
      {
      graphMapper->SetColorEdges(false);
      graphMapper->Update();
      break;
      }
    case RoleBasedEdgeColor:
      {
      vtkGraph* graph = this->GraphModel->GetGraph();
      if (graph && graph->GetNumberOfEdges()
        && this->Internal->EdgeColorArray->GetNumberOfTuples())
        {
        graphMapper->SetColorEdges(true);
        graphMapper->SetEdgeColorArrayName(
          this->Internal->EdgeColorArray->GetName());

        this->SetGraphEdgeThicknessMode(this->GraphEdgeThicknessMode);

        graphMapper->Update();
        }
      break;
      }
    default:
      {
      vtkErrorMacro("ERROR: Unknown edge color mode\n");
      break;
      }
    }
}

//-----------------------------------------------------------------------------
int vtkVgGraphRepresentation::GetGraphEdgeColorMode()
{
  return this->GraphEdgeColorMode;
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::SetGraphEdgeThicknessMode(int mode)
{
  if (mode < CountEdgeThicknessModes)
    {
    this->GraphEdgeThicknessMode = mode;
    }

  vtkVgGraphMapper* graphMapper =
    vtkVgGraphMapper::SafeDownCast(this->Internal->GraphActor->GetMapper());
  if (!graphMapper)
    {
    vtkErrorMacro("ERROR: Invalid graph mapper\n");
    return;
    }

  switch (mode)
    {
    case ThinEdge:
      {
      graphMapper->SetEdgeLineWidth(1.0f);
      this->SetGraphEdgeOpacity(0.5);
      break;
      }
    case ThickEdge:
      {
      graphMapper->SetEdgeLineWidth(2.0f);
      this->SetGraphEdgeOpacity(0.7);
      break;
      }
    case ThickerEdge:
      {
      graphMapper->SetEdgeLineWidth(4.0f);
      this->SetGraphEdgeOpacity(1.0);
      break;
      }
    default:
      {
      vtkErrorMacro("ERROR: Unknown edge thickness mode\n");
      break;
      }
    }
}

//-----------------------------------------------------------------------------
int vtkVgGraphRepresentation::GetGraphEdgeThicknessMode()
{
  return this->GraphEdgeThicknessMode;
}

//-----------------------------------------------------------------------------
void vtkVgGraphRepresentation::SetGraphEdgeOpacity(double value)
{
  vtkVgGraphMapper* graphMapper =
    vtkVgGraphMapper::SafeDownCast(this->Internal->GraphActor->GetMapper());
  if (!graphMapper)
    {
    vtkErrorMacro("ERROR: Invalid graph mapper\n");
    return;
    }

  if (this->GraphEdgeColorMode == DefaultEdgeColor)
    {
    graphMapper->SetEdgeOpacity(value);
    }
  else if (this->GraphEdgeColorMode == RoleBasedEdgeColor)
    {
    vtkIdType count = this->Internal->EdgeColorArray->GetNumberOfTuples();
    for (vtkIdType i = 0; i < count; ++i)
      {
      unsigned char newRGBA[4];
      unsigned char previousRGBA[4];
      this->Internal->EdgeColorArray->GetTypedTuple(i, previousRGBA);
      newRGBA[0] = previousRGBA[0];
      newRGBA[1] = previousRGBA[1];
      newRGBA[2] = previousRGBA[2];
      newRGBA[3] = value * 255;

      this->Internal->EdgeColorArray->SetTypedTuple(i, newRGBA);
      }
    }
  else
    {
    // Do nothing.
    }
  graphMapper->Update();
}

//-----------------------------------------------------------------------------
vtkVgPickData::PickData vtkVgGraphRepresentation::Pick(vtkSelection* selection)
{
  vtkVgPickData::PickData pickedData;

  if (!selection)
    {
    return pickedData;
    }

  vtkSelectionNode* glyphIds = selection->GetNode(0);

  if (!glyphIds)
    {
    return pickedData;
    }

  vtkAbstractArray* selectionList = glyphIds->GetSelectionList();
  if (!selectionList)
    {
    return pickedData;
    }

  vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast(selectionList);
  if (ids != 0)
    {
    vtkIdType numSelPoints = ids->GetNumberOfTuples();
    for (vtkIdType i = 0; i < numSelPoints; i++)
      {
      vtkIdType id = ids->GetValue(i);
      vtkIdType type = this->GraphModel->GetVertexEntityType(id);

      if (type != -1)
        {
        vtkVgPickData::PickEntity pickedEntity;
        pickedEntity.PickedType = type;
        pickedEntity.PickedId = id;

        pickedData.PickedEntities.push_back(pickedEntity);
        }
      }
    }

  return pickedData;
}

//-----------------------------------------------------------------------------
std::string vtkVgGraphRepresentation::GetGraphEdgeColorModeString(int mode)
{
  if (mode < CountEdgeColorModes)
    {
    for (int i = 0; i < CountEdgeColorModes; ++i)
      {
      if (mode == i)
        {
        return vtkInternal::GetGraphEdgeColorModeString(i);
        }
      }
    }

  return vtkInternal::GetGraphEdgeColorModeString(-1);
}

//-----------------------------------------------------------------------------
std::string vtkVgGraphRepresentation::GetGraphEdgeThicknessModeString(int mode)
{
  if (mode < CountEdgeThicknessModes)
    {
    for (int i = 0; i < CountEdgeThicknessModes; ++i)
      {
      if (mode == i)
        {
        return vtkInternal::GetGraphEdgeThicknessModeString(i);
        }
      }
    }

  return vtkInternal::GetGraphEdgeThicknessModeString(-1);
}
