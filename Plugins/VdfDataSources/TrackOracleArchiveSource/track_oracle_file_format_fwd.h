// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __track_oracle_file_format_fwd_h
#define __track_oracle_file_format_fwd_h

#ifdef KWIVER_TRACK_ORACLE
namespace kwiver
{
  namespace track_oracle
  {
    class file_format_base;
  }
}
#else
namespace vidtk
{
  class file_format_base;
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

#endif
