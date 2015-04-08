/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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

  double MarkerPosition[3];
  double DefaultColor[3];
  double SelectionColor[3];

  vtkVgMarker();
  virtual ~vtkVgMarker();

private:
  class vtkInternal;
  vtkInternal* Implementation;

  vtkVgMarker(const vtkVgMarker&);  // Not implemented.
  void operator=(const vtkVgMarker&);   // Not implemented.
};

#endif // __vtkVgMarker_h
