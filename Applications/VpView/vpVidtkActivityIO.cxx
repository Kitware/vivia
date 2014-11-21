/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVidtkActivityIO.h"

#include "vpActivityConfig.h"
#include "vpVidtkReader.h"

#include <vtkVgActivity.h>
#include <vtkVgActivityManager.h>
#include <vtkVgEventModel.h>

#include <vgActivityType.h>

#include <assert.h>

//-----------------------------------------------------------------------------
vpVidtkActivityIO::vpVidtkActivityIO(vpVidtkReader& reader,
                                     vtkVgActivityManager* activityManager,
                                     vpActivityConfig* activityConfig) :
  vpActivityIO(activityManager, activityConfig), Reader(reader)
{}

//-----------------------------------------------------------------------------
vpVidtkActivityIO::~vpVidtkActivityIO()
{}

//-----------------------------------------------------------------------------
bool vpVidtkActivityIO::ReadActivities()
{
  if (this->Reader.GetImageHeight() == 0)
    {
    return false;
    }

  if (!this->Reader.ReadActivities(this->Activities))
    {
    return false;
    }

  vcl_vector<vidtk::activity_sptr>::iterator activityIter = this->Activities.begin();
  for (; activityIter != this->Activities.end(); activityIter++)
    {
    vtkSmartPointer<vtkVgActivity> activity = vtkSmartPointer<vtkVgActivity>::New();

    if (this->SetupActivity(*activityIter, activity))
      {
      this->ActivityManager->AddActivity(activity);
      this->ActivityMap[activity] = (*activityIter);
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vpVidtkActivityIO::SetupActivity(
  vidtk::activity_sptr& vidtkActivity, vtkVgActivity* vgActivity)
{
  vgActivity->SetId(static_cast<int>(vidtkActivity->get_id()));

  int type = this->ActivityConfig->GetId(vidtkActivity->my_name());
  this->ActivityConfig->MarkTypeUsed(type);
  const vgActivityType& at = this->ActivityConfig->GetActivityType(type);

  vgActivity->SetType(type);
  vgActivity->SetIconIndex(at.GetIconIndex());
  vgActivity->SetName(at.GetName());
  vgActivity->SetBoundsMode(at.GetDisplayMode() == vgActivityType::DM_Bounds);

  // spatial bounds
  double imageYExtent = this->Reader.GetImageHeight() - 1.0;
  const vgl_box_2d<double>& vglBBox = vidtkActivity->get_spatial_bounds();
  vgActivity->SetSpatialBounds(vglBBox.min_x(), vglBBox.max_x(),
                               imageYExtent - vglBBox.max_y(),
                               imageYExtent - vglBBox.min_y());

  vtkVgEventModel* eventModel = this->ActivityManager->GetEventModel();
  assert(eventModel);

  const vcl_vector<vidtk::event_sptr>& events = vidtkActivity->get_supporting_events();
  vcl_vector<vidtk::event_sptr>::const_iterator eventIter = events.begin();
  for (; eventIter != events.end(); eventIter++)
    {
    vtkVgEvent* theEvent = eventModel->GetEvent((*eventIter)->get_id());
    if (!theEvent)
      {
      std::cerr << "ERROR: Unable to find event using event id "
                << (*eventIter)->get_id() << std::endl;
      std::cerr << "ERROR: Failed to setup activity " << vgActivity->GetId()
                << std::endl;
      return false;
      }
    vgActivity->AddEvent(theEvent);
    }

  vgActivity->SetSaliency(vidtkActivity->get_saliency());
  vgActivity->SetProbability(vidtkActivity->get_probability());

  return true;
}
