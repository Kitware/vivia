/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __visgui_descriptor_type_h
#define __visgui_descriptor_type_h

#include "track_oracle_utils.h"

#ifdef KWIVER_TRACK_ORACLE
#include <track_oracle/core/track_base.h>
#include <track_oracle/core/track_field.h>
#else
#include <track_oracle/track_base.h>
#include <track_oracle/track_field.h>
#endif

#include <vgl/vgl_box_2d.h>

//-----------------------------------------------------------------------------
struct visgui_classifier_descriptor_type :
  public track_oracle::track_base<visgui_classifier_descriptor_type>
{
  track_oracle::track_field<unsigned int>& external_id;
  track_oracle::track_field<std::vector<unsigned int>>& source_track_ids;
  track_oracle::track_field<std::vector<double>>& descriptor_classifier;

  track_oracle::track_field<unsigned int>& frame_number;
  track_oracle::track_field<unsigned long long>& timestamp_usecs;
  track_oracle::track_field<vgl_box_2d<double>>& bounding_box;

  visgui_classifier_descriptor_type() :
    TRACK_ORACLE_INIT_FIELD(Track, external_id),
    TRACK_ORACLE_INIT_FIELD(Track, source_track_ids),
    TRACK_ORACLE_INIT_FIELD(Track, descriptor_classifier),

    TRACK_ORACLE_INIT_FIELD(Frame, frame_number),
    TRACK_ORACLE_INIT_FIELD(Frame, timestamp_usecs),
    TRACK_ORACLE_INIT_FIELD(Frame, bounding_box)
    {
    }
};

//-----------------------------------------------------------------------------
struct visgui_pvo_descriptor_type :
  public track_oracle::track_base<visgui_pvo_descriptor_type>
{
  track_oracle::track_field<std::vector<unsigned int>>& source_track_ids;
  track_oracle::track_field<std::vector<double>>& descriptor_pvo_raw_scores;

  visgui_pvo_descriptor_type() :
    TRACK_ORACLE_INIT_FIELD(Track, source_track_ids),
    TRACK_ORACLE_INIT_FIELD(Track, descriptor_pvo_raw_scores)
    {
    }
};

//-----------------------------------------------------------------------------
struct visgui_generic_event_type :
  public track_oracle::track_base<visgui_generic_event_type,
                                  visgui_classifier_descriptor_type>
{
  track_oracle::track_field<double>& start_time_secs;
  track_oracle::track_field<double>& end_time_secs;
  track_oracle::track_field<std::string>& basic_annotation;
  track_oracle::track_field<std::string>& augmented_annotation;

  visgui_generic_event_type() :
    TRACK_ORACLE_INIT_FIELD(Track, start_time_secs),
    TRACK_ORACLE_INIT_FIELD(Track, end_time_secs),
    TRACK_ORACLE_INIT_FIELD(Track, basic_annotation),
    TRACK_ORACLE_INIT_FIELD(Track, augmented_annotation)
    {
    }
};

#endif
