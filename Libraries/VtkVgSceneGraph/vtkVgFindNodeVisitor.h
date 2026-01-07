// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgFindNodeVisitor_h
#define __vtkVgFindNodeVisitor_h

#include "vtkVgNodeVisitor.h"

#include <vgExport.h>

class vtkVgVideoRepresentationBase0;
class vtkVgGeode;
class vtkVgVideoNode;
class vtkPropCollection;
class vtkProp3D;

//-----------------------------------------------------------------------------
class VTKVG_SCENEGRAPH_EXPORT vtkVgFindNodeVisitor : public vtkVgNodeVisitor
{
public:
  vtkVgFindNodeVisitor();
  virtual ~vtkVgFindNodeVisitor();

  // Description:
  // Overridden function. This traverse a video node.
  virtual void Visit(vtkVgVideoNode&     videoNode);
  virtual void Visit(vtkVgGeode&         geode);
  virtual void Visit(vtkVgGroupNode&     groupNode);
  virtual void Visit(vtkVgTransformNode& transformNode);
  virtual void Visit(vtkVgLeafNodeBase&  leafNode);

  // Description:
  // Set/Get \c vtkProp3D that is used as search criteria.
  void SetUsingProp3D(vtkProp3D* prop3D);
  vtkProp3D* GetUsingProp3D();
  const vtkProp3D* GetUsingProp3D() const;

  // Description:
  // Return the first node that found during the search
  // containing  \c vtkProp3D set using \c SetUsingProp3D(..).
  vtkVgNodeBase* GetNode();

private:
  vtkProp3D*       UsingProp3D;
  vtkVgNodeBase*   Node;

  vtkVgFindNodeVisitor(const vtkVgFindNodeVisitor&); // Not implemented.
  void operator= (const vtkVgFindNodeVisitor&);           // Not implemented.
};

#endif // __vtkVgFindNodeVisitor_h
