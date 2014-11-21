/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgStringUtils_h
#define __vgStringUtils_h

#include <sstream>
#include <string>
#include <vector>

class vgStringUtils
{
public:
  static std::vector<std::string> &Split(
    const std::string &s, char delim, std::vector<std::string> &elems)
    {
      std::stringstream ss(s);
      std::string item;
      while(std::getline(ss, item, delim)) {
          elems.push_back(item);
      }
    return elems;
    }


  static std::vector<std::string> Split(const std::string &s, char delim)
    {
    std::vector<std::string> elems;
    return vgStringUtils::Split(s, delim, elems);
    }
};

#endif // __vgStringUtils_h
