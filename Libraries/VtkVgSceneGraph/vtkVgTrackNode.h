// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVGTrackNode_h
#define __vtkVGTrackNode_h

#include "vtkVgLeafNodeBase.h"

// VisGUI includes
#include <vtkVgMacros.h>

#include <vgExport.h>

// Forward declarations.
class vtkVgNodeVisitorBase;
class vtkVgTrackModel;
class vtkVgTrackRepresentation;

class VTKVG_SCENEGRAPH_EXPORT vtkVgTrackNode : public vtkVgLeafNodeBase
{
public:

  // Description:
  // Standard macros.
  vtkVgClassMacro(vtkVgTrackNode);
  vtkTypeMacro(vtkVgTrackNode, vtkVgLeafNodeBase);

  // Description:
  // Print information on data members.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgTrackNode* New();

  // Description:
  // Get track model for the track node.
  const vtkVgTrackModel* GetTrackModel() const;
  vtkVgTrackModel*       GetTrackModel();
  void                   SetTrackModel(vtkVgTrackModel* trackModel);

  // Description:
  // Get track representation for the track node.
  const vtkVgTrackRepresentation* GetTrackRepresentation() const;
  vtkVgTrackRepresentation*       GetTrackRepresentation();
  void                            SetTrackRepresentation(
    vtkVgTrackRepresentation* trackRepresentation);

  virtual void ComputeBounds();

  virtual void Update(vtkVgNodeVisitorBase& nodeVisitor);

  virtual void UpdateRenderObjects(vtkVgPropCollection* propCollection);

  virtual void Accept(vtkVgNodeVisitorBase&  nodeVisitor);

  virtual void Traverse(vtkVgNodeVisitorBase& nodeVisitor);

protected:

  vtkVgTrackNode();
  virtual ~vtkVgTrackNode();

  vtkSmartPointer<vtkVgTrackModel>          TrackModel;
  vtkSmartPointer<vtkVgTrackRepresentation> TrackRepresentation;

private:

  // Not implemented.
  vtkVgTrackNode(const vtkVgTrackNode&);

  // Not implemented.
  void operator= (const vtkVgTrackNode&);
};

#endif // __vtkVGTrackNode_h
