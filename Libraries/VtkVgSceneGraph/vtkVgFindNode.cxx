/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgFindNode.h"

#include "vtkVgGroupNode.h"
#include "vtkVgNodeBase.h"

bool vtkVgFindNode::ContainsNode(vtkVgGroupNode* root, vtkVgNodeBase* searchNode)
{
  bool retVal = false;

  if (!root || !searchNode)
    {
    return retVal;
    }

  int numberOfChildren = root->GetNumberOfChildren();
  for (int i = 0; i < numberOfChildren; ++i)
    {
    vtkVgNodeBase* node = root->GetChild(i);

    if (node == searchNode)
      {
      retVal = true;
      return retVal;
      }

    vtkVgGroupNode* groupNode = dynamic_cast<vtkVgGroupNode*>(node);
    if (groupNode)
      {
      retVal = vtkVgFindNode::ContainsNode(groupNode, searchNode);

      if (retVal)
        {
        return retVal;
        }
      }
    }

  return false;
}
