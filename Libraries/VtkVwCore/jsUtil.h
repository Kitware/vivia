// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __jsUtil_h
#define __jsUtil_h

#include <vvJson.h>

namespace jsUtil
{
  namespace qt
  {
    // Need both of these so template below can determine correct overload
    using qtJson::encode;
    using vvJson::encode;
  }

  template <typename T> std::string encode(const T& object)
    {
    const qtJson::JsonData data = qt::encode(object);
    return std::string(data.constData(), data.length());
    }
}

#endif
