/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __visgui_track_type_h
#define __visgui_track_type_h

#include <track_oracle/track_base.h>
#include <track_oracle/track_field.h>
#include <vgl/vgl_box_2d.h>
#include <vgl/vgl_point_2d.h>

//-----------------------------------------------------------------------------
struct visgui_base_track_type :
  public vidtk::track_base<visgui_base_track_type>
{
  vidtk::track_field<unsigned int>& external_id;
  vidtk::track_field<vgl_point_2d<double> >& obj_location;
  vidtk::track_field<vgl_box_2d<double> >& bounding_box;

  visgui_base_track_type() :
    external_id(Track.add_field<unsigned int>("external_id")),
    obj_location(Frame.add_field<vgl_point_2d<double> >("obj_location")),
    bounding_box(Frame.add_field<vgl_box_2d<double> >("bounding_box"))
    {
    }
};

//-----------------------------------------------------------------------------
struct visgui_fn_track_type :
  public vidtk::track_base<visgui_fn_track_type, visgui_base_track_type>
{
  vidtk::track_field<unsigned>& frame_number;

  visgui_fn_track_type() :
    frame_number(Frame.add_field<unsigned>("frame_number"))
    {
    }
};

//-----------------------------------------------------------------------------
struct visgui_ts_track_type :
  public vidtk::track_base<visgui_ts_track_type, visgui_base_track_type>
{
  vidtk::track_field<unsigned long long>& timestamp_usecs;

  visgui_ts_track_type() :
    timestamp_usecs(Frame.add_field<unsigned long long>("timestamp_usecs"))
    {
    }
};

//-----------------------------------------------------------------------------
struct visgui_track_type :
  public vidtk::track_base<visgui_track_type, visgui_base_track_type>
{
  vidtk::track_field<unsigned long long>& timestamp_usecs;
  vidtk::track_field<unsigned>& frame_number;

  visgui_track_type() :
    timestamp_usecs(Frame.add_field<unsigned long long>("timestamp_usecs")),
    frame_number(Frame.add_field<unsigned>("frame_number"))
    {
    }
};

#endif
