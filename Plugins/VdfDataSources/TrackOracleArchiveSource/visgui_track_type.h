/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __visgui_track_type_h
#define __visgui_track_type_h

#include <track_oracle/track_base.h>
#include <track_oracle/track_field.h>

#include <vgl/vgl_box_2d.h>
#include <vgl/vgl_point_2d.h>
#include <vgl/vgl_point_3d.h>

#include <boost/uuid/uuid.hpp>

//-----------------------------------------------------------------------------
struct visgui_minimal_track_type :
  public vidtk::track_base<visgui_minimal_track_type>
{
  vidtk::track_field<unsigned int>& external_id;

  vidtk::track_field<unsigned long long>& timestamp_usecs;
  vidtk::track_field<vgl_box_2d<double> >& bounding_box;
  vidtk::track_field<vgl_point_2d<double> >& obj_location;

  visgui_minimal_track_type() :
    external_id(Track.add_field<unsigned int>("external_id")),
    timestamp_usecs(Frame.add_field<unsigned long long>("timestamp_usecs")),
    bounding_box(Frame.add_field<vgl_box_2d<double> >("bounding_box")),
    obj_location(Frame.add_field<vgl_point_2d<double> >("obj_location"))
    {
    }
};

//-----------------------------------------------------------------------------
struct visgui_track_type :
  public vidtk::track_base<visgui_track_type, visgui_minimal_track_type>
{
  vidtk::track_field<boost::uuids::uuid>& unique_id;

  vidtk::track_field<unsigned>& frame_number;
  vidtk::track_field<vgl_point_3d<double> >& world_location;

  visgui_track_type() :
    unique_id(Track.add_field<boost::uuids::uuid>("unique_id")),
    frame_number(Frame.add_field<unsigned>("frame_number")),
    world_location(Frame.add_field<vgl_point_3d<double> >("world_location"))
    {
    }
};

#endif
