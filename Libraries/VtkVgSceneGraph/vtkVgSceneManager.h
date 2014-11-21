/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgSceneManager_h
#define __vtkVgSceneManager_h

// VG includes.
#include "vtkVgMacros.h"

// VTK includes.
#include <vtkObject.h>
#include <vtkSmartPointer.h>

#include <vgExport.h>

// Forward declarations.
class vtkVgGroupNode;
class vtkVgTimeStamp;
class vtkVgNodeVisitorBase;
class vtkVgNodeBase;

class vtkAbstractPropPicker;;
class vtkRenderer;

//-----------------------------------------------------------------------------
class VTKVG_SCENEGRAPH_EXPORT vtkVgSceneManager : public vtkObject
{
public:
  // Description:
  // Define easy to use types.
  vtkVgClassMacro(vtkVgSceneManager);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgSceneManager, vtkObject);

  static vtkVgSceneManager* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // \c SceneManager creates its own renderer.This is important
  // As nodes expect that scene manager is going to have only one
  // scene renderer and they attach all their render objects (vtkActor)
  // to it.
  void SetSceneRenderer(vtkRenderer* renderer);
  vtkRenderer* GetSceneRenderer();
  const vtkRenderer* GetSceneRenderer() const;

  // Description:
  // Set/Get scene root. This is the top most node in the
  // scene tree.
  void SetSceneRoot(vtkVgNodeBase* root);
  vtkVgNodeBase* GetSceneRoot();
  const vtkVgNodeBase* GetSceneRoot() const;

  // Description:
  // Set node visitor that will be used for scene traversal.
  // Don't set this unless required.
  void SetNodeVisitor(vtkVgNodeVisitorBase* nodeVisitor);
  const vtkVgNodeVisitorBase* GetNodeVisitor() const;

  // Description:
  // Update scene using \param timeStamp.
  void Update(const vtkVgTimeStamp& timeStamp);

  // Description:
  // Pick a node in this view.
  virtual vtkVgNodeBase* Pick(const double& x, const double& y, const double& z);

protected:
  vtkVgSceneManager();
  virtual ~vtkVgSceneManager();

  // Description:
  //
  void Initialize();

  bool                                        Initialized;

  vtkSmartPointer<vtkVgNodeBase>              SceneRoot;
  vtkSmartPointer<vtkRenderer>                SceneRenderer;

  vtkVgNodeVisitorBase*                        NodeVisitor;

  vtkSmartPointer<vtkAbstractPropPicker>      ScenePicker;

private:
  vtkVgSceneManager(const vtkVgSceneManager&);
  void operator= (vtkVgSceneManager&);
};

#endif // __vtkVgSceneManager_h
