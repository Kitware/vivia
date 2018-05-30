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

#include <track_oracle/data_terms/data_terms.h>

//-----------------------------------------------------------------------------
struct visgui_minimal_track_type :
  public track_oracle::track_base<visgui_minimal_track_type>
{
  TRACK_ORACLE_FIELD(tracking, external_id);

  TRACK_ORACLE_FIELD(tracking, timestamp_usecs);
  TRACK_ORACLE_FIELD(tracking, bounding_box);
  TRACK_ORACLE_FIELD(tracking, obj_location);

  visgui_minimal_track_type()
    {
    Track.add_field(external_id);

    Frame.add_field(timestamp_usecs);
    Frame.add_field(bounding_box);
    Frame.add_field(obj_location);
    }
};

//-----------------------------------------------------------------------------
struct visgui_track_type :
  public track_oracle::track_base<visgui_track_type, visgui_minimal_track_type>
{
  TRACK_ORACLE_FIELD(tracking, track_uuid);

  TRACK_ORACLE_FIELD(tracking, frame_number);
  TRACK_ORACLE_FIELD(tracking, world_location);

  visgui_track_type()
    {
    Track.add_field(track_uuid);

    Frame.add_field(frame_number);
    Frame.add_field(world_location);
    }
};

#endif
