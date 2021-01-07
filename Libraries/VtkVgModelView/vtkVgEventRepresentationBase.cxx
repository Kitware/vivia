// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgEventRepresentationBase.h"

#include "vtkVgContourOperatorManager.h"
#include "vtkVgEventFilter.h"
#include "vtkVgEventModel.h"
#include "vtkVgEventTypeRegistry.h"
#include "vtkVgPickData.h"
#include "vtkVgPicker.h"

#include <vtkCommand.h>

vtkCxxSetObjectMacro(vtkVgEventRepresentationBase,
                     EventFilter,
                     vtkVgEventFilter);

vtkCxxSetObjectMacro(vtkVgEventRepresentationBase,
                     ContourOperatorManager,
                     vtkVgContourOperatorManager);

vtkCxxSetObjectMacro(vtkVgEventRepresentationBase,
                     EventTypeRegistry,
                     vtkVgEventTypeRegistry);

//-----------------------------------------------------------------------------
vtkVgEventRepresentationBase::vtkVgEventRepresentationBase()
  : vtkVgRepresentationBase(),
    EventModel(0), EventFilter(0), EventTypeRegistry(0),
    ContourOperatorManager(0),
    UseNormalcyCues(false), NormalcyCuesSwapped(false)
{
  this->SetLayerIndex(2);
}

//-----------------------------------------------------------------------------
vtkVgEventRepresentationBase::~vtkVgEventRepresentationBase()
{
  this->SetEventModel(0);
  this->SetEventFilter(0);
  this->SetEventTypeRegistry(0);
  this->SetContourOperatorManager(0);
}

//-----------------------------------------------------------------------------
void vtkVgEventRepresentationBase::SetEventModel(vtkVgEventModel* eventModel)
{
  vtkSetObjectBodyMacro(EventModel, vtkVgEventModel, eventModel);

  if (this->EventModel)
    {
    this->EventModel->AddObserver(vtkVgEventModel::EventRemoved, this,
                                  &vtkVgEventRepresentationBase::EventRemoved);
    if (this->UseAutoUpdate)
      {
      this->EventModel->AddObserver(vtkCommand::UpdateDataEvent, this,
                                    &vtkVgEventRepresentationBase::Update);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventRepresentationBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgEventRepresentationBase::Pick(double vtkNotUsed(renX),
                                             double vtkNotUsed(renY),
                                             vtkRenderer* vtkNotUsed(ren),
                                             vtkIdType& pickType)
{
  pickType = vtkVgPickData::EmptyPick;
  return -1;
}

//-----------------------------------------------------------------------------
void vtkVgEventRepresentationBase::GetPickPosition(double vtkNotUsed(pos)[])
{
}

//-----------------------------------------------------------------------------
void vtkVgEventRepresentationBase::SwapNormalcyCues()
{
  this->NormalcyCuesSwapped = !this->NormalcyCuesSwapped;
  this->Modified();
}
