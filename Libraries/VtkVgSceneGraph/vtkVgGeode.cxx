/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// VG includes.
#include "vtkVgComputeBounds.h"
#include "vtkVgGeode.h"
#include "vtkVgGroupNode.h"
#include "vtkVgNodeVisitorBase.h"
#include "vtkVgPropCollection.h"
#include "vtkVgSceneManager.h"

// VTK includes.
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkProp.h>
#include <vtkPropCollection.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkVgGeode);

//-----------------------------------------------------------------------------
vtkVgGeode::vtkVgGeode() : vtkVgLeafNodeBase()
{
  this->ActiveDrawables  = vtkSmartPointer<vtkPropCollection>::New();
  this->NewDrawables     = vtkSmartPointer<vtkPropCollection>::New();
  this->ExpiredDrawables = vtkSmartPointer<vtkPropCollection>::New();
}

//-----------------------------------------------------------------------------
vtkVgGeode::~vtkVgGeode()
{
}

//-----------------------------------------------------------------------------
void vtkVgGeode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
int vtkVgGeode::SetVisible(int flag)
{
  if (!this->Superclass::SetVisible(flag))
    {
    return 0;
    }

  this->ActiveDrawables->InitTraversal();

  int numberOfItems =  this->ActiveDrawables->GetNumberOfItems();
  for (int i = 0; i < numberOfItems; ++i)
    {
    // \TODO: We are dealing with only 3d props. We need to find a way for
    // setting a user transform to vtkProp if needed.
    vtkProp3D* currentProp =
      static_cast<vtkProp3D*>(this->ActiveDrawables->GetNextProp());
    if (currentProp)
      {
      currentProp->SetVisibility(flag);
      }
    }

  return 1;
}

//-----------------------------------------------------------------------------
void vtkVgGeode::AddDrawable(vtkProp* prop)
{
  if (prop)
    {
    this->ActiveDrawables->AddItem(prop);

    this->NewDrawables->AddItem(prop);

    this->BoundsDirtyOn();
    }
}

//-----------------------------------------------------------------------------
void vtkVgGeode::RemoveDrawable(vtkProp* prop)
{
  if (prop)
    {
    this->ActiveDrawables->RemoveItem(prop);
    this->ExpiredDrawables->AddItem(prop);

    this->BoundsDirtyOn();
    }
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgGeode::GetActiveDrawables()
{
  return this->ActiveDrawables;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgGeode::GetActiveDrawables() const
{
  return this->ActiveDrawables;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgGeode::GetNewDrawables()
{
  return this->NewDrawables;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgGeode::GetNewDrawables() const
{
  return this->NewDrawables;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgGeode::GetExpiredDrawables()
{
  return this->ExpiredDrawables;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgGeode::GetExpiredDrawables() const
{
  return this->ExpiredDrawables;
}

//-----------------------------------------------------------------------------
void vtkVgGeode::Update(vtkVgNodeVisitorBase& nodeVisitor)
{
  this->UpdateRenderObjects(nodeVisitor.GetPropCollection());

  if (((this->NodeReferenceFrame ==
        vtkVgNodeBase::RELATIVE_REFERENCE) && this->Parent && this->Parent->GetDirty()) ||
      ((this->NodeReferenceFrame ==
        vtkVgNodeBase::RELATIVE_REFERENCE) && this->Parent && this->Dirty))
    {
    this->FinalMatrix->DeepCopy(this->Parent->GetFinalMatrix());

    this->ActiveDrawables->InitTraversal();
    for (int i = 0; i < this->ActiveDrawables->GetNumberOfItems(); ++i)
      {
      // \TODO: We are dealing with only 3d props. We need to find a way for
      // setting a user transform to vtkProp if needed.
      vtkProp3D* currentProp = static_cast<vtkProp3D*>(this->ActiveDrawables->GetNextProp());
      if (currentProp)
        {
        double w = vtkMath::Norm(this->FinalMatrix->Element[3], 4);
        for (int i = 0; i < 4; ++i)
          {
          for (int j = 0; j < 4; ++j)
            {
            this->FinalMatrix->Element[i][j] /= w;
            }
          }

        currentProp->SetUserMatrix(this->FinalMatrix);
        }
      }

    this->BoundsDirtyOn();
    }

  if (this->GetBoundsDirty())
    {
    this->ComputeBounds();
    }
}

//-----------------------------------------------------------------------------
void vtkVgGeode::UpdateRenderObjects(vtkVgPropCollection* propCollection)
{
  if (this->GetRemoveNode())
    {
    propCollection->AddExpired(this->GetActiveDrawables());

    // Since we removed the active render objects, we would need to copy them to the
    // new render objects to have them shown up if this node is added to SG again.
    this->ActiveDrawables->InitTraversal();
    int numberOfItems = this->ActiveDrawables->GetNumberOfItems();
    for (int i = 0; i < numberOfItems; ++i)
      {
      this->NewDrawables->AddItem(this->ActiveDrawables->GetNextProp());
      }

    this->ActiveDrawables->RemoveAllItems();
    this->RemoveNodeOff();
    }
  else
    {
    propCollection->AddNew(this->NewDrawables);
    propCollection->AddExpired(this->ExpiredDrawables);

    this->NewDrawables->RemoveAllItems();
    this->ExpiredDrawables->RemoveAllItems();
    }
}

//-----------------------------------------------------------------------------
void vtkVgGeode::Accept(vtkVgNodeVisitorBase& nodeVisitor)
{
  nodeVisitor.Visit(*this);
}

//-----------------------------------------------------------------------------
void vtkVgGeode::Traverse(vtkVgNodeVisitorBase& nodeVisitor)
{
  // Then call base class Traverse which calls Update on this.
  this->Superclass::Traverse(nodeVisitor);

  if (this->BoundsDirty)
    {
    this->ComputeBounds();
    }

  this->DirtyOff();
}

//-----------------------------------------------------------------------------
void vtkVgGeode::ComputeBounds()
{
  vtkVgComputeBounds::ComputeVisiblePropBounds(this->GetActiveDrawables(),
                                               this->Bounds);

  if (vtkMath::AreBoundsInitialized(this->Bounds) ||
      (this->GetActiveDrawables()->GetNumberOfItems() < 1))
    {
    this->BoundsDirtyOff();
    }
}
