// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
