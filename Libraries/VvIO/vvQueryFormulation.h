/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
