// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
