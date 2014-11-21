/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgNodeVisitorBase_h
#define __vtkVgNodeVisitorBase_h

// VTK includes.
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkTimeStamp.h>

// VG includes.
#include "vtkVgTimeStamp.h"

#include <vgExport.h>

// Forward declarations.
class vtkVgGeode;
class vtkVgGroupNode;
class vtkVgLeafNodeBase;
class vtkVgNodeBase;
class vtkVgTransformNode;
class vtkVgVideoNode;


class vtkVgPropCollection;

class VTKVG_SCENEGRAPH_EXPORT vtkVgNodeVisitorBase
{
public:
  vtkVgNodeVisitorBase();
  virtual ~vtkVgNodeVisitorBase();


  // Description:
  // Types of node visitor.
  enum VisitorType
    {
    NODE_VISITOR = 0,
    UPDATE_VISITOR,
    EVENT_VISITOR,
    COLLECT_OCCLUDER_VISITOR,
    CULL_VISITOR
    };

  // Description:
  // Set the VisitorType, used to distinguish different visitors during
  // traversal of the scene, typically used in the \c Node::Traverse() method
  // to decided what action to take.
  void SetVisitorType(VisitorType type) {this->NodeVisitorType = type; }

  // Description:
  // Get the VisitorType.
  VisitorType GetVisitorType() const { return this->NodeVisitorType; }

  // Description:
  // Start traversing a particular node type.
  // Derived class need to implement this as needed.
  virtual void Visit(vtkVgGeode& vtkNotUsed(geode)) {;}
  virtual void Visit(vtkVgGroupNode& vtkNotUsed(groupNode)) {;}
  virtual void Visit(vtkVgTransformNode& vtkNotUsed(transformNode)) {;}
  virtual void Visit(vtkVgVideoNode& vtkNotUsed(videoNode)) {;}
  virtual void Visit(vtkVgLeafNodeBase& vtkNotUsed(leafNode)) {;}

  // Description:
  // Traverse a node.
  virtual void Traverse(vtkVgNodeBase& node);


  void SetTimeStamp(const vtkVgTimeStamp& timeStamp);
  const vtkVgTimeStamp& GetTimeStamp() const;
  vtkVgTimeStamp& GetTimeStamp();

  void SetPropCollection(vtkVgPropCollection* propCollection);
  const vtkVgPropCollection* GetPropCollection() const;
  vtkVgPropCollection* GetPropCollection();

  virtual void ShallowCopy(vtkVgNodeVisitorBase* other);


protected:

  VisitorType                           NodeVisitorType;

  vtkVgTimeStamp                        TimeStamp;

  vtkTimeStamp                          ModifiedTimeStamp;

  vtkSmartPointer<vtkVgPropCollection>  PropCollection;


private:
  vtkVgNodeVisitorBase(const vtkVgNodeVisitorBase&); // Not implemented.
  void operator= (const vtkVgNodeVisitorBase&);      // Not implemented.
};

#endif //  __vtkVgNodeVisitorBase_h
