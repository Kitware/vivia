// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgPropCollection.h"

// VTK includes.
#include <vtkObjectFactory.h>
#include <vtkPropCollection.h>

// STL includes.
#include <algorithm>
#include <vector>

vtkStandardNewMacro(vtkVgPropCollection);

//-----------------------------------------------------------------------------
vtkVgPropCollection::vtkVgPropCollection() : vtkObject(),
  Dirty(true),
  DirtyCache(true),
  AllocationSize(200)
{
}

//-----------------------------------------------------------------------------
vtkVgPropCollection::~vtkVgPropCollection()
{
}

//-----------------------------------------------------------------------------
void vtkVgPropCollection::PrintSelf(ostream& vtkNotUsed(os),
                                    vtkIndent vtkNotUsed(indent))
{
}

//-----------------------------------------------------------------------------
void vtkVgPropCollection::AddNew(vtkPropCollection* collection, int layerIndex)
{
  if (!collection)
    {
    return;
    }

  int numberOfItems = collection->GetNumberOfItems();

  if (numberOfItems)
    {

    collection->InitTraversal();

    if (this->ActiveProps[layerIndex].empty())
      {
      this->ActiveProps[layerIndex].reserve(this->AllocationSize);
      }

    for (int i = 0; i < numberOfItems; ++i)
      {
      this->ActiveProps[layerIndex].push_back(collection->GetNextProp());
      }

    this->DirtyCacheOn();
    this->DirtyOn();
    }
}

//-----------------------------------------------------------------------------
void vtkVgPropCollection::AddExpired(vtkPropCollection* collection, int layerIndex)
{
  if (!collection)
    {
    return;
    }

  int numberOfItems = collection->GetNumberOfItems();

  if (numberOfItems)
    {
    collection->InitTraversal();

    for (int i = 0; i < numberOfItems; ++i)
      {
      vtkProp* nextProp = collection->GetNextProp();
      this->ExpiredProps[layerIndex].push_back(nextProp);

      // Remove from the active props here.
      // \NOTE: We have to check the performance. May be it is better to use
      // STL list but then traversal will be expensive.
      vtkVgPropCollection::Props props = this->ActiveProps[layerIndex];
      props.erase(std::remove(props.begin(), props.end(), nextProp), props.end());
      this->ActiveProps[layerIndex] = props;
      }

    this->DirtyOn();
    }
}

//-----------------------------------------------------------------------------
void vtkVgPropCollection::Reset()
{
  this->ExpiredProps.clear();

  this->DirtyCacheOff();
  this->DirtyOff();
}

//-----------------------------------------------------------------------------
void vtkVgPropCollection::ResetComplete()
{
  this->ActiveProps.clear();
  this->ExpiredProps.clear();

  this->DirtyCacheOff();
  this->DirtyOff();
}

//-----------------------------------------------------------------------------
const vtkVgPropCollection::SortedProps& vtkVgPropCollection::GetActiveProps() const
{
  return this->ActiveProps;
}

//-----------------------------------------------------------------------------
vtkVgPropCollection::SortedProps& vtkVgPropCollection::GetActiveProps()
{
  return this->ActiveProps;
}

//-----------------------------------------------------------------------------
const vtkVgPropCollection::SortedProps& vtkVgPropCollection::GetExpiredProps() const
{
  return this->ExpiredProps;
}

//-----------------------------------------------------------------------------
vtkVgPropCollection::SortedProps& vtkVgPropCollection::GetExpiredProps()
{
  return this->ExpiredProps;
}
