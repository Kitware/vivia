/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgLeafNodeBase.h"

// SceneGraph includes.
#include "vtkVgPropCollection.h"
#include "vtkVgRepresentationBase.h"

//-----------------------------------------------------------------------------
void vtkVgLeafNodeBase::PrepareForAddition(
  vtkVgPropCollection* propCollection, vtkVgRepresentationBase* representation)
{
  if (!representation || !propCollection)
    {
    // Do nothing.
    return;
    }

  propCollection->AddNew(
    representation->GetNewRenderObjects(),
    representation->GetLayerIndex());

  propCollection->AddExpired(
    representation->GetExpiredRenderObjects(),
    representation->GetLayerIndex());

  representation->ResetTemporaryRenderObjects();
}

//-----------------------------------------------------------------------------
void vtkVgLeafNodeBase::PrepareForRemoval(
  vtkVgPropCollection* propCollection, vtkVgRepresentationBase* representation)
{
  if (!representation || !propCollection)
    {
    // Do nothing.
    return;
    }

  representation->ResetTemporaryRenderObjects();

  propCollection->AddExpired(representation->GetActiveRenderObjects(),
                             representation->GetLayerIndex());

  // Since we removed the active render objects, we would need to copy them to the
  // new render objects to have them shown up if this node is added to SG again.
  vtkPropCollection* activeRO = representation->GetActiveRenderObjects();
  vtkPropCollection* newRO    = representation->GetNewRenderObjects();

  activeRO->InitTraversal();
  int numberOfItems = activeRO->GetNumberOfItems();
  for (int i = 0; i < numberOfItems; ++i)
    {
    newRO->AddItem(activeRO->GetNextProp());
    }
}
