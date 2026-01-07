// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
