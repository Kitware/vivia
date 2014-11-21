/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVgGraphMapper.h

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
// .NAME vtkVgGraphMapper - map vtkGraph and derived
// classes to graphics primitives

// .SECTION Description
// vtkVgGraphMapper is a mapper to map vtkGraph
// (and all derived classes) to graphics primitives.

#ifndef __vtkVgGraphMapper_h
#define __vtkVgGraphMapper_h

#include "vtkMapper.h"

#include "vtkSmartPointer.h"    // Required for smart pointer internal ivars.

#include <vgExport.h>

class vtkActor2D;
class vtkVgArcParallelEdgeStrategy;
class vtkCamera;
class vtkCellCenters;
class vtkEdgeLayout;
class vtkFollower;
class vtkGlyph3D;
class vtkGlyph3DMapper;
class vtkGraph;
class vtkGraphToPolyData;
class vtkIconGlyphFilter;
class vtkLabelPlacementMapper;
class vtkLookupTable;
class vtkMapArrayValues;
class vtkPassThroughEdgeStrategy;
class vtkPointSetToLabelHierarchy;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkPolyDataMapper2D;
class vtkTexture;
class vtkTexturedActor2D;
class vtkTransformCoordinateSystems;
class vtkVertexGlyphFilter;
class vtkViewTheme;

class vtkVgApplySelectedColor;

// C++ includes.
#include <map>

class VTKVG_CORE_EXPORT vtkVgGraphMapper : public vtkMapper
{
public:
  static vtkVgGraphMapper* New();
  vtkTypeMacro(vtkVgGraphMapper, vtkMapper);
  void PrintSelf(ostream& os, vtkIndent indent);
  void Render(vtkRenderer* ren, vtkActor* act);

  // Description:
  // The array to use for coloring vertices.  Default is "color".
  void SetVertexColorArrayName(const char* name);
  const char* GetVertexColorArrayName();

  void SetVertexSelectionArrayName(const char* name);
  const char* GetVertexSelectionArrayName();

  // Description:
  // The array to use for labeling vertices.
  void SetLabelArrayName(const char* name);
  const char* GetLabelArrayName();

  // Description:
  // Whether to use vertex selection array.  Default is off.
  void SetUseVertexSelection(bool enable);
  bool GetUseVertexSelection();

  // Description:
  // Whether to color vertices.  Default is off.
  void SetColorVertices(bool vis);
  bool GetColorVertices();

  // Description:
  // Whether scaled glyphs are on or not.  Default is off.
  // By default this mapper uses vertex glyphs that do not
  // scale. If you turn this option on you will get circles
  // at each vertex and they will scale as you zoom in/out.
  void SetScaledGlyphs(bool arg);
  vtkGetMacro(ScaledGlyphs, bool);
  vtkBooleanMacro(ScaledGlyphs, bool);

  // Description:
  // Glyph scaling array name. Default is "scale"
  vtkSetStringMacro(ScalingArrayName);
  vtkGetStringMacro(ScalingArrayName);

  // Description:
  // Whether to show vertices or not.  Default is on.
  void SetVertexVisibility(bool vis);
  bool GetVertexVisibility();
  vtkBooleanMacro(VertexVisibility, bool);

  // Description:
  // Whether to show edges or not.  Default is on.
  void SetEdgeVisibility(bool vis);
  bool GetEdgeVisibility();
  vtkBooleanMacro(EdgeVisibility, bool);

  // Description:
  // The array to use for coloring edges.  Default is "color".
  void SetEdgeColorArrayName(const char* name);
  const char* GetEdgeColorArrayName();

  void SetEdgeSelectionArrayName(const char* name);
  const char* GetEdgeSelectionArrayName();

  // Description:
  // Whether to use edge selection array.  Default is off.
  void SetUseEdgeSelection(bool enable);
  bool GetUseEdgeSelection();

  // Description:
  // Whether to color edges.  Default is off.
  void SetColorEdges(bool vis);
  bool GetColorEdges();

  // Description:
  // The array to use for coloring edges.  Default is "color".
  vtkSetStringMacro(EnabledEdgesArrayName);
  vtkGetStringMacro(EnabledEdgesArrayName);

  // Description:
  // Whether to enable/disable edges using array values.  Default is off.
  vtkSetMacro(EnableEdgesByArray, int);
  vtkGetMacro(EnableEdgesByArray, int);
  vtkBooleanMacro(EnableEdgesByArray, int);

  // Description:
  // The array to use for coloring edges.  Default is "color".
  vtkSetStringMacro(EnabledVerticesArrayName);
  vtkGetStringMacro(EnabledVerticesArrayName);

  // Description:
  // Whether to enable/disable vertices using array values.  Default is off.
  vtkSetMacro(EnableVerticesByArray, int);
  vtkGetMacro(EnableVerticesByArray, int);
  vtkBooleanMacro(EnableVerticesByArray, int);

  // Description:
  // Whether to layout edges or just pass through
  void SetUseEdgeLayout(bool enable);

  // Description:
  // The array to use for assigning icons.
  void SetIconArrayName(const char* name);
  const char* GetIconArrayName();

  // Description:
  // Associate the icon at index "index" in the vtkTexture to all vertices
  // containing "type" as a value in the vertex attribute array specified by
  // IconArrayName.
  void AddIconType(char* type, int index);

  // Description:
  // Clear all icon mappings.
  void ClearIconTypes();

  // Description:
  // Specify the Width and Height, in pixels, of an icon in the icon sheet.
  void SetIconSize(int* size);
  int* GetIconSize();

  // Description:
  // Specify where the icons should be placed in relation to the vertex.
  // See vtkIconGlyphFilter.h for possible values.
  void SetIconAlignment(int alignment);

  // Description:
  // The texture containing the icon sheet.
  vtkTexture* GetIconTexture();
  void SetIconTexture(vtkTexture* texture);

  // Description:
  // Whether to show icons.  Default is off.
  void SetIconVisibility(bool vis);
  bool GetIconVisibility();
  vtkBooleanMacro(IconVisibility, bool);

  // Description:
  // Whether to show labels.  Default is off.
  void SetLabelVisibility(bool vis);
  bool GetLabelVisibility();
  vtkBooleanMacro(LabelVisibility, bool);

  // Description:
  // Get/Set the vertex point size
  vtkGetMacro(VertexPointSize, float);
  void SetVertexPointSize(float size);

  // Description:
  // Get/Set the edge line width
  vtkGetMacro(EdgeLineWidth, float);
  void SetEdgeLineWidth(float width);

  // Description:
  // Apply the theme to this view.
  virtual void ApplyViewTheme(vtkViewTheme* theme);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow*);

  // Description:
  // Get the mtime also considering the lookup table.
  unsigned long GetMTime();

  // Description:
  // Set the Input of this mapper.
  void SetInputData(vtkGraph* input);
  vtkGraph* GetInput();

  // Description:
  // Return bounding box (array of six doubles) of data expressed as
  // (xmin,xmax, ymin,ymax, zmin,zmax).
  virtual double* GetBounds();
  virtual void GetBounds(double* bounds)
    { Superclass::GetBounds(bounds); }

  // Description:
  // Access to the lookup tables used by the vertex and edge mappers.
  vtkGetObjectMacro(EdgeLookupTable, vtkLookupTable);
  vtkGetObjectMacro(VertexLookupTable, vtkLookupTable);

  // Description:
  // Set/Get polydata used for representing vertex.
  void SetVertexGeometry(vtkPolyData* vertexGeometry);
  vtkSmartPointer<vtkPolyData> GetVertexGeometry();

  // Description:
  // Set/Get polydata per index basis.
  void SetVertexGeometry(vtkIdType index, vtkPolyData* vertexGeometry);
  vtkSmartPointer<vtkPolyData> GetVertexGeometry(vtkIdType index);

  // Description:
  // Set/Get geometry index array.
  void SetVertexGeometryIndexArray(const char* arrayname);

  // Description:
  // Convenience method to set the array to use as index within the sources.
  void SetVertexGeometryIndexArray(int fieldAttributeType);

  //  Description:
  // Enable/disable indexing into table of the glyph sources. When disabled,
  // only the 1st source input will be used to generate the glyph. Otherwise the
  // source index array will be used to select the glyph source. The source
  // index array can be specified using SetSourceIndexArray().
  void SetVertexGeometryIndexing(int value);
  void GetVertexGeometryIndexing(int& value);

  // Description:
  // Set number of sources for geometry.
  void SetNumberOfVertexGeometrySources(int number);
  void GetNumberOfVertexGeometrySources(int& number);

  // Description:
  // Set edge opacity.
  void SetEdgeOpacity(double value);


protected:
  vtkVgGraphMapper();
  ~vtkVgGraphMapper();

  virtual bool GetSupportsSelection()
    { return true; }

  // Description:
  // Used to store the vertex and edge color array names
  vtkGetStringMacro(VertexColorArrayNameInternal);
  vtkSetStringMacro(VertexColorArrayNameInternal);
  vtkGetStringMacro(EdgeColorArrayNameInternal);
  vtkSetStringMacro(EdgeColorArrayNameInternal);

  vtkGetStringMacro(VertexSelectionArrayNameInternal);
  vtkSetStringMacro(VertexSelectionArrayNameInternal);
  vtkGetStringMacro(EdgeSelectionArrayNameInternal);
  vtkSetStringMacro(EdgeSelectionArrayNameInternal);

  int GeometryIndexing;

  char* VertexColorArrayNameInternal;
  char* VertexSelectionArrayNameInternal;

  char* EdgeColorArrayNameInternal;
  char* EdgeSelectionArrayNameInternal;

  char* EnabledEdgesArrayName;
  char* EnabledVerticesArrayName;
  int EnableEdgesByArray;
  int EnableVerticesByArray;

  vtkGetStringMacro(IconArrayNameInternal);
  vtkSetStringMacro(IconArrayNameInternal);
  char* IconArrayNameInternal;

  //BTX
  vtkSmartPointer<vtkGlyph3D>                    VertexGlyph3d;
  vtkSmartPointer<vtkGlyph3D>                    CircleOutlineGlyph;

  vtkSmartPointer<vtkGraphToPolyData>            GraphToPoly;
  vtkSmartPointer<vtkVertexGlyphFilter>          VertexGlyph;
  vtkSmartPointer<vtkIconGlyphFilter>            IconGlyph;
  vtkSmartPointer<vtkMapArrayValues>             IconTypeToIndex;
  vtkSmartPointer<vtkTransformCoordinateSystems> IconTransform;
  vtkSmartPointer<vtkPointSetToLabelHierarchy>   PointsToLabels;

  vtkSmartPointer<vtkEdgeLayout>                 EdgeLayout;
  vtkSmartPointer<vtkPassThroughEdgeStrategy>
  PassThroughEdgeStrategy;
  vtkSmartPointer<vtkVgArcParallelEdgeStrategy>
  ArcParallelEdgeStrategy;

  vtkSmartPointer<vtkPolyDataMapper>    EdgeMapper;
  vtkSmartPointer<vtkGlyph3DMapper>     VertexMapper;
  vtkSmartPointer<vtkPolyDataMapper2D>  IconMapper;
  vtkSmartPointer<vtkLabelPlacementMapper> LabelMapper;

  vtkSmartPointer<vtkActor>             EdgeActor;
  vtkSmartPointer<vtkActor>             VertexActor;
  vtkSmartPointer<vtkActor>             OutlineActor;
  vtkSmartPointer<vtkTexturedActor2D>   IconActor;
  vtkSmartPointer<vtkActor2D>           LabelActor;

  vtkSmartPointer<vtkVgApplySelectedColor> ApplyColors;

  std::map<vtkIdType, vtkSmartPointer<vtkPolyData> > VertexGeometries;
  //ETX

  // Color maps
  vtkLookupTable* EdgeLookupTable;
  vtkLookupTable* VertexLookupTable;

  virtual void ReportReferences(vtkGarbageCollector*);

  // see algorithm for more info
  virtual int FillInputPortInformation(int port, vtkInformation* info);

private:
  vtkVgGraphMapper(const vtkVgGraphMapper&);  // Not implemented.
  void operator=(const vtkVgGraphMapper&);  // Not implemented.

  // Helper function
  vtkPolyData* CreateCircle(bool filled);

  float VertexPointSize;
  float EdgeLineWidth;
  bool ScaledGlyphs;
  char* ScalingArrayName;
};

#endif
