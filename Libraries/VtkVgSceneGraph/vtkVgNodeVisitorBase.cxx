/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgNodeVisitorBase.h"

#include "vtkVgNodeBase.h"
#include "vtkVgPropCollection.h"

//-----------------------------------------------------------------------------
vtkVgNodeVisitorBase::vtkVgNodeVisitorBase() :
  NodeVisitorType(NODE_VISITOR),
  PropCollection(NULL)
{
  this->TimeStamp.SetTime(-1.0);
  this->ModifiedTimeStamp.Modified();
}

//-----------------------------------------------------------------------------
vtkVgNodeVisitorBase::~vtkVgNodeVisitorBase()
{
}

//-----------------------------------------------------------------------------
void vtkVgNodeVisitorBase::Traverse(vtkVgNodeBase& node)
{
  node.Traverse(*this);
}

//-----------------------------------------------------------------------------
void vtkVgNodeVisitorBase::SetTimeStamp(const vtkVgTimeStamp& timeStamp)
{
  if (!(this->TimeStamp == timeStamp))
    {
    this->TimeStamp = timeStamp;
    this->ModifiedTimeStamp.Modified();
    }
}

//-----------------------------------------------------------------------------
const vtkVgTimeStamp& vtkVgNodeVisitorBase::GetTimeStamp() const
{
  return this->TimeStamp;
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp& vtkVgNodeVisitorBase::GetTimeStamp()
{
  return this->TimeStamp;
}

//-----------------------------------------------------------------------------
void vtkVgNodeVisitorBase::SetPropCollection(vtkVgPropCollection* propCollection)
{
  if (propCollection && (this->PropCollection != propCollection))
    {
    this->PropCollection = propCollection;
    this->ModifiedTimeStamp.Modified();
    }
}

//-----------------------------------------------------------------------------
const vtkVgPropCollection* vtkVgNodeVisitorBase::GetPropCollection() const
{
  return this->PropCollection;
}

//-----------------------------------------------------------------------------
vtkVgPropCollection* vtkVgNodeVisitorBase::GetPropCollection()
{
  return this->PropCollection;
}

//-----------------------------------------------------------------------------
void vtkVgNodeVisitorBase::ShallowCopy(vtkVgNodeVisitorBase* other)
{
  if (!other)
    {
    return;
    }

  this->NodeVisitorType = other->NodeVisitorType;
  this->TimeStamp       = other->TimeStamp;

  this->ModifiedTimeStamp.Modified();

  this->PropCollection = other->GetPropCollection();
}
