// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
