/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
