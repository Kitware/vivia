// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgMarker_h
#define __vtkVgMarker_h

// VisGUI includes
#include <vtkVgMacros.h>
#include <vgExport.h>

// VTK includes
#include <vtkOpenGLActor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

// Forward declarations
class vtkPoints;
class vtkRenderer;

// Single entity that represents a video with optional child models.
class VTKVG_MODELVIEW_EXPORT vtkVgMarker : public vtkOpenGLActor
{
public:
  vtkVgClassMacro(vtkVgMarker);
  vtkTypeMacro(vtkVgMarker, vtkOpenGLActor);

  // Description:
  // Use \c New() to create an instance of \c vtkVgMarker.
  static vtkVgMarker* New();

  // Description:
  // Print values of data members.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set Id which identifies the marker.
  //
  // The default value of id is -1.
  vtkGetMacro(Id, vtkIdType);
  vtkSetMacro(Id, vtkIdType);

  // Description:
  // Get/Set Type which describes the marker.
  //
  // The default value of Type is -1.
  vtkGetMacro(Type, int);
  vtkSetMacro(Type, int);

  // Description:
  // Get/Set marker position in 3d space.
  //
  // This is different than setting the actors position
  // as the marker position modifies the actual data
  // contained.
  vtkGetVector3Macro(MarkerPosition, double);
  void SetMarkerPosition(double* pos);

  // Description:
  // Get/Set the default color (r, g, b) for the marker.
  //
  // Default is yellow color (1, 1, 0).
  vtkGetVector3Macro(DefaultColor, double);
  vtkSetVector3Macro(DefaultColor, double);

  // Description:
  // Get/Set the selection color (r, g, b) for the marker.
  //
  // Default is reddish color (1, 0.1, 0.1).
  vtkGetVector3Macro(SelectionColor, double);
  vtkSetVector3Macro(SelectionColor, double);

  enum MarkerSizeModeType
    {
    MSM_FixedScreenSize,    // Fixed screen size, regardless of zoom level
    MSM_SceneCoordinates    // Size specified in coordinates of the scene
    };

  // Description:
  // Set/Get the mode for determining marker size
  //
  // Default is MSM_FixedScreenSize.
  void SetMarkerSizeMode(MarkerSizeModeType sizeMode);
  void SetMarkerSizeModeToFixedScreenSize()
    {
    this->SetMarkerSizeMode(MSM_FixedScreenSize);
    }
  void SetMarkerSizeModeToSceneCoordinates()
    {
    this->SetMarkerSizeMode(MSM_SceneCoordinates);
    }
  vtkGetMacro(MarkerSizeMode, MarkerSizeModeType);
  const char *GetMarkerSizeModeAsString();

  // Description:
  // Get/Set the marker size - depending on the MarkerSizeMode, the value is
  // either used as a screen size (MSM_FixedScreenSize) or as the circle radius
  // specified in coordinates of the scene (MSM_SceneCoordinates).
  //
  // Default is 10.0.
  void SetMarkerSize(double markerSize);
  vtkGetMacro(MarkerSize, double);

  // Description
  // Get/Set the set of points defining a region (polygon) to represent the
  // marker. If set to 0 (default), a glyph /circle shape is used.
  virtual void SetRegionPoints(vtkPoints*);
  vtkGetObjectMacro(RegionPoints, vtkPoints);

  // Description:
  // Set the renderer.
  //
  // Renderer is needed to ensure fixed pixel size for the markers.
  virtual void SetRenderer(vtkRenderer* ren);

  // Description:
  // Select / undo select (highlight) the marker.
  void Select() { this->SetSelected(true); }
  void Deselect() { this->SetSelected(false); }

  // Description:
  // Get/Set current state of marker selection.
  bool IsSelected();
  vtkSetMacro(Selected, bool)

  // Description:
  // Render this marker.
  void Render(vtkRenderer* ren, vtkMapper* mapper);

protected:
  vtkIdType Id;
  int Type;

  bool Selected;

  MarkerSizeModeType MarkerSizeMode;
  double MarkerSize;

  double MarkerPosition[3];
  double DefaultColor[3];
  double SelectionColor[3];

  vtkPoints* RegionPoints;

  vtkVgMarker();
  virtual ~vtkVgMarker();

private:
  class vtkInternal;
  vtkInternal* Implementation;

  vtkVgMarker(const vtkVgMarker&);  // Not implemented.
  void operator=(const vtkVgMarker&);   // Not implemented.
};

//BTX
inline const char* vtkVgMarker::GetMarkerSizeModeAsString()
  {
  switch (this->MarkerSizeMode)
    {
    case MSM_FixedScreenSize:
      return "FixedScreenSize";
    case MSM_SceneCoordinates:
      return "SceneCoordinates";
    default:
      return "Unrecognized";
    }
  }
//ETX
#endif // __vtkVgMarker_h
