// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVg2DGraphMapper_h
#define __vtkVg2DGraphMapper_h

#include "vtkSmartPointer.h"    // Required for smart pointer internal ivars.

#include <vgExport.h>

// VTK includes
#include <vtkMapper.h>
#include <vtkPolyData.h>

class vtkActor2D;
class vtkVgArcParallelEdgeStrategy;
class vtkCamera;
class vtkCellCenters;
class vtkDistanceToCamera;
class vtkEdgeLayout;
class vtkFollower;
class vtkGlyphSource2D;
class vtkGlyph3D;
class vtkGlyph3DMapper;
class vtkGraph;
class vtkGraphLayout;
class vtkGraphToPolyData;
class vtkIconGlyphFilter;
class vtkLabelPlacementMapper;
class vtkLookupTable;
class vtkMapArrayValues;
class vtkPassThroughEdgeStrategy;
class vtkPointSetToLabelHierarchy;
class vtkPointSource;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkPolyDataMapper2D;
class vtkTexture;
class vtkTexturedActor2D;
class vtkTransformCoordinateSystems;
class vtkVertexGlyphFilter;
class vtkViewTheme;

class vtkVgApplySelectedColor;
class vtkVgAssignCoordinatesLayoutStrategy;

// C++ includes.
#include <map>

class VTKVG_CORE_EXPORT vtkVg2DGraphMapper : public vtkMapper
{
public:
  static vtkVg2DGraphMapper* New();
  vtkTypeMacro(vtkVg2DGraphMapper, vtkMapper);
  void PrintSelf(ostream& os, vtkIndent indent);
  void Render(vtkRenderer* ren, vtkActor* act);

  // Description:
  // The array to use for positioning vertices.
  void SetVertexPositionArrayName(const char* name);
  const char* GetVertexPositionArrayName();

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

  void SetLabelColor(double color[3]);

  // Description:
  // Whether to edge glyphs.  Default is on.
  void SetEdgeGlyphVisibility(bool vis);
  bool GetEdgeGlyphVisibility();
  vtkBooleanMacro(EdgeGlyphVisibility, bool);

  void SetEdgeGlyphColor(double *color);

  // Description:
  // Get/Set the vertex point size
  vtkGetMacro(VertexPointSize, double);
  void SetVertexPointSize(double size);

  // Description:
  // Get/Set the edge line width
  vtkGetMacro(EdgeLineWidth, double);
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
  vtkMTimeType GetMTime();

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
  void SetVertexGeometry(vtkPolyData* geometry);
  vtkPolyData* GetVertexGeometry();

  // Description:
  // Set vertex opacity.
  void SetVertexOpacity(double value);

  // Description:
  // Set edge opacity.
  void SetEdgeOpacity(double value);

  // Description:
  // Return edge glyph source
  vtkGlyphSource2D* GetEdgeGlyphSource();

protected:
  vtkVg2DGraphMapper();
  ~vtkVg2DGraphMapper();

  virtual bool GetSupportsSelection()
    { return true; }

  // Description:
  // Used to store the vertex and edge color array names
  vtkGetStringMacro(VertexPositionArrayNameInternal);
  vtkSetStringMacro(VertexPositionArrayNameInternal);

  vtkGetStringMacro(VertexColorArrayNameInternal);
  vtkSetStringMacro(VertexColorArrayNameInternal);
  vtkGetStringMacro(EdgeColorArrayNameInternal);
  vtkSetStringMacro(EdgeColorArrayNameInternal);

  vtkGetStringMacro(VertexSelectionArrayNameInternal);
  vtkSetStringMacro(VertexSelectionArrayNameInternal);
  vtkGetStringMacro(EdgeSelectionArrayNameInternal);
  vtkSetStringMacro(EdgeSelectionArrayNameInternal);

  char* VertexPositionArrayNameInternal;
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
  vtkSmartPointer<vtkDistanceToCamera>           DistanceToCamera;
  vtkSmartPointer<vtkDistanceToCamera>           DistanceToCamera2;
  vtkSmartPointer<vtkGlyphSource2D>              EdgeGlyphSource;
  vtkSmartPointer<vtkGlyph3D>                    EdgeGlyph;
  vtkSmartPointer<vtkGraphToPolyData>            GraphToPoly;
  vtkSmartPointer<vtkVertexGlyphFilter>          VertexGlyph;
  vtkSmartPointer<vtkIconGlyphFilter>            IconGlyph;
  vtkSmartPointer<vtkMapArrayValues>             IconTypeToIndex;
  vtkSmartPointer<vtkTransformCoordinateSystems> IconTransform;
  vtkSmartPointer<vtkPointSetToLabelHierarchy>   PointsToLabels;

  vtkSmartPointer<vtkGraphLayout>                GraphLayout;
  vtkSmartPointer<vtkVgAssignCoordinatesLayoutStrategy>
    GraphLayoutStrategy;

  vtkSmartPointer<vtkEdgeLayout>                 EdgeLayout;
  vtkSmartPointer<vtkPassThroughEdgeStrategy>
  PassThroughEdgeStrategy;
  vtkSmartPointer<vtkVgArcParallelEdgeStrategy>
  ArcParallelEdgeStrategy;

  vtkSmartPointer<vtkPolyDataMapper>    EdgeMapper;
  vtkSmartPointer<vtkGlyph3DMapper>     VertexMapper;
  vtkSmartPointer<vtkPolyDataMapper2D>  IconMapper;
  vtkSmartPointer<vtkLabelPlacementMapper> LabelMapper;
  vtkSmartPointer<vtkPolyDataMapper> EdgeGlyphMapper;

  vtkSmartPointer<vtkActor>             EdgeActor;
  vtkSmartPointer<vtkActor>             VertexActor;
  vtkSmartPointer<vtkActor>             OutlineActor;
  vtkSmartPointer<vtkTexturedActor2D>   IconActor;
  vtkSmartPointer<vtkActor2D>           LabelActor;
  vtkSmartPointer<vtkActor> EdgeGlyphActor;

  vtkSmartPointer<vtkVgApplySelectedColor> ApplyColors;

  vtkSmartPointer<vtkPolyData> VertexGeometry;
  //ETX

  // Color maps
  vtkLookupTable* EdgeLookupTable;
  vtkLookupTable* VertexLookupTable;

  virtual void ReportReferences(vtkGarbageCollector*);

  // see algorithm for more info
  virtual int FillInputPortInformation(int port, vtkInformation* info);

private:
  vtkVg2DGraphMapper(const vtkVg2DGraphMapper&);  // Not implemented.
  void operator=(const vtkVg2DGraphMapper&);  // Not implemented.

  // Helper function
  vtkPolyData* CreateCircle(bool filled);

  double VertexPointSize;
  double EdgeLineWidth;
  bool ScaledGlyphs;
  char* ScalingArrayName;
};

#endif
