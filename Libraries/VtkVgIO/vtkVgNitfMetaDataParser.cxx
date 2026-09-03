// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgNitfMetaDataParser.h"

#include "vtkVgNitfEngrda.h"

#include <vgCalendarUtils.h>
#include <vgStringUtils.h>

#include <vtkVgTimeStamp.h>

// C/C++ includes
#include <cstdint>

//----------------------------------------------------------------------------
namespace
{
  int ConvertToInt(const std::string& str)
    {
    int result;
    std::istringstream iss (str);
    iss >> result;
    return result;
    }
}

//----------------------------------------------------------------------------
bool vtkVgNitfMetaDataParser::ParseDateTime(
  const std::vector<std::string>& mdata,
  const std::vector<std::string>& treMetaData,
  vtkVgTimeStamp& time)
{
  if (mdata.empty())
    {
    return false;
    }

  std::vector<std::string> tokens;

  for (size_t i = 0; i < mdata.size(); ++i)
    {
    size_t found = mdata[i].find("NITF_IDATIM");
    if (found != std::string::npos)
      {
      tokens = vgStringUtils::Split(mdata[i], '=');

      // We get time in CCYYMMDDhhmmss format, that is
      // (century, year, months, hours, minutes, and seconds)
      if (tokens[1].empty())
        {
        return false;
        }
      else
        {
        break;
        }
      }
    }

  std::string nitfTime = tokens[1];

  int years = 1970;
  int months = 0;
  int days = 0;
  int hrs = 0;
  int mins = 0;
  int secs = 0;
  // Milliseconds
  int ms = -1;

  years = ConvertToInt(std::string(nitfTime, 0, 4)) - 1900;
  months = ConvertToInt(std::string(nitfTime, 4, 2)) - 1;
  days = ConvertToInt(std::string(nitfTime, 6, 2));
  hrs = ConvertToInt(std::string(nitfTime, 8, 2));
  mins = ConvertToInt(std::string(nitfTime, 10, 2));
  secs = ConvertToInt(std::string(nitfTime, 12, 2));

  for (size_t i = 0; i < treMetaData.size(); ++i)
    {
    size_t found = treMetaData[i].find("ENGRDA");

    if (found != std::string::npos)
      {
      tokens = vgStringUtils::Split(treMetaData[i], '=');

      if (!tokens[1].empty())
        {
        std::string msstr;
        vtkVgNitfEngrda nitfEngdra;
        vtkVgNitfMetaDataParser::ParseEngrda(tokens[1], nitfEngdra);
        auto mseconds = nitfEngdra.Get("milliseconds");
        if (mseconds)
          {
          mseconds->GetData(msstr);

          if (!msstr.empty())
            {
            ms = ConvertToInt(msstr);
            }
          }
        }
      }
    }

  // If ms data not read from ENGRDA, look for it in the NITF_IMAGE_COMMENTS
  if (ms < 0)
    {
    for (size_t i = 0; i < mdata.size(); ++i)
      {
      size_t found = mdata[i].find("NITF_IMAGE_COMMENTS");
      if (found != std::string::npos)
        {
        tokens = vgStringUtils::Split(mdata[i], ' ');

        // Format: "     COLLECTION TIMESTAMP: 20150731184301.043659    UTC"
        for (int i = 0; i < tokens.size() - 2; ++i)
          {
          if (tokens[i] == "COLLECTION" && tokens[i + 1] == "TIMESTAMP:")
            {
            // Split date/time in secs from data after the decimal point
            tokens = vgStringUtils::Split(tokens[i + 2], '.');
            // Make sure we have exactly 3 characters
            tokens[1].resize(3, '0');
            ms = ConvertToInt(tokens[1]);
            }
          }

        break;
        }
      }
    }

  const vgCalendar::Date date = {years + 1900, months + 1, days};
  const int64_t msecs =
    ((vgCalendar::daysFromCivil(date) * 24 + hrs) * 60 + mins) * 60000 +
    secs * 1000 + (ms > -1 ? ms : 0);

  // To micoseconds
  time.SetTime(static_cast<double>(msecs) * 1e3);

  return true;
}

//----------------------------------------------------------------------------
bool vtkVgNitfMetaDataParser::ParseEngrda(const char* data, int len,
                                          vtkVgNitfEngrda& engrda)
{
  return engrda.Parse(data, len);
}
