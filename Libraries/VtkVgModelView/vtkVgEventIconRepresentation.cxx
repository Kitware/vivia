/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgEventIconRepresentation.h"

#include <vtkIdListCollection.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkTimeStamp.h>

#include "vgEventType.h"

#include "vtkVgEvent.h"
#include "vtkVgEventFilter.h"
#include "vtkVgEventModel.h"
#include "vtkVgEventTypeRegistry.h"
#include "vtkVgIconManager.h"
#include "vtkVgTrack.h"
#include "vtkVgTrackModel.h"

vtkStandardNewMacro(vtkVgEventIconRepresentation);

vtkCxxSetObjectMacro(vtkVgEventIconRepresentation, IconManager, vtkVgIconManager);

//-----------------------------------------------------------------------------
vtkVgEventIconRepresentation::vtkVgEventIconRepresentation()
{
  this->IconManager = 0;

  // These aren't actually used right now...
  this->NewPropCollection    = vtkPropCollectionRef::New();
  this->ActivePropCollection = vtkPropCollectionRef::New();
  this->ExpirePropCollection = vtkPropCollectionRef::New();
}

//-----------------------------------------------------------------------------
vtkVgEventIconRepresentation::~vtkVgEventIconRepresentation()
{
  if (this->IconManager)
    {
    this->IconManager->UnRegister(this);
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventIconRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgEventIconRepresentation::GetNewRenderObjects() const
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgEventIconRepresentation::GetActiveRenderObjects() const
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgEventIconRepresentation::GetExpiredRenderObjects() const
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgEventIconRepresentation::GetNewRenderObjects()
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgEventIconRepresentation::GetActiveRenderObjects()
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgEventIconRepresentation::GetExpiredRenderObjects()
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
void vtkVgEventIconRepresentation::ResetTemporaryRenderObjects()
{
  this->NewPropCollection->RemoveAllItems();
  this->ExpirePropCollection->RemoveAllItems();
}

//-----------------------------------------------------------------------------
void vtkVgEventIconRepresentation::SetVisible(int flag)
{
  if (this->IconManager->GetVisibility() != flag)
    {
    this->IconManager->SetVisibility(flag);
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
int vtkVgEventIconRepresentation::GetVisible() const
{
  return this->IconManager->GetVisibility();
}

//-----------------------------------------------------------------------------
void vtkVgEventIconRepresentation::UpdateEventIcons(vtkVgEvent* event)
{
  int id = this->EventFilter->GetBestClassifier(event);
  if (id == -1)
    {
    this->HideEventIcons(event);
    return;
    }

  vtkIdList* iconIds = event->GetIconIds();
  int numDisplayedIcons = 0;

  // show an icon at the head of each event track
  int end = this->EventModel->GetUseTrackGroups() ?
            event->GetNumberOfTrackGroups() : event->GetNumberOfTracks();
  for (int i = 0; i < end; ++i)
    {
    vtkVgTrackDisplayData tdd = this->EventModel->GetTrackDisplayData(event, i);
    if (tdd.NumIds == 0)
      {
      if (i < iconIds->GetNumberOfIds())
        {
        this->IconManager->SetIconVisibility(iconIds->GetId(i), 0);
        }
      continue;
      }

    ++numDisplayedIcons;

    double x[3];
    this->EventModel->GetTrackModel()->GetPoints()
    ->GetPoint(tdd.IdsStart[tdd.NumIds - 1], x);

    double scale = 1.0;
    if (this->UseNormalcyCues)
      {
      scale = 1.0 - 0.7 * event->GetActiveClassifierNormalcy();
      if (this->NormalcyCuesSwapped)
        {
        scale = 1.0 - scale;
        }
      }

    int pos[] = { static_cast<int>(x[0]), static_cast<int>(x[1]) };
    if (i >= iconIds->GetNumberOfIds())
      {
      int iconType =
        this->EventTypeRegistry->GetTypeById(id).GetTrackIconIndex(i);

      if (iconType >= 0)
        {
        iconIds->InsertNextId(
          this->IconManager->AddIcon(vgEventTypeDefinitions::Event,
                                     iconType, 1, pos, scale));
        }
      }
    else
      {
      // @TODO: Recreate the icon if the classifier has changed.
      vtkIdType id = iconIds->GetId(i);
      this->IconManager->SetIconVisibility(id, 1);
      this->IconManager->SetIconScale(id, scale);
      this->IconManager->UpdateIconPosition(id, pos);
      }
    }

  if (numDisplayedIcons == 0)
    {
    this->HideEventIcons(event);
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventIconRepresentation::HideEventIcons(vtkVgEvent* event)
{
  vtkIdList* iconIds = event->GetIconIds();
  for (vtkIdType i = 0; i < iconIds->GetNumberOfIds(); i++)
    {
    this->IconManager->SetIconVisibility(iconIds->GetId(i), 0);
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventIconRepresentation::Update()
{
  if (!this->GetVisible())
    {
    if (this->GetMTime() > this->UpdateTime)
      {
      this->IconManager->UpdateIconActors(
        this->EventModel->GetCurrentTimeStamp().GetFrameNumber());
      this->UpdateTime.Modified();
      }
    return;
    }

  this->EventModel->InitEventTraversal();
  vtkVgEventInfo info;
  while ((info = this->EventModel->GetNextEvent()).GetEvent())
    {
    if (info.GetDisplayEvent())
      {
      this->UpdateEventIcons(info.GetEvent());
      }
    else
      {
      this->HideEventIcons(info.GetEvent());
      }
    }

  this->IconManager->UpdateIconActors(
    this->EventModel->GetCurrentTimeStamp().GetFrameNumber());

  this->UpdateTime.Modified();
}
