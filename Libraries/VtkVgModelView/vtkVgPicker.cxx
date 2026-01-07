// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgPicker.h"

#include "vtkVgCellPicker.h"
#include "vtkObjectFactory.h"
#include "vtkVgIconManager.h"
#include "vtkVgTrackRepresentationBase.h"
#include "vtkVgEventRepresentationBase.h"
#include "vtkVgActivityManager.h"

#include "vtkVgPickData.h"

#include "vtkImageActor.h"
#include "vtkRenderer.h"
#include "vtkCommand.h"

// C++ includes
#include <algorithm>

vtkStandardNewMacro(vtkVgPicker);
vtkCxxSetObjectMacro(vtkVgPicker, IconManager, vtkVgIconManager);
vtkCxxSetObjectMacro(vtkVgPicker, ActivityManager, vtkVgActivityManager);
vtkCxxSetObjectMacro(vtkVgPicker, ImageActor, vtkImageActor);
vtkCxxSetObjectMacro(vtkVgPicker, Actor, vtkActor);

// Construct object with initial tolerance of 1/40th of window. There are no
// pick methods and picking is performed from the renderer's actors.
//-----------------------------------------------------------------------------
vtkVgPicker::vtkVgPicker()
{
  this->Verbose = 1;

  this->PickedId = -1;

  this->IconManager = NULL;
  this->IconId = -1;

  this->ActivityManager = NULL;
  this->ActivityIndex = -1;

  this->ImageActor = NULL;

  this->Actor = NULL;

  this->Picker = vtkSmartPointer<vtkVgCellPicker>::New();
  this->Picker->PickFromListOn();
}

//-----------------------------------------------------------------------------
vtkVgPicker::~vtkVgPicker()
{
  if (this->IconManager)
    {
    this->IconManager->Delete();
    }
  if (this->ActivityManager)
    {
    this->ActivityManager->Delete();
    }
  if (this->ImageActor)
    {
    this->ImageActor->Delete();
    }
  if (this->Actor)
    {
    this->Actor->Delete();
    }

}

//-----------------------------------------------------------------------------
void vtkVgPicker::AddRepresentation(vtkVgRepresentationBase* representation)
{
  if (!representation)
    {
    vtkErrorMacro("ERROR: Invalid representation \n");
    return;
    }

  if (std::find(this->Representations.begin(), this->Representations.end(),  representation) !=
      this->Representations.end())
    {
    vtkErrorMacro("ERROR: Representation " << representation << " already exists\n");
    return;
    }

  this->Representations.push_back(representation);
}

//-----------------------------------------------------------------------------
// There is an order to the picking process. 1) icons 2) tracks 3) background image
// 4) event 5) activity.
int vtkVgPicker::
Pick(double selectionX, double selectionY, double selectionZ, vtkRenderer* renderer)
{
  // Invoke start pick method if defined
  this->InvokeEvent(vtkCommand::StartPickEvent, NULL);

  // Set up the pick
  this->Initialize();
  this->IconId = -1;
  this->Renderer = renderer;
  this->SelectionPoint[0] = selectionX;
  this->SelectionPoint[1] = selectionY;
  this->SelectionPoint[2] = 0;

  // Try picking against the icon manager
  if (this->IconManager)
    {
    this->IconId = this->IconManager->Pick(selectionX, selectionY, renderer);
    if (this->IconId > -1)
      {
      this->InvokeEvent(vtkCommand::PickEvent, NULL);

      int pos[2];
      this->IconManager->GetIconPosition(this->IconId, pos);

      if (this->Verbose)
        {
        cout << "Icon Id/Type: (" << this->IconId << ","
             << this->IconManager->GetIcon(this->IconId)->GetType() << ")\n";
        cout << "Icon Position: (" << pos[0] << "," << pos[1] << ")\n";
        }
      this->PickPosition[0] = static_cast<double>(pos[0]);
      this->PickPosition[1] = static_cast<double>(pos[1]);
      this->PickPosition[2] = 0.0;
      return vtkVgPickData::PickedIcon;
      }
    }

  std::vector<vtkSmartPointer<vtkVgRepresentationBase> >::const_iterator constItr
    = this->Representations.begin();
  while (constItr != this->Representations.end())
    {
    vtkIdType pickType = vtkVgPickData::EmptyPick;
    this->PickedId = (*constItr)->Pick(selectionX, selectionY, renderer, pickType);

    if (this->PickedId > -1)
      {
      double pos[3];
      (*constItr)->GetPickPosition(pos);

      if (this->Verbose)
        {
        cout << "PickedId Id: " << this->PickedId << "\n";
        cout << "PickedId Position: (" << pos[0] << "," << pos[1] << ")\n";
        }
      this->PickPosition[0] = pos[0];
      this->PickPosition[1] = pos[1];
      this->PickPosition[2] = 0.0;

      return pickType;
      }

    ++constItr;
    }

  // Try picking against the activity manager
  if (this->ActivityManager)
    {
    this->ActivityIndex =
      this->ActivityManager->Pick(selectionX, selectionY, renderer);

    if (this->ActivityIndex > -1)
      {
      this->InvokeEvent(vtkCommand::PickEvent, NULL);

      double pos[3];
      this->ActivityManager->GetPickPosition(pos);
      if (this->Verbose)
        {
        cout << "Activity Index: " << this->ActivityIndex << "\n";
        cout << "Activity Position: (" << pos[0] << "," << pos[1] << ")\n";
        }
      this->PickPosition[0] = pos[0];
      this->PickPosition[1] = pos[1];
      this->PickPosition[2] = 0.0;
      return vtkVgPickData::PickedActivity;
      }
    }

  // If we are at this point there is no icon picked, try picking background image.
  if (this->ImageActor)
    {
    this->Picker->InitializePickList();
    this->Picker->AddPickList(this->ImageActor);
    int pickStatus = this->Picker->Pick(selectionX, selectionY, selectionZ, renderer);
    if (pickStatus)
      {
      this->Picker->GetPickPosition(this->PickPosition);
      if (this->Verbose)
        {
        cout << "Picked Image\n";
        cout << "Pick Position: (" << this->PickPosition[0] << ","
             << this->PickPosition[1] << ")\n";
        }
      return vtkVgPickData::PickedImage;
      }
    }

  if (this->Actor)
    {
    this->Picker->InitializePickList();
    this->Picker->AddPickList(this->Actor);
    int pickStatus = this->Picker->Pick(selectionX, selectionY, selectionZ, renderer);
    if (pickStatus)
      {
      this->Picker->GetPickPosition(this->PickPosition);
      if (this->Verbose)
        {
        cout << "Picked Actor\n";
        cout << "Pick Position: (" << this->PickPosition[0] << ","
             << this->PickPosition[1] << ")\n";
        }
      return vtkVgPickData::PickedActor;
      }
    }

  // Complete the picking process.
  this->InvokeEvent(vtkCommand::EndPickEvent, NULL);

  return vtkVgPickData::EmptyPick;
}

//-----------------------------------------------------------------------------
void vtkVgPicker::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
