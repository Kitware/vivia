// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgCompositeEventRepresentation.h"

#include <vtkObjectFactory.h>

#include "vtkVgEventFilter.h"
#include "vtkVgEventModel.h"
#include "vtkVgPickData.h"
#include "vtkVgPicker.h"

#include <vector>

vtkStandardNewMacro(vtkVgCompositeEventRepresentation);

//-----------------------------------------------------------------------------
class vtkVgCompositeEventRepresentation::vtkInternal
{
public:
  std::vector<vtkSmartPointer<vtkVgEventRepresentationBase> > EventRepresentations;
  vtkVgEventModel* EventModel;
  vtkVgEventFilter* EventFilter;
};

//-----------------------------------------------------------------------------
vtkVgCompositeEventRepresentation::vtkVgCompositeEventRepresentation()
  : vtkVgEventRepresentationBase()
{
  this->Visible = 1;
  this->Internal = new vtkInternal;
  this->Internal->EventModel = 0;
  this->Internal->EventFilter = 0;

  this->NewPropCollection    = vtkPropCollectionRef::New();
  this->ActivePropCollection = vtkPropCollectionRef::New();
  this->ExpirePropCollection = vtkPropCollectionRef::New();
}

//-----------------------------------------------------------------------------
vtkVgCompositeEventRepresentation::~vtkVgCompositeEventRepresentation()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgCompositeEventRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgCompositeEventRepresentation::GetNewRenderObjects() const
{
  return const_cast<vtkVgCompositeEventRepresentation*>(this)->GetNewRenderObjects();
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgCompositeEventRepresentation::GetActiveRenderObjects() const
{
  return const_cast<vtkVgCompositeEventRepresentation*>(this)->GetActiveRenderObjects();
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgCompositeEventRepresentation::GetExpiredRenderObjects() const
{
  return const_cast<vtkVgCompositeEventRepresentation*>(this)->GetExpiredRenderObjects();
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgCompositeEventRepresentation::GetNewRenderObjects()
{
  this->NewPropCollection->RemoveAllItems();

  for (size_t i = 0, size = this->Internal->EventRepresentations.size();
       i < size; ++i)
    {
    vtkPropCollection* PC =
      this->Internal->EventRepresentations[i]->GetNewRenderObjects();

    PC->InitTraversal();
    while (vtkProp* prop = PC->GetNextProp())
      {
      this->NewPropCollection->AddItem(prop);
      }
    }

  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgCompositeEventRepresentation::GetActiveRenderObjects()
{
  this->ActivePropCollection->RemoveAllItems();

  for (size_t i = 0, size = this->Internal->EventRepresentations.size();
       i < size; ++i)
    {
    vtkPropCollection* PC =
      this->Internal->EventRepresentations[i]->GetActiveRenderObjects();

    PC->InitTraversal();
    while (vtkProp* prop = PC->GetNextProp())
      {
      this->ActivePropCollection->AddItem(prop);
      }
    }

  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgCompositeEventRepresentation::GetExpiredRenderObjects()
{
  this->ExpirePropCollection->RemoveAllItems();

  for (size_t i = 0, size = this->Internal->EventRepresentations.size();
       i < size; ++i)
    {
    vtkPropCollection* PC =
      this->Internal->EventRepresentations[i]->GetExpiredRenderObjects();

    PC->InitTraversal();
    while (vtkProp* prop = PC->GetNextProp())
      {
      this->ExpirePropCollection->AddItem(prop);
      }
    }

  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
void vtkVgCompositeEventRepresentation::ResetTemporaryRenderObjects()
{
  for (size_t i = 0, size = this->Internal->EventRepresentations.size();
       i < size; ++i)
    {
    this->Internal->EventRepresentations[i]->ResetTemporaryRenderObjects();
    }
}

//-----------------------------------------------------------------------------
void vtkVgCompositeEventRepresentation::SetVisible(int flag)
{
  if (flag == this->Visible)
    {
    return;
    }

  for (size_t i = 0, size = this->Internal->EventRepresentations.size();
       i < size; ++i)
    {
    this->Internal->EventRepresentations[i]->SetVisible(flag);
    }

  this->Visible = flag;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgCompositeEventRepresentation::SetEventModel(vtkVgEventModel* model)
{
  for (size_t i = 0, size = this->Internal->EventRepresentations.size();
       i < size; ++i)
    {
    this->Internal->EventRepresentations[i]->SetEventModel(model);
    }

  this->Internal->EventModel = model;
}

//-----------------------------------------------------------------------------
void vtkVgCompositeEventRepresentation::SetEventFilter(vtkVgEventFilter* filter)
{
  for (size_t i = 0, size = this->Internal->EventRepresentations.size();
       i < size; ++i)
    {
    this->Internal->EventRepresentations[i]->SetEventFilter(filter);
    }

  this->Internal->EventFilter = filter;
}

//-----------------------------------------------------------------------------
void vtkVgCompositeEventRepresentation::Update()
{
  for (size_t i = 0, size = this->Internal->EventRepresentations.size();
       i < size; ++i)
    {
    this->Internal->EventRepresentations[i]->Update();
    }
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgCompositeEventRepresentation::Pick(
  double renX, double renY, vtkRenderer* ren, vtkIdType& pickType)
{
  pickType = vtkVgPickData::EmptyPick;
  for (size_t i = 0, size = this->Internal->EventRepresentations.size();
       i < size; ++i)
    {
    vtkIdType result =
      this->Internal->EventRepresentations[i]->Pick(renX, renY, ren, pickType);
    if (result != -1)
      {
      return result;
      }
    }
  return -1;
}

//-----------------------------------------------------------------------------
void vtkVgCompositeEventRepresentation::
AddEventRepresentation(vtkVgEventRepresentationBase* rep)
{
  // Add representation
  this->Internal->EventRepresentations.push_back(rep);

  // Set representation's model and filter to match ours, if we have them
  if (this->Internal->EventModel)
    {
    rep->SetEventModel(this->Internal->EventModel);
    }
  if (this->Internal->EventFilter)
    {
    rep->SetEventFilter(this->Internal->EventFilter);
    }

  // Set representation visibility to match ours
  rep->SetVisible(this->Visible);

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgCompositeEventRepresentation::
RemoveEventRepresentation(vtkVgEventRepresentationBase* rep)
{
  for (size_t i = 0, size = this->Internal->EventRepresentations.size();
       i < size; ++i)
    {
    if (this->Internal->EventRepresentations[i] == rep)
      {
      this->Internal->EventRepresentations.erase(
        this->Internal->EventRepresentations.begin() + i);
      break;
      }
    }
  this->Modified();
}
