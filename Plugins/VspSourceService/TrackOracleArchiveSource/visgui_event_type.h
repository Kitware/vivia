/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __visgui_descriptor_type_h
#define __visgui_descriptor_type_h

#include <track_oracle/track_base.h>
#include <track_oracle/track_field.h>
#include <vgl/vgl_box_2d.h>

//-----------------------------------------------------------------------------
struct visgui_classifier_descriptor_type :
  public vidtk::track_base<visgui_classifier_descriptor_type>
{
  vidtk::track_field<unsigned int>& external_id;
  vidtk::track_field<std::vector<unsigned int> >& source_track_ids;
  vidtk::track_field<std::vector<double> >& descriptor_classifier;

  vidtk::track_field<unsigned int>& frame_number;
  vidtk::track_field<unsigned long long>& timestamp_usecs;
  vidtk::track_field<vgl_box_2d<double> >& bounding_box;

  visgui_classifier_descriptor_type() :
    external_id(Track.add_field<unsigned int>("external_id")),
    source_track_ids(Track.add_field<std::vector<unsigned int> >("source_track_ids")),
    descriptor_classifier(Track.add_field<std::vector<double> >("descriptor_classifier")),

    frame_number(Frame.add_field<unsigned int>("frame_number")),
    timestamp_usecs(Frame.add_field< unsigned long long>("timestamp_usecs")),
    bounding_box(Frame.add_field< vgl_box_2d<double> >("bounding_box"))
    {
    }
};

//-----------------------------------------------------------------------------
struct visgui_pvo_descriptor_type :
  public vidtk::track_base<visgui_pvo_descriptor_type>
{
  vidtk::track_field<std::vector<unsigned int> >& source_track_ids;
  vidtk::track_field<std::vector<double> >& descriptor_pvo_raw_scores;

  visgui_pvo_descriptor_type() :
    source_track_ids(Track.add_field<std::vector<unsigned int> >("source_track_ids")),
    descriptor_pvo_raw_scores(Track.add_field<std::vector<double> >("descriptor_pvo_raw_scores"))
    {
    }
};

//-----------------------------------------------------------------------------
struct visgui_generic_event_type :
  public vidtk::track_base<visgui_generic_event_type,
                           visgui_classifier_descriptor_type>
{
  vidtk::track_field<double>& start_time_secs;
  vidtk::track_field<double>& end_time_secs;
  vidtk::track_field<std::string>& basic_annotation;
  vidtk::track_field<std::string>& augmented_annotation;

  visgui_generic_event_type() :
    start_time_secs(Track.add_field<double>("start_time_secs")),
    end_time_secs(Track.add_field<double>("end_time_secs")),
    basic_annotation(Track.add_field<std::string>("basic_annotation")),
    augmented_annotation(Track.add_field<std::string>("augmented_annotation"))
    {
    }
};

#endif
