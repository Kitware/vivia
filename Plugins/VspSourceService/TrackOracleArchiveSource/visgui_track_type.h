/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __visgui_track_type_h
#define __visgui_track_type_h

#include "track_oracle_utils.h"

#ifdef KWIVER_TRACK_ORACLE
#include <track_oracle/core/track_base.h>
#include <track_oracle/core/track_field.h>
#else
#include <track_oracle/track_base.h>
#include <track_oracle/track_field.h>
#endif

#include <vgl/vgl_box_2d.h>
#include <vgl/vgl_point_2d.h>

//-----------------------------------------------------------------------------
struct visgui_base_track_type :
  public track_oracle::track_base<visgui_base_track_type>
{
  track_oracle::track_field<unsigned int>& external_id;
  track_oracle::track_field<vgl_point_2d<double>>& obj_location;
  track_oracle::track_field<vgl_box_2d<double>>& bounding_box;

  visgui_base_track_type() :
    TRACK_ORACLE_INIT_FIELD(Track, external_id),
    TRACK_ORACLE_INIT_FIELD(Frame, obj_location),
    TRACK_ORACLE_INIT_FIELD(Frame, bounding_box)
    {
    }
};

//-----------------------------------------------------------------------------
struct visgui_fn_track_type :
  public track_oracle::track_base<visgui_fn_track_type, visgui_base_track_type>
{
  track_oracle::track_field<unsigned>& frame_number;

  visgui_fn_track_type() :
    TRACK_ORACLE_INIT_FIELD(Frame, frame_number)
    {
    }
};

//-----------------------------------------------------------------------------
struct visgui_ts_track_type :
  public track_oracle::track_base<visgui_ts_track_type, visgui_base_track_type>
{
  track_oracle::track_field<unsigned long long>& timestamp_usecs;

  visgui_ts_track_type() :
    TRACK_ORACLE_INIT_FIELD(Frame, timestamp_usecs)
    {
    }
};

//-----------------------------------------------------------------------------
struct visgui_track_type :
  public track_oracle::track_base<visgui_track_type, visgui_base_track_type>
{
  track_oracle::track_field<unsigned long long>& timestamp_usecs;
  track_oracle::track_field<unsigned>& frame_number;

  visgui_track_type() :
    TRACK_ORACLE_INIT_FIELD(Frame, timestamp_usecs),
    TRACK_ORACLE_INIT_FIELD(Frame, frame_number)
    {
    }
};

#endif
