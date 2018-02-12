/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVgGraphMapper.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/
#include "vtkVgGraphMapper.h"

#include "vtkVgApplySelectedColor.h"

#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkVgArcParallelEdgeStrategy.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDirectedGraph.h"
#include "vtkEdgeLayout.h"
#include "vtkExecutive.h"
#include "vtkFollower.h"
#include "vtkGarbageCollector.h"
#include "vtkGlyph3DMapper.h"
#include "vtkGraphToPolyData.h"
#include "vtkIconGlyphFilter.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkLabelPlacementMapper.h"
#include "vtkLookupTableWithEnabling.h"
#include "vtkMapArrayValues.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPassThroughEdgeStrategy.h"
#include "vtkPNGReader.h"
#include "vtkPlaneSource.h"
#include "vtkPointData.h"
#include "vtkPointSetToLabelHierarchy.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkTextProperty.h"
#include "vtkTexture.h"
#include "vtkTexturedActor2D.h"
#include "vtkTransformCoordinateSystems.h"
#include "vtkUndirectedGraph.h"
#include "vtkVertexGlyphFilter.h"
#include "vtkViewTheme.h"

vtkStandardNewMacro(vtkVgGraphMapper);

#define VTK_CREATE(type,name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//----------------------------------------------------------------------------
vtkVgGraphMapper::vtkVgGraphMapper()
{
  this->GraphToPoly       = vtkSmartPointer<vtkGraphToPolyData>::New();
  this->VertexGlyph       = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  this->IconTypeToIndex   = vtkSmartPointer<vtkMapArrayValues>::New();
  this->IconGlyph         = vtkSmartPointer<vtkIconGlyphFilter>::New();
  this->IconTransform     = vtkSmartPointer<vtkTransformCoordinateSystems>::New();
  this->EdgeMapper        = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->VertexMapper      = vtkSmartPointer<vtkGlyph3DMapper>::New();
  this->IconMapper        = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->EdgeActor         = vtkSmartPointer<vtkActor>::New();
  this->VertexActor       = vtkSmartPointer<vtkActor>::New();
  this->IconActor         = vtkSmartPointer<vtkTexturedActor2D>::New();
  this->LabelMapper       = vtkSmartPointer<vtkLabelPlacementMapper>::New();
  this->LabelActor        = vtkSmartPointer<vtkActor2D>::New();
  this->PointsToLabels    = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
  this->ApplyColors       = vtkSmartPointer<vtkVgApplySelectedColor>::New();

  this->EdgeLayout  = vtkSmartPointer<vtkEdgeLayout>::New();
  this->PassThroughEdgeStrategy
    = vtkSmartPointer<vtkPassThroughEdgeStrategy>::New();
  this->ArcParallelEdgeStrategy
    = vtkSmartPointer<vtkVgArcParallelEdgeStrategy>::New();

  this->ArcParallelEdgeStrategy->SetMinLoopHeight(0.05);
  this->ArcParallelEdgeStrategy->SetMaxLoopHeight(0.15);
  this->ArcParallelEdgeStrategy->SetNumberOfSubdivisions(20);
  this->EdgeLayout->SetLayoutStrategy(this->PassThroughEdgeStrategy);

  this->VertexLookupTable = vtkLookupTableWithEnabling::New();
  this->EdgeLookupTable   = vtkLookupTableWithEnabling::New();
  this->VertexColorArrayNameInternal = 0;
  this->VertexSelectionArrayNameInternal = 0;
  this->EdgeColorArrayNameInternal = 0;
  this->EdgeSelectionArrayNameInternal = 0;
  this->EnabledEdgesArrayName = 0;
  this->EnabledVerticesArrayName = 0;
  this->VertexPointSize = 5;
  this->EdgeLineWidth = 1;
  this->ScaledGlyphs = false;
  this->ScalingArrayName = 0;

  this->VertexMapper->SetScalarModeToUsePointFieldData();
  this->VertexMapper->SelectColorArray("vtkVgApplySelectedColor color");
  this->VertexMapper->SetLookupTable(this->VertexLookupTable);
  this->VertexMapper->SetScalarVisibility(true);
  this->VertexMapper->ImmediateModeRenderingOff();
  this->VertexMapper->SetClamping(false);
  this->VertexActor->PickableOn();
  this->VertexActor->GetProperty()->SetPointSize(this->GetVertexPointSize());
  this->EdgeMapper->SetScalarModeToUseCellFieldData();
  this->EdgeMapper->SelectColorArray("vtkVgApplySelectedColor color");
  this->EdgeMapper->SetLookupTable(this->EdgeLookupTable);
  this->EdgeMapper->SetScalarVisibility(true);
  this->EdgeMapper->ImmediateModeRenderingOff();
  this->EdgeActor->SetPosition(0, 0, -0.003);
  this->EdgeActor->GetProperty()->SetLineWidth(this->GetEdgeLineWidth());
  //this->EdgeActor->GetProperty()->SetColor(0.5, 0.5, 0.5);
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

  this->VertexMapper->SetInputConnection(this->VertexGlyph->GetOutputPort());
  this->VertexActor->SetMapper(this->VertexMapper);
  this->EdgeMapper->SetInputConnection(this->GraphToPoly->GetOutputPort());
  this->EdgeActor->SetMapper(this->EdgeMapper);

  this->PointsToLabels->SetInputConnection(this->GraphToPoly->GetOutputPort());
  this->PointsToLabels->GetTextProperty()->SetJustificationToCentered();
  this->PointsToLabels->GetTextProperty()->SetVerticalJustificationToCentered();
  this->PointsToLabels->GetTextProperty()->SetBold(1);

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
vtkVgGraphMapper::~vtkVgGraphMapper()
{
  // Delete internally created objects.
  // Note: All of the smartpointer objects
  //       will be deleted for us

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
  if (this->ScalingArrayName != 0)
    {
    delete[] this->ScalingArrayName;
    }
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetUseEdgeLayout(bool enabled)
{
  if (enabled)
    {
    this->EdgeLayout->SetLayoutStrategy(this->ArcParallelEdgeStrategy);
    }
  else
    {
    this->EdgeLayout->SetLayoutStrategy(this->PassThroughEdgeStrategy);
    }
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetIconArrayName(const char* name)
{
  this->SetIconArrayNameInternal(name);
  this->IconGlyph->SetInputArrayToProcess(0, 0, 0,
                                          vtkDataObject::FIELD_ASSOCIATION_POINTS, name);
  this->IconTypeToIndex->SetInputArrayName(name);
}

//----------------------------------------------------------------------------
const char* vtkVgGraphMapper::GetIconArrayName()
{
  return this->GetIconArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetScaledGlyphs(bool arg)
{
  if (arg)
    {
    if (this->ScalingArrayName)
      {
      if (this->VertexGeometries.empty())
        {
        vtkPolyData* circle = this->CreateCircle(true);
        this->SetVertexGeometry(circle);
        circle->Delete();
        }

      this->VertexMapper->SetScaling(1);
      this->VertexMapper->SetScaleArray(this->ScalingArrayName);
      this->VertexMapper->SetScaleModeToScaleByMagnitude();
      }
    else
      {
      vtkErrorMacro("No scaling array name set");
      }
    }
  else
    {
    this->VertexMapper->SetInputConnection(this->VertexGlyph->GetOutputPort());
    }
}


//----------------------------------------------------------------------------
// Helper method
vtkPolyData* vtkVgGraphMapper::CreateCircle(bool vtkNotUsed(filled))
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
void vtkVgGraphMapper::SetVertexColorArrayName(const char* name)
{
  this->SetVertexColorArrayNameInternal(name);
  this->ApplyColors->SetInputArrayToProcess(1, 0, 0,
                                            vtkDataObject::FIELD_ASSOCIATION_VERTICES, name);
}

//----------------------------------------------------------------------------
const char* vtkVgGraphMapper::GetVertexColorArrayName()
{
  return this->GetVertexColorArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetVertexSelectionArrayName(const char* name)
{
  this->SetVertexSelectionArrayNameInternal(name);
  this->ApplyColors->SetInputArrayToProcess(0, 0, 0,
                                            vtkDataObject::FIELD_ASSOCIATION_VERTICES, name);
}

//----------------------------------------------------------------------------
const char* vtkVgGraphMapper::GetVertexSelectionArrayName()
{
  return this->GetVertexSelectionArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetLabelArrayName(const char* name)
{
  this->PointsToLabels->SetLabelArrayName(name);
}

//----------------------------------------------------------------------------
const char* vtkVgGraphMapper::GetLabelArrayName()
{
  return this->PointsToLabels->GetLabelArrayName();
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetUseVertexSelection(bool enable)
{
  this->ApplyColors->SetUsePointSelection(enable);
}

//----------------------------------------------------------------------------
bool vtkVgGraphMapper::GetUseVertexSelection()
{
  return this->ApplyColors->GetUsePointSelection();
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetColorVertices(bool vis)
{
  this->ApplyColors->SetUsePointScalars(vis);
}

//----------------------------------------------------------------------------
bool vtkVgGraphMapper::GetColorVertices()
{
  return this->ApplyColors->GetUsePointScalars();
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetIconVisibility(bool vis)
{
  this->IconActor->SetVisibility(vis);
}

//----------------------------------------------------------------------------
bool vtkVgGraphMapper::GetIconVisibility()
{
  return this->IconActor->GetVisibility() != 0;
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetLabelVisibility(bool vis)
{
  this->LabelActor->SetVisibility(vis);
}

//----------------------------------------------------------------------------
bool vtkVgGraphMapper::GetLabelVisibility()
{
  return this->LabelActor->GetVisibility() != 0;
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetEdgeColorArrayName(const char* name)
{
  this->SetEdgeColorArrayNameInternal(name);
  this->ApplyColors->SetInputArrayToProcess(3, 0, 0,
    vtkDataObject::FIELD_ASSOCIATION_EDGES, name);
}

//----------------------------------------------------------------------------
const char* vtkVgGraphMapper::GetEdgeColorArrayName()
{
  return this->GetEdgeColorArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetEdgeSelectionArrayName(const char* name)
{
  this->SetEdgeSelectionArrayNameInternal(name);
  this->ApplyColors->SetInputArrayToProcess(2, 0, 0,
    vtkDataObject::FIELD_ASSOCIATION_EDGES, name);
}

//----------------------------------------------------------------------------
const char* vtkVgGraphMapper::GetEdgeSelectionArrayName()
{
  return this->GetEdgeSelectionArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetUseEdgeSelection(bool enable)
{
  this->ApplyColors->SetUseCellSelection(enable);
}

//----------------------------------------------------------------------------
bool vtkVgGraphMapper::GetUseEdgeSelection()
{
  return this->ApplyColors->GetUseCellSelection();
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetColorEdges(bool vis)
{
  this->ApplyColors->SetUseCellScalars(vis);
}

//----------------------------------------------------------------------------
bool vtkVgGraphMapper::GetColorEdges()
{
  return this->ApplyColors->GetUseCellScalars();
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetVertexPointSize(float size)
{
  this->VertexPointSize = size;
  this->VertexActor->GetProperty()->SetPointSize(this->GetVertexPointSize());
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetEdgeLineWidth(float width)
{
  this->EdgeLineWidth = width;
  this->EdgeActor->GetProperty()->SetLineWidth(this->GetEdgeLineWidth());
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::AddIconType(char* type, int index)
{
  this->IconTypeToIndex->AddToMap(type, index);
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::ClearIconTypes()
{
  this->IconTypeToIndex->ClearMap();
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetVertexVisibility(bool vis)
{
  this->VertexActor->SetVisibility(vis);
}

//----------------------------------------------------------------------------
bool vtkVgGraphMapper::GetVertexVisibility()
{
  return this->VertexActor->GetVisibility() ? true : false;
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetEdgeVisibility(bool vis)
{
  this->EdgeActor->SetVisibility(vis);
}

//----------------------------------------------------------------------------
bool vtkVgGraphMapper::GetEdgeVisibility()
{
  return this->EdgeActor->GetVisibility() ? true : false;
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetIconSize(int* size)
{
  this->IconGlyph->SetIconSize(size);
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetIconAlignment(int alignment)
{
  this->IconGlyph->SetGravity(alignment);
}

//----------------------------------------------------------------------------
int* vtkVgGraphMapper::GetIconSize()
{
  return this->IconGlyph->GetIconSize();
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetIconTexture(vtkTexture* texture)
{
  this->IconActor->SetTexture(texture);
}

//----------------------------------------------------------------------------
vtkTexture* vtkVgGraphMapper::GetIconTexture()
{
  return this->IconActor->GetTexture();
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetInputData(vtkGraph* input)
{
  if (input)
    {
    this->SetInputDataInternal(0, input);
    }
  this->Modified();
}

//----------------------------------------------------------------------------
vtkGraph* vtkVgGraphMapper::GetInput()
{
  vtkGraph* inputGraph =
    vtkGraph::SafeDownCast(this->Superclass::GetInputAsDataSet());
  return inputGraph;
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetVertexGeometry(vtkPolyData* vertexGeometry)
{
  this->SetVertexGeometry(0, vertexGeometry);
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> vtkVgGraphMapper::GetVertexGeometry()
{
  return this->GetVertexGeometry(0);
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::SetVertexGeometry(vtkIdType index, vtkPolyData* vertexGeometry)
{
  if (!vertexGeometry)
    {
    vtkErrorMacro("ERROR: Invalid (NULL) vertex geometry\n");
    return;
    }

  this->VertexGeometries[index] = vertexGeometry;

  this->VertexMapper->SetSourceData(index, this->VertexGeometries[index]);

  this->Modified();
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> vtkVgGraphMapper::GetVertexGeometry(vtkIdType index)
{
  std::map<vtkIdType, vtkSmartPointer<vtkPolyData> >::iterator itr =
    this->VertexGeometries.find(index);

  if (itr != this->VertexGeometries.end())
    {
    return itr->second;
    }
  else
    {
    return 0;
    }
}

// ---------------------------------------------------------------------------
void vtkVgGraphMapper::SetVertexGeometryIndexArray(const char* arrayName)
{
  this->VertexMapper->SetSourceIndexArray(arrayName);
}

// ---------------------------------------------------------------------------
void vtkVgGraphMapper::SetVertexGeometryIndexArray(int fieldAttributeType)
{
  this->VertexMapper->SetSourceIndexArray(fieldAttributeType);
}

// ---------------------------------------------------------------------------
void vtkVgGraphMapper::SetVertexGeometryIndexing(int value)
{
  if (value == 1)
    {
    this->VertexMapper->SetSourceIndexing(true);
    }
  else if (value == 0)
    {
    this->VertexMapper->SetSourceIndexing(false);
    }
  else
    {
    // Do nothing.
    }
}

// ---------------------------------------------------------------------------
void vtkVgGraphMapper::GetVertexGeometryIndexing(int& value)
{
  value = this->VertexMapper->GetSourceIndexing();
}

// ---------------------------------------------------------------------------
void vtkVgGraphMapper::SetNumberOfVertexGeometrySources(int number)
{
  this->VertexMapper->SetRange(0.0, static_cast<double>(number));
}

// ---------------------------------------------------------------------------
void vtkVgGraphMapper::GetNumberOfVertexGeometrySources(int& number)
{
  double range[2];
  this->VertexMapper->GetRange(range);

  number = static_cast<int>(range[1]);
}

// ---------------------------------------------------------------------------
void vtkVgGraphMapper::SetEdgeOpacity(double value)
{
  this->EdgeActor->GetProperty()->SetOpacity(value);
}

//----------------------------------------------------------------------------
void vtkVgGraphMapper::ReleaseGraphicsResources(vtkWindow* renWin)
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
void vtkVgGraphMapper::Render(vtkRenderer* ren, vtkActor* vtkNotUsed(act))
{
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

  this->EdgeLayout->SetInputData(graph);
  this->GraphToPoly->SetInputConnection(this->ApplyColors->GetOutputPort());
  this->VertexGlyph->SetInputConnection(this->ApplyColors->GetOutputPort());
  graph->Delete();
  this->GraphToPoly->Update();
  this->VertexGlyph->Update();
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

void vtkVgGraphMapper::ApplyViewTheme(vtkViewTheme* theme)
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
void vtkVgGraphMapper::PrintSelf(ostream& os, vtkIndent indent)
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
  os << indent << "ScaledGlyphs: " << this->ScaledGlyphs << endl;
  os << indent << "ScalingArrayName: "
     << (this->ScalingArrayName ? "" : "(null)") << endl;
  os << indent << "EnableEdgesByArray: " << this->EnableEdgesByArray << endl;
  os << indent << "EnableVerticesByArray: " << this->EnableVerticesByArray << endl;
  os << indent << "EnabledEdgesArrayName: "
     << (this->EnabledEdgesArrayName ? "" : "(null)") << endl;
  os << indent << "EnabledVerticesArrayName: "
     << (this->EnabledVerticesArrayName ? "" : "(null)") << endl;

}

//----------------------------------------------------------------------------
vtkMTimeType vtkVgGraphMapper::GetMTime()
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
int vtkVgGraphMapper::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkGraph");
  return 1;
}

//----------------------------------------------------------------------------
double* vtkVgGraphMapper::GetBounds()
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
    graph->GetBounds(this->Bounds);
    }

  return this->Bounds;
}

#if 1
//----------------------------------------------------------------------------
void vtkVgGraphMapper::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
  // These filters share our input and are therefore involved in a
  // reference loop.
  //vtkGarbageCollectorReport(collector, this->GraphToPoly,
  //                          "GraphToPoly");
}

#endif
