// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvHeader_h
#define __vvHeader_h

#include <vgExport.h>

struct VV_IO_EXPORT vvHeader
{
  static const char* TracksTag;
  static const char* DescriptorsTag;
  static const char* QueryPlanTag;
  static const char* QueryResultsTag;
  static const char* EventSetInfoTag;

  vvHeader() : type(Unknown), version(static_cast<unsigned int>(-1)) {}
  bool isValid();

  enum FileType
    {
    Unknown,
    Tracks,
    Descriptors,
    QueryPlan,
    QueryResults,
    EventSetInfo
    };

  FileType type;
  unsigned int version;
};

#endif
