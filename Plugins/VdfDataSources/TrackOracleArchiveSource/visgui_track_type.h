/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
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
#include <vgl/vgl_point_3d.h>

#include <boost/uuid/uuid.hpp>

//-----------------------------------------------------------------------------
struct visgui_minimal_track_type :
  public track_oracle::track_base<visgui_minimal_track_type>
{
  track_oracle::track_field<unsigned int>& external_id;

  track_oracle::track_field<unsigned long long>& timestamp_usecs;
  track_oracle::track_field<vgl_box_2d<double> >& bounding_box;
  track_oracle::track_field<vgl_point_2d<double> >& obj_location;

  visgui_minimal_track_type() :
    TRACK_ORACLE_INIT_FIELD(Track, external_id),

    TRACK_ORACLE_INIT_FIELD(Frame, timestamp_usecs),
    TRACK_ORACLE_INIT_FIELD(Frame, bounding_box),
    TRACK_ORACLE_INIT_FIELD(Frame, obj_location)
    {
    }
};

//-----------------------------------------------------------------------------
struct visgui_track_type :
  public track_oracle::track_base<visgui_track_type, visgui_minimal_track_type>
{
#ifndef KWIVER_TRACK_ORACLE
  // TODO: Reenable this when UUID's are supported by KWIVER's track_oracle
  track_oracle::track_field<boost::uuids::uuid>& unique_id;
#endif

  track_oracle::track_field<unsigned>& frame_number;
  track_oracle::track_field<vgl_point_3d<double> >& world_location;

  visgui_track_type() :
#ifndef KWIVER_TRACK_ORACLE
    TRACK_ORACLE_INIT_FIELD(Track, unique_id),
#endif

    TRACK_ORACLE_INIT_FIELD(Frame, frame_number),
    TRACK_ORACLE_INIT_FIELD(Frame, world_location)
    {
    }
};

#endif
