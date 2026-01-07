// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgNitfMetaDataParser_h
#define __vtkVgNitfMetaDataParser_h

#include <vgExport.h>

// C++ includes
#include <string>
#include <vector>

class vtkVgNitfEngrda;
class vtkVgTimeStamp;

namespace vtkVgNitfMetaDataParser
{
  VTKVG_IO_EXPORT bool ParseDateTime(const std::vector<std::string>& mdata,
                                     const std::vector<std::string>& treMetaData,
                                     vtkVgTimeStamp& time);

  VTKVG_IO_EXPORT bool ParseEngrda(const char* data, int len,
                                   vtkVgNitfEngrda& engrda);

  inline bool ParseEngrda(const std::string& meta,
                          vtkVgNitfEngrda& engrda)
    {
    return vtkVgNitfMetaDataParser::ParseEngrda(
      meta.c_str(), static_cast<int>(meta.size()), engrda);
    }
};

#endif // __vtkVgNitfMetaDataParser_h
