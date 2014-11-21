/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgComputeBounds.h"

// VG includes.
#include "vtkVgGroupNode.h"

// VTK includes.
#include <vtkMath.h>
#include <vtkProp.h>
#include <vtkPropCollection.h>

//-----------------------------------------------------------------------------
void vtkVgComputeBounds::ComputeVisiblePropBounds(vtkPropCollection* props,
                                                  double allBounds[6])
{
  vtkProp*    prop;
  double*     bounds;
  int        nothingVisible = 1;

  allBounds[0] = allBounds[2] = allBounds[4] = VTK_DOUBLE_MAX;
  allBounds[1] = allBounds[3] = allBounds[5] = -VTK_DOUBLE_MAX;

  // loop through all props
  vtkCollectionSimpleIterator pit;
  for (props->InitTraversal(pit);
       (prop = props->GetNextProp(pit));)
    {
    // For now consider bounds for invisible props as well.
    // If prop bounds should be ignored, or has no geometry, we can skip the rest
    if (prop->GetUseBounds())
      {
      bounds = prop->GetBounds();

      // make sure we haven't got bogus bounds
      if (bounds != NULL && vtkMath::AreBoundsInitialized(bounds))
        {
        nothingVisible = 0;

        if (bounds[0] < allBounds[0])
          {
          allBounds[0] = bounds[0];
          }
        if (bounds[1] > allBounds[1])
          {
          allBounds[1] = bounds[1];
          }
        if (bounds[2] < allBounds[2])
          {
          allBounds[2] = bounds[2];
          }
        if (bounds[3] > allBounds[3])
          {
          allBounds[3] = bounds[3];
          }
        if (bounds[4] < allBounds[4])
          {
          allBounds[4] = bounds[4];
          }
        if (bounds[5] > allBounds[5])
          {
          allBounds[5] = bounds[5];
          }
        }//not bogus
      }
    }

  if (nothingVisible)
    {
    vtkMath::UninitializeBounds(allBounds);
    return;
    }
}

//-----------------------------------------------------------------------------
void vtkVgComputeBounds::ComputeVisiblePropBounds(vtkVgGroupNode* groupNode)
{
  bool useCalculatedBounds = false;
  double groupBounds[6];
  groupBounds[0] = groupBounds[2] = groupBounds[4] = VTK_DOUBLE_MAX;
  groupBounds[1] = groupBounds[3] = groupBounds[5] = -VTK_DOUBLE_MAX;

  int numberOfChildren = groupNode->GetNumberOfChildren();

  for (int i = 0; i < numberOfChildren; ++i)
    {
    vtkVgNodeBase* node = groupNode->GetChild(i);
    double* bounds = const_cast<double*>(node->GetBounds());

    if (bounds != NULL && !vtkMath::AreBoundsInitialized(bounds))
      {
      if (node->GetBoundsDirty())
        {
        node->ComputeBounds();
        }
      }

    if (bounds != NULL && vtkMath::AreBoundsInitialized(bounds))
      {
      useCalculatedBounds = true;

      if (bounds[0] < groupBounds[0])
        {
        groupBounds[0] = bounds[0];
        }
      if (bounds[1] > groupBounds[1])
        {
        groupBounds[1] = bounds[1];
        }
      if (bounds[2] < groupBounds[2])
        {
        groupBounds[2] = bounds[2];
        }
      if (bounds[3] > groupBounds[3])
        {
        groupBounds[3] = bounds[3];
        }
      if (bounds[4] < groupBounds[4])
        {
        groupBounds[4] = bounds[4];
        }
      if (bounds[5] > groupBounds[5])
        {
        groupBounds[5] = bounds[5];
        }
      }
    }

  if (!useCalculatedBounds)
    {
    vtkMath::UninitializeBounds(groupBounds);
    }

  groupNode->SetBounds(groupBounds);
}
