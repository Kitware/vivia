/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgFindNodeVisitor.h"
#include "vtkVgGroupNode.h"
#include "vtkVgNodeBase.h"
#include "vtkVgNodeVisitor.h"
#include "vtkVgPropCollection.h"
#include "vtkVgRenderer.h"
#include "vtkVgSceneManager.h"
#include "vtkVgTimeStamp.h"

// VTK includes.
#include <vtkObjectFactory.h>
#include <vtkPropPicker.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkVgSceneManager);

//-----------------------------------------------------------------------------
vtkVgSceneManager::vtkVgSceneManager() : vtkObject(),
  Initialized(false),
  SceneRoot(NULL),
  NodeVisitor(NULL)
{
  this->SceneRenderer = vtkSmartPointer<vtkVgRenderer>::New();
  this->ScenePicker   = vtkSmartPointer<vtkPropPicker>::New();
}

//-----------------------------------------------------------------------------
vtkVgSceneManager::~vtkVgSceneManager()
{
  delete this->NodeVisitor;
  this->NodeVisitor = 0;
}

//-----------------------------------------------------------------------------
void vtkVgSceneManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgSceneManager::SetSceneRenderer(vtkRenderer* renderer)
{
  if (renderer && (this->SceneRenderer != renderer))
    {
    this->SceneRenderer = renderer;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
const vtkRenderer* vtkVgSceneManager::GetSceneRenderer() const
{
  return this->SceneRenderer;
}

//-----------------------------------------------------------------------------
vtkRenderer* vtkVgSceneManager::GetSceneRenderer()
{
  return this->SceneRenderer;
}

//-----------------------------------------------------------------------------
void vtkVgSceneManager::SetSceneRoot(vtkVgNodeBase* root)
{
  if (root && (this->SceneRoot != root))
    {
    this->SceneRenderer->RemoveAllViewProps();

    if (this->NodeVisitor && this->NodeVisitor->GetPropCollection())
      {
      this->NodeVisitor->GetPropCollection()->ResetComplete();
      }

    this->SceneRoot = root;
    }
}

//-----------------------------------------------------------------------------
vtkVgNodeBase* vtkVgSceneManager::GetSceneRoot()
{
  return this->SceneRoot;
}

//-----------------------------------------------------------------------------
const vtkVgNodeBase* vtkVgSceneManager::GetSceneRoot() const
{
  return this->SceneRoot;
}

//-----------------------------------------------------------------------------
void vtkVgSceneManager::SetNodeVisitor(vtkVgNodeVisitorBase* nodeVisitor)
{
  if (nodeVisitor)
    {
    this->NodeVisitor->ShallowCopy(nodeVisitor);
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
const vtkVgNodeVisitorBase* vtkVgSceneManager::GetNodeVisitor() const
{
  return this->NodeVisitor;
}

//-----------------------------------------------------------------------------
void vtkVgSceneManager::Update(const vtkVgTimeStamp& timeStamp)
{
  if (!this->Initialized)
    {
    this->Initialize();
    }

  // Set the timestamp on node visitor.
  this->NodeVisitor->SetTimeStamp(timeStamp);

  // Traverse the scene.
  this->SceneRoot->Accept(*this->NodeVisitor);

  // Reset and add new props or remove expired props.
  vtkVgPropCollection* propCollection = this->NodeVisitor->GetPropCollection();
  if (propCollection->GetDirty())
    {
    if (propCollection->GetDirtyCache())
      {
      this->SceneRenderer->RemoveAllViewProps();

      vtkVgPropCollection::ConstItertor beginItr =
        propCollection->GetActiveProps().begin();
      vtkVgPropCollection::ConstItertor endItr =
        propCollection->GetActiveProps().end();

      while (beginItr != endItr)
        {
        vtkVgPropCollection::Props props = beginItr->second;
        for (size_t i = 0; i < props.size(); ++i)
          {
          this->SceneRenderer->AddViewProp(props[i]);
          this->ScenePicker->AddPickList(props[i]);
          }
        ++beginItr;
        }
      }
    else
      {
      vtkVgPropCollection::ConstItertor beginItr =
        propCollection->GetExpiredProps().begin();
      vtkVgPropCollection::ConstItertor endItr =
        propCollection->GetExpiredProps().end();

      while (beginItr != endItr)
        {
        vtkVgPropCollection::Props props = beginItr->second;
        for (size_t i = 0; i < props.size(); ++i)
          {
          this->SceneRenderer->RemoveViewProp(props[i]);
          this->ScenePicker->DeletePickList(props[i]);
          }
        ++beginItr;
        }
      }

    propCollection->Reset();
    }
}

//-----------------------------------------------------------------------------
void vtkVgSceneManager::Initialize()
{
  // Create a default one.
  if (!this->NodeVisitor)
    {
    this->NodeVisitor = new vtkVgNodeVisitor();
    this->NodeVisitor->SetVisitorType(vtkVgNodeVisitorBase::UPDATE_VISITOR);
    this->NodeVisitor->SetPropCollection(vtkVgPropCollection::SmartPtr::New());
    }

  this->Initialized = true;
}

//-----------------------------------------------------------------------------
vtkVgNodeBase* vtkVgSceneManager::Pick(const double& x, const double& y, const double& z)
{
  if (this->ScenePicker->Pick(x, y, z, this->SceneRenderer))
    {
    vtkProp3D* prop3D = this->ScenePicker->GetProp3D();

    vtkVgFindNodeVisitor findNodeVisitor;
    findNodeVisitor.SetUsingProp3D(prop3D);

    this->SceneRoot->Accept(findNodeVisitor);

    return findNodeVisitor.GetNode();
    }

  return NULL;
}
