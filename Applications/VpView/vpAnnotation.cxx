/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpAnnotation.h"

#include <vtkIdListCollection.h>
#include <vtkMatrix4x4.h>
#include <vtkPoints.h>

#include "vpViewCore.h"
#include "vtkVgAnnotationActor.h"
#include "vtkVgTrack.h"
#include "vtkVgEvent.h"
#include "vtkVgActivity.h"
#include "vtkVgTypeDefs.h"

#include <sstream>

//-----------------------------------------------------------------------------
vpAnnotation::vpAnnotation()
  : Visible(false), ObjectType(-1), ObjectId(-1)
{
  this->Actor = vtkVgAnnotationActor::New();
  this->Actor->AutoCenterXOn();

  this->RepresentationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
}

//-----------------------------------------------------------------------------
vpAnnotation::~vpAnnotation()
{
  this->Actor->Delete();
}

//-----------------------------------------------------------------------------
vtkProp* vpAnnotation::GetProp()
{
  return this->Actor;
}

//-----------------------------------------------------------------------------
void vpAnnotation::SetAnnotatedTrack(vtkVgTrack* track)
{
  if (this->ObjectType == vgObjectTypeDefinitions::Track &&
      this->Object.Track == track)
    {
    return;
    }

  this->ObjectType = vgObjectTypeDefinitions::Track;
  this->Object.Track = track;
  this->Update();
}

//-----------------------------------------------------------------------------
void vpAnnotation::SetAnnotatedEvent(vtkVgEvent* event)
{
  if (this->ObjectType == vgObjectTypeDefinitions::Event &&
      this->Object.Event == event)
    {
    return;
    }

  this->ObjectType = vgObjectTypeDefinitions::Event;
  this->Object.Event = event;
  this->ObjectId = event->GetId();
  this->Update();
}

//-----------------------------------------------------------------------------
void vpAnnotation::SetAnnotatedActivity(vtkVgActivity* activity)
{
  if (this->ObjectType == vgObjectTypeDefinitions::Activity &&
      this->Object.Activity == activity)
    {
    return;
    }

  this->ObjectType = vgObjectTypeDefinitions::Activity;
  this->Object.Activity = activity;
  this->ObjectId = activity->GetId();
  this->Update();
}

//-----------------------------------------------------------------------------
void vpAnnotation::SetAnnotatedSceneElement(vtkVgTrack* track, vtkIdType id)
{
  if (this->ObjectType == vgObjectTypeDefinitions::SceneElement &&
      this->Object.Track == track)
    {
    return;
    }

  this->ObjectType = vgObjectTypeDefinitions::SceneElement;
  this->Object.Track = track;
  this->ObjectId = id;
  this->Update();
}

//-----------------------------------------------------------------------------
void vpAnnotation::SetRepresentationMatrix(vtkMatrix4x4* matrix)
{
  if (!matrix || (this->RepresentationMatrix == matrix))
    {
    return;
    }

  this->RepresentationMatrix->DeepCopy(matrix);
}

//-----------------------------------------------------------------------------
void vpAnnotation::Update()
{
  if (!this->Visible)
    {
    this->Actor->SetVisibility(0);
    return;
    }

  bool visible;
  switch (this->ObjectType)
    {
    case vgObjectTypeDefinitions::Track:
      visible = this->UpdateTrackAnnotation(this->Object.Track, "T");
      break;

    case vgObjectTypeDefinitions::Event:
      visible = this->UpdateEventAnnotation(this->Object.Event);
      break;

    case vgObjectTypeDefinitions::Activity:
      visible = this->UpdateActivityAnnotation(this->Object.Activity);
      break;

    case vgObjectTypeDefinitions::SceneElement:
      visible = this->UpdateTrackAnnotation(this->Object.Track, "SE");
      break;

    default:
      return;
    }

  this->Actor->SetVisibility(visible ? 1 : 0);
}

//-----------------------------------------------------------------------------
bool vpAnnotation::UpdateTrackAnnotation(vtkVgTrack* track, const char* prefix)
{
  vtkIdList* pointIds = track->GetPointIds();
  if (pointIds->GetNumberOfIds() == 0)
    {
    return false;
    }

  std::ostringstream ostr;
  ostr << prefix << this->ObjectId;
  unsigned int currentFrame = this->ViewCoreInstance->getCurrentFrameIndex() +
                              this->ViewCoreInstance->getFrameNumberOffset();

  this->AddFrameDelta(ostr, currentFrame,
                      track->GetStartFrame().GetFrameNumber(),
                      track->GetEndFrame().GetFrameNumber());

  double point[4];
  track->GetClosestFramePt(this->ViewCoreInstance->getCoreTimeStamp(), point);
  point[2] = 0.0;
  point[3] = 1.0;

  this->RepresentationMatrix->MultiplyPoint(point, point);
  point[0] /= point[3];
  point[1] /= point[3];

  this->Actor->SetText(ostr.str().c_str());
  this->Actor->SetPosition(point);
  return true;
}

//-----------------------------------------------------------------------------
bool vpAnnotation::UpdateEventAnnotation(vtkVgEvent* event)
{
  std::ostringstream ostr;
  ostr << 'E' << event->GetId();
  unsigned int currentFrame = this->ViewCoreInstance->getCurrentFrameIndex() +
                              this->ViewCoreInstance->getFrameNumberOffset();

  // show time delta to the period when event becomes / was active
  this->AddFrameDelta(ostr, currentFrame,
                      event->GetStartFrame().GetFrameNumber(),
                      event->GetEndFrame().GetFrameNumber());

  double point[4];
  event->GetDisplayPosition(this->ViewCoreInstance->getCoreTimeStamp(), point);
  point[2] = 0.0;
  point[3] = 1.0;

  this->RepresentationMatrix->MultiplyPoint(point, point);
  point[0] /= point[3];
  point[1] /= point[3];

  this->Actor->SetText(ostr.str().c_str());
  this->Actor->SetPosition(point);
  return true;
}

//-----------------------------------------------------------------------------
bool vpAnnotation::UpdateActivityAnnotation(vtkVgActivity* activity)
{
  double point[4] = { 0.0, 0.0, 0.0, 1.0 };
  double bounds[4];

  int numEvents = activity->GetNumberOfEvents();
  for (int i = 0; i < numEvents; ++i)
    {
    vtkVgEvent* event = activity->GetEvent(i);
    event->GetFullBounds(bounds);

    point[0] += 0.5 * (bounds[0] + bounds[1]);
    point[1] += 0.5 * (bounds[2] + bounds[3]);
    }

  // put the annotation near the "middle" of the activity
  point[0] /= numEvents;
  point[1] /= numEvents;

  this->RepresentationMatrix->MultiplyPoint(point, point);
  point[0] /= point[3];
  point[1] /= point[3];

  std::ostringstream ostr;
  ostr << 'A' << activity->GetId();

  vtkVgTimeStamp startFrame, endFrame;
  activity->GetActivityFrameExtents(startFrame, endFrame);
  unsigned int currentFrame = this->ViewCoreInstance->getCurrentFrameIndex() +
                              this->ViewCoreInstance->getFrameNumberOffset();

  this->AddFrameDelta(ostr, currentFrame,
                      startFrame.GetFrameNumber(), endFrame.GetFrameNumber());

  this->Actor->SetText(ostr.str().c_str());
  this->Actor->SetPosition(point);
  return true;
}

//-----------------------------------------------------------------------------
void vpAnnotation::AddFrameDelta(std::ostream& os,
                                 unsigned int currFrame,
                                 unsigned int firstFrame,
                                 unsigned int lastFrame)
{
  if (currFrame < firstFrame)
    {
    os << "\n(-" << firstFrame - currFrame << ')';
    }
  else if (currFrame > lastFrame)
    {
    os << "\n(+" << currFrame - lastFrame << ')';
    }
}
