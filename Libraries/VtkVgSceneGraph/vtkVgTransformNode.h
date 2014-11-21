/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgTransformNode_h
#define __vtkVgTransformNode_h

// VG includes.
#include "vtkVgGroupNode.h"
#include "vtkVgMacros.h"

#include <vgExport.h>

// Forward declarations.
class vtkMatrix4x4;

class vtkVgTimeStamp;

class VTKVG_SCENEGRAPH_EXPORT vtkVgTransformNode : public vtkVgGroupNode
{
public:
  // Description:
  // Usual VTK functions.
  vtkVgClassMacro(vtkVgTransformNode);

  vtkTypeMacro(vtkVgTransformNode, vtkVgGroupNode);

  static vtkVgTransformNode* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Descrption:
  // Set/Get transformation matrix.
  void SetMatrix(vtkMatrix4x4* matrix, bool preMultiply = false);
  vtkMatrix4x4* GetMatrix();
  const vtkMatrix4x4* GetMatrix() const;

  // Descrption:
  // Pre/Post multiply existing transformation matrix using \param matrix.
  void PreMultiplyMatrix(vtkMatrix4x4* matrix);
  void PostMultiplyMatrix(vtkMatrix4x4* matrix);

  // Descrption:
  // \TODO: Not implemented yet.
  const vtkMatrix4x4* GetInverseInternalMatrix() const {return NULL;}

  // Descrption:
  // \TODO: Not implemented yet.
  virtual bool ComputeLocalToWorldMatrix(vtkMatrix4x4*) const {return false;}
  virtual bool ComputeWorldToLocalMatrix(vtkMatrix4x4*) const {return false;}


  // Descrption:
  // Overridden functions.
  virtual void Update(vtkVgNodeVisitorBase& nodeVisitor);

  virtual void Accept(vtkVgNodeVisitorBase& nodeVisitor);

  virtual void Traverse(vtkVgNodeVisitorBase& nodeVisitor);


protected:

  bool                          PreMultiply;
  vtkSmartPointer<vtkMatrix4x4> Matrix;

  vtkVgTransformNode();
  virtual ~vtkVgTransformNode();

private:
  vtkVgTransformNode(const vtkVgTransformNode&); // Not implemented.
  void operator= (const vtkVgTransformNode&);    // Not implemented.

};

#endif // __vtkVgTransformNode_h
