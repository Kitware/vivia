// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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

#include <track_oracle/data_terms/data_terms.h>

//-----------------------------------------------------------------------------
struct visgui_base_track_type :
  public track_oracle::track_base<visgui_base_track_type>
{
  TRACK_ORACLE_FIELD(tracking, external_id);

  TRACK_ORACLE_FIELD(tracking, obj_location);
  TRACK_ORACLE_FIELD(tracking, bounding_box);

  visgui_base_track_type()
    {
    Track.add_field(external_id);

    Frame.add_field(bounding_box);
    Frame.add_field(obj_location);
    }
};

//-----------------------------------------------------------------------------
struct visgui_fn_track_type :
  public track_oracle::track_base<visgui_fn_track_type, visgui_base_track_type>
{
  TRACK_ORACLE_FIELD(tracking, frame_number);

  visgui_fn_track_type()
    {
    Frame.add_field(frame_number);
    }
};

//-----------------------------------------------------------------------------
struct visgui_ts_track_type :
  public track_oracle::track_base<visgui_ts_track_type, visgui_base_track_type>
{
  TRACK_ORACLE_FIELD(tracking, timestamp_usecs);

  visgui_ts_track_type()
    {
    Frame.add_field(timestamp_usecs);
    }
};

//-----------------------------------------------------------------------------
struct visgui_track_type :
  public track_oracle::track_base<visgui_track_type, visgui_base_track_type>
{
  TRACK_ORACLE_FIELD(tracking, timestamp_usecs);
  TRACK_ORACLE_FIELD(tracking, frame_number);

  visgui_track_type()
    {
    Frame.add_field(timestamp_usecs);
    Frame.add_field(frame_number);
    }
};

#endif
