/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvHeader.h"

const char* vvHeader::TracksTag = "TRACKS";
const char* vvHeader::DescriptorsTag = "DESCRIPTORS";
const char* vvHeader::QueryPlanTag = "QUERY";
const char* vvHeader::QueryResultsTag = "RESULTS";
const char* vvHeader::EventSetInfoTag = "EVENT_META";

//-----------------------------------------------------------------------------
bool vvHeader::isValid()
{
  return this->type != Unknown
         && this->version != static_cast<unsigned int>(-1);
}
