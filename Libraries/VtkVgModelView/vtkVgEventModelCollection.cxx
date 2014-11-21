
#include "vtkVgEventModelCollection.h"

// VisGUI includes.
#include "vtkVgEventModel.h"
#include "vtkVgEventInfo.h"

#include <vtkVgEvent.h>
#include <vtkVgEventBase.h>

// VTK includes.
#include <vtkObjectFactory.h>

// C++ includes.
#include <algorithm>
#include <list>

vtkStandardNewMacro(vtkVgEventModelCollection);

//-----------------------------------------------------------------------------
// Pimpl implementation.
class vtkVgEventModelCollection::Internal
{
public:

  Internal()
    {
    this->InternalEventModel = vtkVgEventModel::SmartPtr::New();

    // \note: Make sure internal event model is the first model
    // in the list.
    this->EventModels.push_back(this->InternalEventModel);
    }

  ~Internal()
    {
    this->EventModels.clear();
    }

  void AddEvents(vtkVgEventModel* otherEventModel);

  vtkVgEventModel::SmartPtr InternalEventModel;

  std::list<vtkVgEventModel::SmartPtr> EventModels;

  typedef std::list<vtkVgEventModel::SmartPtr>::iterator Itr;
  typedef std::list<vtkVgEventModel::SmartPtr>::const_iterator ConstItr;
};

//-----------------------------------------------------------------------------
void vtkVgEventModelCollection::Internal::AddEvents(vtkVgEventModel* otherEventModel)
{
  vtkVgEvent* event;
  this->InternalEventModel->InitEventTraversal();

  while ((event = this->InternalEventModel->GetNextEvent().GetEvent()))
    {
    otherEventModel->AddEvent(event);
    }
}

//-----------------------------------------------------------------------------
vtkVgEventModelCollection::vtkVgEventModelCollection() : vtkObject()
{
  this->Implementation = new Internal();
}

//-----------------------------------------------------------------------------
vtkVgEventModelCollection::~vtkVgEventModelCollection()
{
  delete this->Implementation; this->Implementation = 0;
}

//-----------------------------------------------------------------------------
void vtkVgEventModelCollection::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Number of event models in the collection: "
     << (this->Implementation->EventModels.size() - 1) << "\n";
}

//-----------------------------------------------------------------------------
void vtkVgEventModelCollection::Modified()
{
  this->Implementation->InternalEventModel->Modified();

  Internal::Itr itr = this->Implementation->EventModels.begin();

  // Skip internal event model.
  for (++itr; itr != this->Implementation->EventModels.end(); ++itr)
    {
    (*itr)->Modified();
    }

  vtkObject::Modified();
}

//-----------------------------------------------------------------------------
vtkVgEvent* vtkVgEventModelCollection::AddEvent(vtkVgEventBase* event)
{
  if (!event)
    {
    vtkErrorMacro("ERROR: Invalid (or NULL) event \n");
    return 0;
    }

  vtkVgEvent* newEvent =
    this->Implementation->InternalEventModel->AddEvent(event);

  if (!newEvent)
    {
    vtkErrorMacro("ERROR: Failed to add event to the list of event models\n");
    return 0;
    }

  Internal::Itr itr = this->Implementation->EventModels.begin();

  // Skip internal event model.
  for (++itr; itr != this->Implementation->EventModels.end(); ++itr)
    {
    (*itr)->AddEvent(newEvent);
    }

  this->Modified();

  return newEvent;
}

//-----------------------------------------------------------------------------
void vtkVgEventModelCollection::RemoveEvent(vtkIdType id)
{
  Internal::Itr itr = this->Implementation->EventModels.begin();

  for (; itr != this->Implementation->EventModels.end(); ++itr)
    {
    // \note: Currently we are not checking if the action has succeeded.
    (*itr)->RemoveEvent(id);
    }

  this->Modified();
}

//-----------------------------------------------------------------------------
vtkVgEvent* vtkVgEventModelCollection::GetEvent(vtkIdType id)
{
  return this->Implementation->InternalEventModel->GetEvent(id);
}

//-----------------------------------------------------------------------------
vtkVgEventModel* vtkVgEventModelCollection::GetInternalEventModel()
{
  return this->Implementation->InternalEventModel;
}

//-----------------------------------------------------------------------------
void vtkVgEventModelCollection::AddEventModel(vtkVgEventModel* eventModel)
{
  if (!eventModel)
    {
    vtkErrorMacro("ERROR: Invalid (or NULL) event model \n");
    return;
    }

  // Check if the event model already exists, if yes do nothing.
  std::list<vtkVgEventModel::SmartPtr>::iterator itr =
    std::find(this->Implementation->EventModels.begin(),
              this->Implementation->EventModels.end(),  eventModel);

  if (itr != this->Implementation->EventModels.end())
    {
    return;
    }

  // Set new model to use same SharedRegionPoints as other models
  // in the collection.
  eventModel->SetSharedRegionPoints(
    this->GetInternalEventModel()->GetSharedRegionPoints());

  this->Implementation->EventModels.push_back(eventModel);

  // Add current events to the newly added event model.
  this->Implementation->AddEvents(eventModel);

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventModelCollection::RemoveEventModel(vtkVgEventModel* eventModel)
{
  if (!eventModel)
    {
    vtkErrorMacro("ERROR: Invalid (or NULL) event model \n");
    return;
    }

  this->Implementation->EventModels.remove(eventModel);

  this->Modified();
}
