/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgRenderer.h"

#include "vtkVgAreaPicker.h"

// VTK inlcludes.
#include "vtkRenderer.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkCullerCollection.h"
#include "vtkCuller.h"
#include "vtkFrustumCoverageCuller.h"
#include "vtkGraphicsFactory.h"
#include "vtkHardwareSelector.h"
#include "vtkLightCollection.h"
#include "vtkLight.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkOutputWindow.h"
#include "vtkPainterPolyDataMapper.h"
#include "vtkPicker.h"
#include "vtkProp3DCollection.h"
#include "vtkPropCollection.h"
#include "vtkRendererDelegate.h"
#include "vtkRenderWindow.h"
#include "vtkTimerLog.h"
#include "vtkVolume.h"
#include "vtkRenderPass.h"
#include "vtkRenderState.h"
#include "vtkTexture.h"

vtkStandardNewMacro(vtkVgRenderer);

void vtkVgRenderer::PickRender(vtkPropCollection* props)
{
  vtkProp*  aProp;
  vtkAssemblyPath* path;

  this->InvokeEvent(vtkCommand::StartEvent, NULL);
  if (props->GetNumberOfItems() <= 0)
    {
    return;
    }

  // Create a place to store all props that remain after culling
  vtkPropCollection* pickFrom = vtkPropCollection::New();

  // Extract all the prop3D's out of the props collection.
  // This collection will be further culled by using a bounding box
  // pick later (vtkPicker). Things that are not vtkProp3D will get
  // put into the Paths list directly.
  vtkCollectionSimpleIterator pit;
  for (props->InitTraversal(pit); (aProp = props->GetNextProp(pit));)
    {
    if (aProp->GetPickable() && aProp->GetVisibility())
      {
      if (aProp->IsA("vtkProp3D"))
        {
        pickFrom->AddItem(aProp);
        }
      else //must be some other type of prop (e.g., vtkActor2D)
        {
        for (aProp->InitPathTraversal(); (path = aProp->GetNextPath());)
          {
          this->PathArray[this->PathArrayCount++] = path;
          }
        }
      }//pickable & visible
    }//for all props

  // For a first pass at the pick process, just use a vtkPicker to
  // intersect with bounding boxes of the objects.  This should greatly
  // reduce the number of polygons that the hardware has to pick from, and
  // speeds things up substantially.
  //

  vtkPicker* pCullPicker = NULL;
  vtkVgAreaPicker* aCullPicker = NULL;
  vtkProp3DCollection* cullPicked;
  if (this->GetPickWidth() == 1 && this->GetPickHeight() == 1)
    {
    // Create a picker to do the culling process
    pCullPicker = vtkPicker::New();

    // Add each of the Actors from the pickFrom list into the picker
    for (pickFrom->InitTraversal(pit); (aProp = pickFrom->GetNextProp(pit));)
      {
      pCullPicker->AddPickList(aProp);
      }

    // make sure this selects from the pickers list and not the renderers list
    pCullPicker->PickFromListOn();

    // do the pick
    pCullPicker->Pick(this->GetPickX(), this->GetPickY(), 0, this);

    cullPicked = pCullPicker->GetProp3Ds();
    }
  else
    {
    aCullPicker = vtkVgAreaPicker::New();

    // Add each of the Actors from the pickFrom list into the picker
    for (pickFrom->InitTraversal(pit); (aProp = pickFrom->GetNextProp(pit));)
      {
      aCullPicker->AddPickList(aProp);
      }

    // make sure this selects from the pickers list and not the renderers list
    aCullPicker->PickFromListOn();

    // do the pick
    aCullPicker->AreaPick(this->PickX1, this->PickY1,
                          this->PickX2, this->PickY2,
                          this);

    cullPicked = aCullPicker->GetProp3Ds();
    }

  // Put all the ones that were picked by the cull process
  // into the PathArray to be picked from
  vtkCollectionSimpleIterator p3dit;
  for (cullPicked->InitTraversal(p3dit);
       (aProp = cullPicked->GetNextProp3D(p3dit));)
    {
    if (aProp != NULL)
      {
      for (aProp->InitPathTraversal(); (path = aProp->GetNextPath());)
        {
        this->PathArray[this->PathArrayCount++] = path;
        }
      }
    }

  // Clean picking support objects up
  pickFrom->Delete();
  if (pCullPicker)
    {
    pCullPicker->Delete();
    }
  if (aCullPicker)
    {
    aCullPicker->Delete();
    }

  if (this->PathArrayCount == 0)
    {
    vtkDebugMacro(<< "There are no visible props!");
    return;
    }

  // do the render library specific pick render
  this->DevicePickRender();
}
