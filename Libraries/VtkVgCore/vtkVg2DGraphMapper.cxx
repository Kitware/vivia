// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// VisGUI includes
#include "vtkVg2DGraphMapper.h"
#include "vtkVgApplySelectedColor.h"
#include "vtkVgArcParallelEdgeStrategy.h"
#include "vtkVgAssignCoordinatesLayoutStrategy.h"

// VTK includes
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDirectedGraph.h>
#include <vtkDistanceToCamera.h>
#include <vtkEdgeLayout.h>
#include <vtkExecutive.h>
#include <vtkFollower.h>
#include <vtkGarbageCollector.h>
#include <vtkGlyph3DMapper.h>
#include <vtkGlyphSource2D.h>
#include <vtkGraphLayout.h>
#include <vtkGraphToPolyData.h>
#include <vtkIconGlyphFilter.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkLookupTableWithEnabling.h>
#include <vtkMapArrayValues.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPassThroughEdgeStrategy.h>
#include <vtkPNGReader.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkTextProperty.h>
#include <vtkTexture.h>
#include <vtkTexturedActor2D.h>
#include <vtkTransformCoordinateSystems.h>
#include <vtkUndirectedGraph.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkViewTheme.h>

vtkStandardNewMacro(vtkVg2DGraphMapper);

#define VTK_CREATE(type,name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//----------------------------------------------------------------------------
vtkVg2DGraphMapper::vtkVg2DGraphMapper()
{
  this->VertexPointSize = 400.0;
  this->EdgeLineWidth = 1.0;

  this->GraphToPoly = vtkSmartPointer<vtkGraphToPolyData>::New();
  this->GraphToPoly->EdgeGlyphOutputOn();
  this->GraphToPoly->SetEdgeGlyphPosition(0.8);

  this->VertexGlyph  = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  this->IconTypeToIndex = vtkSmartPointer<vtkMapArrayValues>::New();
  this->IconGlyph = vtkSmartPointer<vtkIconGlyphFilter>::New();
  this->IconTransform = vtkSmartPointer<vtkTransformCoordinateSystems>::New();
  this->EdgeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

  this->DistanceToCamera  = vtkSmartPointer<vtkDistanceToCamera>::New();
  this->DistanceToCamera->SetScreenSize(this->VertexPointSize);
  this->DistanceToCamera->SetInputConnection(this->VertexGlyph->GetOutputPort());

  this->DistanceToCamera2  = vtkSmartPointer<vtkDistanceToCamera>::New();
  this->DistanceToCamera2->SetScreenSize(26.0);
  this->DistanceToCamera2->SetInputConnection(this->GraphToPoly->GetOutputPort(1));

  this->EdgeGlyphSource = vtkSmartPointer<vtkGlyphSource2D>::New();
  this->EdgeGlyphSource->SetGlyphTypeToEdgeArrow();
  this->EdgeGlyphSource->SetScale(1);
  this->EdgeGlyphSource->Update();

  this->EdgeGlyph = vtkSmartPointer<vtkGlyph3D>::New();
  this->EdgeGlyph->SetInputConnection(0, this->DistanceToCamera2->GetOutputPort());
  this->EdgeGlyph->SetInputConnection(1, this->EdgeGlyphSource->GetOutputPort());
  this->EdgeGlyph->SetScaleModeToScaleByScalar();
  this->EdgeGlyph->SetInputArrayToProcess(0, 0, 0,
    vtkDataObject::FIELD_ASSOCIATION_POINTS, "DistanceToCamera");

  this->VertexMapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
  this->VertexMapper->SetInputConnection(this->DistanceToCamera->GetOutputPort());
  this->VertexMapper->SetScaleArray("DistanceToCamera");

  this->IconMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->EdgeActor = vtkSmartPointer<vtkActor>::New();
  this->VertexActor = vtkSmartPointer<vtkActor>::New();
  this->IconActor = vtkSmartPointer<vtkTexturedActor2D>::New();
  this->LabelMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
  this->LabelActor = vtkSmartPointer<vtkActor2D>::New();
  this->PointsToLabels = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();

  this->EdgeGlyphMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->EdgeGlyphMapper->SetInputConnection(this->EdgeGlyph->GetOutputPort());
  this->EdgeGlyphMapper->SetScalarVisibility(0);
  this->EdgeGlyphActor = vtkSmartPointer<vtkActor>::New();
  this->EdgeGlyphActor->SetMapper(this->EdgeGlyphMapper);
  this->EdgeGlyphActor->GetProperty()->SetColor(0.0, 0.0, 0.0);

  this->ApplyColors = vtkSmartPointer<vtkVgApplySelectedColor>::New();

  this->GraphLayout = vtkSmartPointer<vtkGraphLayout>::New();
  this->GraphLayoutStrategy =
    vtkSmartPointer<vtkVgAssignCoordinatesLayoutStrategy>::New();

  this->GraphLayout->SetLayoutStrategy(this->GraphLayoutStrategy);

  this->EdgeLayout = vtkSmartPointer<vtkEdgeLayout>::New();
  this->PassThroughEdgeStrategy =
    vtkSmartPointer<vtkPassThroughEdgeStrategy>::New();
  this->ArcParallelEdgeStrategy =
    vtkSmartPointer<vtkVgArcParallelEdgeStrategy>::New();

  this->ArcParallelEdgeStrategy->SetMinLoopHeight(0.05);
  this->ArcParallelEdgeStrategy->SetMaxLoopHeight(0.15);
  this->ArcParallelEdgeStrategy->SetNumberOfSubdivisions(20);
  this->EdgeLayout->SetLayoutStrategy(this->PassThroughEdgeStrategy);
  this->EdgeLayout->SetInputConnection(this->GraphLayout->GetOutputPort());

  this->VertexLookupTable = vtkLookupTableWithEnabling::New();
  this->EdgeLookupTable = vtkLookupTableWithEnabling::New();
  this->VertexPositionArrayNameInternal = 0;
  this->VertexColorArrayNameInternal = 0;
  this->VertexSelectionArrayNameInternal = 0;
  this->EdgeColorArrayNameInternal = 0;
  this->EdgeSelectionArrayNameInternal = 0;
  this->EnabledEdgesArrayName = 0;
  this->EnabledVerticesArrayName = 0;

  this->VertexMapper->SetScalarModeToUsePointFieldData();
  this->VertexMapper->SelectColorArray("vtkVgApplySelectedColor color");
  this->VertexMapper->SetLookupTable(this->VertexLookupTable);
  this->VertexMapper->SetScalarVisibility(true);
  this->VertexMapper->ImmediateModeRenderingOff();
  this->VertexActor->PickableOn();
  this->EdgeMapper->SetScalarModeToUseCellFieldData();
  this->EdgeMapper->SelectColorArray("vtkVgApplySelectedColor color");
  this->EdgeMapper->SetLookupTable(this->EdgeLookupTable);
  this->EdgeMapper->SetScalarVisibility(true);
  this->EdgeMapper->ImmediateModeRenderingOff();
  this->EdgeActor->SetPosition(0, 0, -0.003);
  this->EdgeActor->GetProperty()->SetLineWidth(this->GetEdgeLineWidth());
  this->EdgeActor->GetProperty()->SetOpacity(0.9);

  this->ApplyColors->SetDefaultCellColor(0.5, 0.5, 0.5);
  this->ApplyColors->SetInputConnection(this->EdgeLayout->GetOutputPort());

  this->IconTransform->SetInputCoordinateSystemToWorld();
  this->IconTransform->SetOutputCoordinateSystemToDisplay();
  this->IconTransform->SetInputConnection(this->VertexGlyph->GetOutputPort());

  this->IconTypeToIndex->SetInputConnection(this->IconTransform->GetOutputPort());
  this->IconTypeToIndex->SetFieldType(vtkMapArrayValues::POINT_DATA);
  this->IconTypeToIndex->SetOutputArrayType(VTK_INT);
  this->IconTypeToIndex->SetPassArray(0);
  this->IconTypeToIndex->SetFillValue(-1);

  this->IconGlyph->SetInputConnection(this->IconTypeToIndex->GetOutputPort());
  this->IconGlyph->SetUseIconSize(true);
  this->IconMapper->SetInputConnection(this->IconGlyph->GetOutputPort());
  this->IconMapper->ScalarVisibilityOff();

  this->IconActor->SetMapper(this->IconMapper);
  this->IconArrayNameInternal = 0;

  this->VertexActor->SetMapper(this->VertexMapper);
  this->EdgeMapper->SetInputConnection(this->GraphToPoly->GetOutputPort());
  this->EdgeActor->SetMapper(this->EdgeMapper);

  this->PointsToLabels->SetInputConnection(this->GraphToPoly->GetOutputPort());
  this->PointsToLabels->GetTextProperty()->SetJustificationToCentered();
  this->PointsToLabels->GetTextProperty()->SetVerticalJustificationToCentered();
  this->PointsToLabels->GetTextProperty()->SetBold(1);
  this->PointsToLabels->GetTextProperty()->SetFontFamilyToTimes();
  this->PointsToLabels->GetTextProperty()->SetColor(0.0, 0.0, 0.0);

  // HACK: Setting the maximum depth to a large value helps minimize the strange
  // behavior where labels spiral outwards when two or more points are
  // coincident and there is a non-coincident point nearby. This is because the
  // "spiral offset" is divided by the max depth in
  // vtkLabelHierarchy::ComputeHierarchy().
  this->PointsToLabels->SetMaximumDepth(999999);

  this->LabelMapper->SetInputConnection(this->PointsToLabels->GetOutputPort());
  this->LabelActor->SetMapper(this->LabelMapper);

  // Set default parameters
  this->SetVertexColorArrayName("VertexDegree");
  this->SetVertexSelectionArrayName("Selected");
  this->SetColorVertices(false);
  this->SetEdgeColorArrayName("weight");
  this->SetEdgeSelectionArrayName("Selected");
  this->SetColorEdges(false);
  this->SetEnabledEdgesArrayName("weight");
  this->SetEnabledVerticesArrayName("VertexDegree");
  this->EnableEdgesByArray = 0;
  this->EnableVerticesByArray = 0;
  this->IconVisibilityOff();
  this->LabelVisibilityOff();
}

//----------------------------------------------------------------------------
vtkVg2DGraphMapper::~vtkVg2DGraphMapper()
{
  // Delete internally created objects.
  // Note: All of the smartpointer objects
  //       will be deleted for us
  this->SetVertexPositionArrayNameInternal(0);
  this->SetVertexColorArrayNameInternal(0);
  this->SetVertexSelectionArrayNameInternal(0);
  this->SetEdgeColorArrayNameInternal(0);
  this->SetEdgeSelectionArrayNameInternal(0);
  this->SetEnabledEdgesArrayName(0);
  this->SetEnabledVerticesArrayName(0);
  this->SetIconArrayNameInternal(0);
  this->VertexLookupTable->Delete();
  this->VertexLookupTable = 0;
  this->EdgeLookupTable->Delete();
  this->EdgeLookupTable = 0;
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetUseEdgeLayout(bool enabled)
{
  if (enabled)
    {
    this->EdgeLayout->SetLayoutStrategy(this->ArcParallelEdgeStrategy);
    }
  else
    {
    this->EdgeLayout->SetLayoutStrategy(this->PassThroughEdgeStrategy);
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetIconArrayName(const char* name)
{
  this->SetIconArrayNameInternal(name);
  this->IconGlyph->SetInputArrayToProcess(0, 0, 0,
                                          vtkDataObject::FIELD_ASSOCIATION_POINTS, name);
  this->IconTypeToIndex->SetInputArrayName(name);

  this->Modified();
}

//----------------------------------------------------------------------------
const char* vtkVg2DGraphMapper::GetIconArrayName()
{
  return this->GetIconArrayNameInternal();
}

//----------------------------------------------------------------------------
// Helper method
vtkPolyData* vtkVg2DGraphMapper::CreateCircle(bool vtkNotUsed(filled))
{
  int circleRes = 16;
  vtkIdType ptIds[17];
  double x[3], theta;

  // Allocate storage
  vtkPolyData* poly = vtkPolyData::New();
  VTK_CREATE(vtkPoints, pts);
  VTK_CREATE(vtkCellArray, circle);

  // generate points around the circle
  x[2] = 0.0;
  theta = 2.0 * vtkMath::Pi() / circleRes;
  for (int i = 0; i < circleRes; i++)
    {
    x[0] = 0.5 * cos(i * theta);
    x[1] = 0.5 * sin(i * theta);
    ptIds[i] = pts->InsertNextPoint(x);
    }
  circle->InsertNextCell(circleRes, ptIds);

  poly->SetPoints(pts);
  poly->SetPolys(circle);
  return poly;
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetVertexColorArrayName(const char* name)
{
  this->SetVertexColorArrayNameInternal(name);
  this->ApplyColors->SetInputArrayToProcess(1, 0, 0,
    vtkDataObject::FIELD_ASSOCIATION_VERTICES, name);
  this->Modified();
}

//----------------------------------------------------------------------------
const char* vtkVg2DGraphMapper::GetVertexColorArrayName()
{
  return this->GetVertexColorArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetVertexPositionArrayName(const char* name)
{
  this->SetVertexPositionArrayNameInternal(name);
  this->GraphLayoutStrategy->SetXYZCoordArrayName(name);
  this->Modified();
}

//----------------------------------------------------------------------------
const char* vtkVg2DGraphMapper::GetVertexPositionArrayName()
{
  return this->GetVertexPositionArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetVertexSelectionArrayName(const char* name)
{
  this->SetVertexSelectionArrayNameInternal(name);
  this->ApplyColors->SetInputArrayToProcess(0, 0, 0,
    vtkDataObject::FIELD_ASSOCIATION_VERTICES, name);
  this->Modified();
}

//----------------------------------------------------------------------------
const char* vtkVg2DGraphMapper::GetVertexSelectionArrayName()
{
  return this->GetVertexSelectionArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetLabelArrayName(const char* name)
{
  this->PointsToLabels->SetLabelArrayName(name);
  this->Modified();
}

//----------------------------------------------------------------------------
const char* vtkVg2DGraphMapper::GetLabelArrayName()
{
  return this->PointsToLabels->GetLabelArrayName();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetUseVertexSelection(bool enable)
{
  this->ApplyColors->SetUsePointSelection(enable);
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkVg2DGraphMapper::GetUseVertexSelection()
{
  return this->ApplyColors->GetUsePointSelection();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetColorVertices(bool vis)
{
  this->ApplyColors->SetUsePointScalars(vis);
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkVg2DGraphMapper::GetColorVertices()
{
  return this->ApplyColors->GetUsePointScalars();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetIconVisibility(bool vis)
{
  this->IconActor->SetVisibility(vis);
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkVg2DGraphMapper::GetIconVisibility()
{
  return this->IconActor->GetVisibility() != 0;
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetLabelVisibility(bool vis)
{
  this->LabelActor->SetVisibility(vis);
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkVg2DGraphMapper::GetLabelVisibility()
{
  return this->LabelActor->GetVisibility() != 0;
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetLabelColor(double color[3])
{
  this->PointsToLabels->GetTextProperty()->SetColor(color);
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetEdgeGlyphVisibility(bool vis)
{
  this->EdgeGlyphActor->SetVisibility(vis);
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkVg2DGraphMapper::GetEdgeGlyphVisibility()
{
  return this->EdgeGlyphActor->GetVisibility() != 0;
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetEdgeGlyphColor(double color[3])
{
  this->EdgeGlyphActor->GetProperty()->SetColor(color);
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetEdgeColorArrayName(const char* name)
{
  this->SetEdgeColorArrayNameInternal(name);
  this->ApplyColors->SetInputArrayToProcess(3, 0, 0,
    vtkDataObject::FIELD_ASSOCIATION_EDGES, name);
  this->Modified();
}

//----------------------------------------------------------------------------
const char* vtkVg2DGraphMapper::GetEdgeColorArrayName()
{
  return this->GetEdgeColorArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetEdgeSelectionArrayName(const char* name)
{
  this->SetEdgeSelectionArrayNameInternal(name);
  this->ApplyColors->SetInputArrayToProcess(2, 0, 0,
    vtkDataObject::FIELD_ASSOCIATION_EDGES, name);
  this->Modified();
}

//----------------------------------------------------------------------------
const char* vtkVg2DGraphMapper::GetEdgeSelectionArrayName()
{
  return this->GetEdgeSelectionArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetUseEdgeSelection(bool enable)
{
  this->ApplyColors->SetUseCellSelection(enable);
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkVg2DGraphMapper::GetUseEdgeSelection()
{
  return this->ApplyColors->GetUseCellSelection();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetColorEdges(bool vis)
{
  this->ApplyColors->SetUseCellScalars(vis);
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkVg2DGraphMapper::GetColorEdges()
{
  return this->ApplyColors->GetUseCellScalars();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetVertexPointSize(double size)
{
  this->VertexPointSize = size;

  this->DistanceToCamera->SetScreenSize(size);

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetEdgeLineWidth(float width)
{
  this->EdgeLineWidth = width;
  this->EdgeActor->GetProperty()->SetLineWidth(this->GetEdgeLineWidth());

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::AddIconType(char* type, int index)
{
  this->IconTypeToIndex->AddToMap(type, index);
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::ClearIconTypes()
{
  this->IconTypeToIndex->ClearMap();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetVertexVisibility(bool vis)
{
  this->VertexActor->SetVisibility(vis);

  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkVg2DGraphMapper::GetVertexVisibility()
{
  return this->VertexActor->GetVisibility() ? true : false;
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetEdgeVisibility(bool vis)
{
  this->EdgeActor->SetVisibility(vis);

  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkVg2DGraphMapper::GetEdgeVisibility()
{
  return this->EdgeActor->GetVisibility() ? true : false;
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetIconSize(int* size)
{
  this->IconGlyph->SetIconSize(size);

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetIconAlignment(int alignment)
{
  this->IconGlyph->SetGravity(alignment);

  this->Modified();
}

//----------------------------------------------------------------------------
int* vtkVg2DGraphMapper::GetIconSize()
{
  return this->IconGlyph->GetIconSize();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetIconTexture(vtkTexture* texture)
{
  this->IconActor->SetTexture(texture);

  this->Modified();
}

//----------------------------------------------------------------------------
vtkTexture* vtkVg2DGraphMapper::GetIconTexture()
{
  return this->IconActor->GetTexture();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetInputData(vtkGraph* input)
{
  if (input)
    {
    this->SetInputDataInternal(0, input);
    }

  this->Modified();
}

//----------------------------------------------------------------------------
vtkGraph* vtkVg2DGraphMapper::GetInput()
{
  vtkGraph* inputGraph =
    vtkGraph::SafeDownCast(this->Superclass::GetInputAsDataSet());
  return inputGraph;
}

// ---------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetVertexGeometry(vtkPolyData* geometry)
{
  this->VertexGeometry = geometry;

  this->VertexMapper->SetSourceData(this->VertexGeometry);

  this->Modified();
}

// ---------------------------------------------------------------------------
vtkPolyData* vtkVg2DGraphMapper::GetVertexGeometry()
{
  return this->VertexGeometry;
}

// ---------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetVertexOpacity(double value)
{
  this->VertexActor->GetProperty()->SetOpacity(value);
}

// ---------------------------------------------------------------------------
void vtkVg2DGraphMapper::SetEdgeOpacity(double value)
{
  this->EdgeActor->GetProperty()->SetOpacity(value);
}

// ---------------------------------------------------------------------------
vtkGlyphSource2D* vtkVg2DGraphMapper::GetEdgeGlyphSource()
{
  return this->EdgeGlyphSource;
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::ReleaseGraphicsResources(vtkWindow* renWin)
{
  if (this->EdgeMapper)
    {
    this->EdgeMapper->ReleaseGraphicsResources(renWin);
    }
  if (this->VertexMapper)
    {
    this->VertexMapper->ReleaseGraphicsResources(renWin);
    }
  if (this->IconMapper)
    {
    this->IconMapper->ReleaseGraphicsResources(renWin);
    }
}

//----------------------------------------------------------------------------
// Receives from Actor -> maps data to primitives
//
void vtkVg2DGraphMapper::Render(vtkRenderer* ren, vtkActor* vtkNotUsed(act))
{
  this->DistanceToCamera->SetRenderer(ren);
  this->DistanceToCamera2->SetRenderer(ren);

  // make sure that we've been properly initialized
  if (!this->GetExecutive()->GetInputData(0, 0))
    {
    vtkErrorMacro(<< "No input!\n");
    return;
    }

  // Update the pipeline up until the graph to poly data
  vtkGraph* input = vtkGraph::SafeDownCast(this->GetExecutive()->GetInputData(0, 0));
  if (!input)
    {
    vtkErrorMacro(<< "Input is not a graph!\n");
    return;
    }
  vtkGraph* graph = 0;
  if (vtkDirectedGraph::SafeDownCast(input))
    {
    graph = vtkDirectedGraph::New();
    }
  else
    {
    graph = vtkUndirectedGraph::New();
    }
  graph->ShallowCopy(input);

  this->GraphLayout->SetInputData(graph);
  this->GraphToPoly->SetInputConnection(this->ApplyColors->GetOutputPort());
  this->VertexGlyph->SetInputConnection(this->ApplyColors->GetOutputPort());
  graph->Delete();
  this->GraphToPoly->Update();
  this->DistanceToCamera->Update();
  this->DistanceToCamera2->Update();
  vtkPolyData* edgePd = this->GraphToPoly->GetOutput();
  vtkPolyData* vertPd = this->VertexGlyph->GetOutput();

  // Try to find the range the user-specified color array.
  // If we cannot find that array, use the scalar range.
  double range[2];
  vtkDataArray* arr = 0;
  if (this->GetColorEdges())
    {
    if (this->GetEdgeColorArrayName())
      {
      arr = edgePd->GetCellData()->GetArray(this->GetEdgeColorArrayName());
      }
    if (!arr)
      {
      arr = edgePd->GetCellData()->GetScalars();
      }
    if (arr)
      {
      arr->GetRange(range);
      this->EdgeMapper->SetScalarRange(range[0], range[1]);
      }
    }

  arr = 0;
  if (this->EnableEdgesByArray && this->EnabledEdgesArrayName)
    {
    vtkLookupTableWithEnabling::SafeDownCast(this->EdgeLookupTable)->SetEnabledArray(
      edgePd->GetCellData()->GetArray(this->GetEnabledEdgesArrayName()));
    }
  else
    {
    vtkLookupTableWithEnabling::SafeDownCast(this->EdgeLookupTable)->SetEnabledArray(0);
    }

  // Do the same thing for the vertex array.
  arr = 0;
  if (this->GetColorVertices())
    {
    if (this->GetVertexColorArrayName())
      {
      arr = vertPd->GetPointData()->GetArray(this->GetVertexColorArrayName());
      }
    if (!arr)
      {
      arr = vertPd->GetPointData()->GetScalars();
      }
    if (arr)
      {
      arr->GetRange(range);
      this->VertexMapper->SetScalarRange(range[0], range[1]);
      }
    }

  arr = 0;
  if (this->EnableVerticesByArray && this->EnabledVerticesArrayName)
    {
    vtkLookupTableWithEnabling::SafeDownCast(this->VertexLookupTable)->SetEnabledArray(
      vertPd->GetPointData()->GetArray(this->GetEnabledVerticesArrayName()));
    }
  else
    {
    vtkLookupTableWithEnabling::SafeDownCast(this->VertexLookupTable)->SetEnabledArray(0);
    }

  if (this->IconActor->GetTexture() &&
      this->IconActor->GetTexture()->GetInput() &&
      this->IconActor->GetVisibility())
    {
    this->IconTransform->SetViewport(ren);
    this->IconActor->GetTexture()->MapColorScalarsThroughLookupTableOff();
    this->IconActor->GetTexture()->Update();
    int* dim = this->IconActor->GetTexture()->GetInput()->GetDimensions();
    this->IconGlyph->SetIconSheetSize(dim);
    // Override the array for vtkIconGlyphFilter to process if we have
    // a map of icon types.
    if (this->IconTypeToIndex->GetMapSize())
      {
      this->IconGlyph->SetInputArrayToProcess(0, 0, 0,
        vtkDataObject::FIELD_ASSOCIATION_POINTS,
        this->IconTypeToIndex->GetOutputArrayName());
      }
    }
  if (this->EdgeActor->GetVisibility())
    {
    this->EdgeActor->RenderOpaqueGeometry(ren);
    }
  if (this->VertexActor->GetVisibility())
    {
    this->VertexActor->RenderOpaqueGeometry(ren);
    }
  if (this->IconActor->GetVisibility())
    {
    this->IconActor->RenderOpaqueGeometry(ren);
    }
  if (this->LabelActor->GetVisibility())
    {
    this->LabelActor->RenderOpaqueGeometry(ren);
    }
  if (this->EdgeGlyphActor->GetVisibility())
    {
    this->EdgeGlyphActor->RenderOpaqueGeometry(ren);
    }

  if (this->EdgeActor->GetVisibility())
    {
    this->EdgeActor->RenderTranslucentPolygonalGeometry(ren);
    }
  if (this->VertexActor->GetVisibility())
    {
    this->VertexActor->RenderTranslucentPolygonalGeometry(ren);
    }
  if (this->IconActor->GetVisibility())
    {
    this->IconActor->RenderTranslucentPolygonalGeometry(ren);
    }
  if (this->LabelActor->GetVisibility())
    {
    this->LabelActor->RenderTranslucentPolygonalGeometry(ren);
    }
  if (this->EdgeGlyphActor->GetVisibility())
    {
    this->EdgeGlyphActor->RenderTranslucentPolygonalGeometry(ren);
    }

  if (this->IconActor->GetVisibility())
    {
    this->IconActor->RenderOverlay(ren);
    }
  if (this->LabelActor->GetVisibility())
    {
    this->LabelActor->RenderOverlay(ren);
    }

  this->TimeToDraw = this->EdgeMapper->GetTimeToDraw() +
                     this->VertexMapper->GetTimeToDraw() +
                     this->IconMapper->GetTimeToDraw();
}

void vtkVg2DGraphMapper::ApplyViewTheme(vtkViewTheme* theme)
{
  this->VertexActor->GetProperty()->SetColor(theme->GetPointColor());
  this->VertexActor->GetProperty()->SetOpacity(theme->GetPointOpacity());
  this->SetVertexPointSize(theme->GetPointSize());
  this->VertexLookupTable->SetHueRange(theme->GetPointHueRange());
  this->VertexLookupTable->SetSaturationRange(theme->GetPointSaturationRange());
  this->VertexLookupTable->SetValueRange(theme->GetPointValueRange());
  this->VertexLookupTable->SetAlphaRange(theme->GetPointAlphaRange());
  this->VertexLookupTable->Build();

  this->EdgeActor->GetProperty()->SetColor(theme->GetCellColor());
  this->EdgeActor->GetProperty()->SetOpacity(theme->GetCellOpacity());
  this->SetEdgeLineWidth(theme->GetLineWidth());
  this->EdgeLookupTable->SetHueRange(theme->GetCellHueRange());
  this->EdgeLookupTable->SetSaturationRange(theme->GetCellSaturationRange());
  this->EdgeLookupTable->SetValueRange(theme->GetCellValueRange());
  this->EdgeLookupTable->SetAlphaRange(theme->GetCellAlphaRange());
  this->EdgeLookupTable->Build();
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  if (this->EdgeMapper)
    {
    os << indent << "EdgeMapper: (" << this->EdgeMapper << ")\n";
    }
  else
    {
    os << indent << "EdgeMapper: (none)\n";
    }
  if (this->VertexMapper)
    {
    os << indent << "VertexMapper: (" << this->VertexMapper << ")\n";
    }
  else
    {
    os << indent << "VertexMapper: (none)\n";
    }
  if (this->EdgeActor)
    {
    os << indent << "EdgeActor: (" << this->EdgeActor << ")\n";
    }
  else
    {
    os << indent << "EdgeActor: (none)\n";
    }
  if (this->VertexActor)
    {
    os << indent << "VertexActor: (" << this->VertexActor << ")\n";
    }
  else
    {
    os << indent << "VertexActor: (none)\n";
    }

  if (this->GraphToPoly)
    {
    os << indent << "GraphToPoly: (" << this->GraphToPoly << ")\n";
    }
  else
    {
    os << indent << "GraphToPoly: (none)\n";
    }

  if (this->VertexLookupTable)
    {
    os << indent << "VertexLookupTable: (" << this->VertexLookupTable << ")\n";
    }
  else
    {
    os << indent << "VertexLookupTable: (none)\n";
    }

  if (this->EdgeLookupTable)
    {
    os << indent << "EdgeLookupTable: (" << this->EdgeLookupTable << ")\n";
    }
  else
    {
    os << indent << "EdgeLookupTable: (none)\n";
    }

  os << indent << "VertexPointSize: " << this->VertexPointSize << endl;
  os << indent << "EdgeLineWidth: " << this->EdgeLineWidth << endl;
  os << indent << "EnableEdgesByArray: " << this->EnableEdgesByArray << endl;
  os << indent << "EnableVerticesByArray: " << this->EnableVerticesByArray << endl;
  os << indent << "EnabledEdgesArrayName: "
     << (this->EnabledEdgesArrayName ? "" : "(null)") << endl;
  os << indent << "EnabledVerticesArrayName: "
     << (this->EnabledVerticesArrayName ? "" : "(null)") << endl;

}

//----------------------------------------------------------------------------
vtkMTimeType vtkVg2DGraphMapper::GetMTime()
{
  vtkMTimeType mTime = this->vtkMapper::GetMTime();
  vtkMTimeType time;

  if (this->LookupTable != NULL)
    {
    time = this->LookupTable->GetMTime();
    mTime = (time > mTime ? time : mTime);
    }

  return mTime;
}

//----------------------------------------------------------------------------
int vtkVg2DGraphMapper::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkGraph");
  return 1;
}

//----------------------------------------------------------------------------
double* vtkVg2DGraphMapper::GetBounds()
{
  vtkGraph* graph = vtkGraph::SafeDownCast(this->GetExecutive()->GetInputData(0, 0));
  if (!graph)
    {
    vtkMath::UninitializeBounds(this->Bounds);
    return this->Bounds;
    }
  if (!this->Static)
    {
    this->Update();
    graph = vtkGraph::SafeDownCast(this->GetExecutive()->GetInputData(0, 0));
    }
  if (!graph)
    {
    vtkMath::UninitializeBounds(this->Bounds);
    return this->Bounds;
    }

  if (this->VertexGlyph->GetInput())
    {
    this->VertexMapper->GetBounds(this->Bounds);
    }
  else
    {
    // If there is no input yet to vertex glyph, then it means no render has
    // happened yet, and therefore we don't have good bounds info. However,
    // we may never get rendered at all if we're culled due to returning bad
    // bounds! Work around this chicken-and-egg problem by returning null
    // bounds, ensuring that the initial render will always occur.
    return 0;
    //graph->GetBounds(this->Bounds);
    }

  return this->Bounds;
}

//----------------------------------------------------------------------------
void vtkVg2DGraphMapper::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
  // These filters share our input and are therefore involved in a
  // reference loop.
  //vtkGarbageCollectorReport(collector, this->GraphToPoly,
  //                          "GraphToPoly");
}
