/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __track_oracle_utils_h
#define __track_oracle_utils_h

#ifdef KWIVER_TRACK_ORACLE
namespace kwiver
{
  namespace track_oracle
  {
    template <typename T> class track_field;
  }
}
#else
namespace vidtk
{
  template <typename T> class track_field;
}
#endif

namespace track_oracle
{
#ifdef KWIVER_TRACK_ORACLE
  using namespace kwiver::track_oracle;
#else
  using namespace vidtk;
#endif
}

#define TRACK_ORACLE_FIELD(ns, name) \
  ::track_oracle::track_field<::track_oracle::dt::ns::name> name

#endif
