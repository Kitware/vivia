/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
