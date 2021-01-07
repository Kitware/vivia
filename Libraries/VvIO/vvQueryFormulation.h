// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvQueryFormulation_h
#define __vvQueryFormulation_h

#include <string>

//-----------------------------------------------------------------------------
struct vvProcessingRequest
{
  vvProcessingRequest() : StartTime(-1), EndTime(-1) {}

  std::string QueryId;
  std::string VideoUri;

  long long StartTime;
  long long EndTime;
};

//-----------------------------------------------------------------------------
namespace vvQueryFormulation
{
  enum Type
    {
    FromImage,
    FromVideo,
    FromDatabase,
    UserDefined
    };
}
typedef vvQueryFormulation::Type vvQueryFormulationType;

#endif
