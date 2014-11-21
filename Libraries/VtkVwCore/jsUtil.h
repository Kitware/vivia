/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
