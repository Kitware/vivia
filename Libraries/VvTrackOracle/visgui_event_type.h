/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __visgui_event_type_h
#define __visgui_event_type_h

#include "track_oracle_utils.h"

#include <track_oracle/core/track_base.h>
#include <track_oracle/core/track_field.h>

#include <track_oracle/data_terms/data_terms.h>

//-----------------------------------------------------------------------------
struct visgui_minimal_event_type :
  public track_oracle::track_base<visgui_minimal_event_type>
{
  TRACK_ORACLE_FIELD(events, event_id);
  TRACK_ORACLE_FIELD(events, event_labels);
  TRACK_ORACLE_FIELD(events, actor_intervals);

  visgui_minimal_event_type()
  {
    Track.add_field(event_id);
    Track.add_field(event_labels);
    Track.add_field(actor_intervals);
  }
};

//-----------------------------------------------------------------------------
struct visgui_event_type :
  public track_oracle::track_base<visgui_event_type, visgui_minimal_event_type>
{
  TRACK_ORACLE_FIELD(events, event_start);
  TRACK_ORACLE_FIELD(events, event_stop);

  visgui_event_type()
  {
    Track.add_field(event_start);
    Track.add_field(event_stop);
  }
};

#endif
