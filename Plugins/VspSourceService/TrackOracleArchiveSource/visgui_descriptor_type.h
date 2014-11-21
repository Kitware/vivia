/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
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
  vidtk::track_field<vcl_vector<unsigned int> >& source_track_ids;
  vidtk::track_field<vcl_vector<double> >& descriptor_classifier;

  vidtk::track_field<unsigned int>& frame_number;
  vidtk::track_field<unsigned long long>& timestamp_usecs;
  vidtk::track_field<vgl_box_2d<double> >& bounding_box;

  visgui_classifier_descriptor_type() :
    external_id(Track.add_field<unsigned int>("external_id")),
    source_track_ids(Track.add_field<vcl_vector<unsigned int> >("source_track_ids")),
    descriptor_classifier(Track.add_field<vcl_vector<double> >("descriptor_classifier")),
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
  vidtk::track_field<vcl_vector<unsigned int> >& source_track_ids;
  vidtk::track_field<vcl_vector<double> >& descriptor_pvo_raw_scores;

  visgui_pvo_descriptor_type() :
    source_track_ids(Track.add_field<vcl_vector<unsigned int> >("source_track_ids")),
    descriptor_pvo_raw_scores(Track.add_field<vcl_vector<double> >("descriptor_pvo_raw_scores"))
    {
    }
};

#endif
