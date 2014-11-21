/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgNitfMetaDataParser.h"

#include "vtkVgNitfEngrda.h"

#include <vgStringUtils.h>

#include <vtkVgTimeStamp.h>

// Boost includes
#include <boost/date_time/gregorian/conversion.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// C/C++ includes
#include <ctime>

namespace bt = boost::posix_time;

//----------------------------------------------------------------------------
namespace
{
  // FIXME Move this code somewhere else
  // Convert boost posix time to std time
  std::time_t Convert(const bt::ptime& pt)
    {
      bt::ptime timet_start(boost::gregorian::date(1970,1,1));
      bt::time_duration diff = pt - timet_start;
      return diff.total_milliseconds();
    }

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
  std::tm utm;

  int years = 1970;
  int months = 0;
  int days = 0;
  int hrs = 0;
  int mins = 0;
  int secs = 0;
  // Milliseconds
  int ms = 0;

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
        nitfEngdra.Get("milliseconds")->GetData(msstr);

        if (!msstr.empty())
          {
          ms = ConvertToInt(msstr);
          }
        }
      }
    }

  utm.tm_mday = days;
  utm.tm_mon = months;
  utm.tm_year = years;

  boost::posix_time::ptime pt (boost::gregorian::date_from_tm(utm),
    bt::hours(hrs) + bt::minutes(mins) + bt::seconds(secs) +
    bt::milliseconds(ms));

  // To micoseconds
  time.SetTime(static_cast<double>(Convert(pt)) * 1e3);

  return true;
}

//----------------------------------------------------------------------------
bool vtkVgNitfMetaDataParser::ParseEngrda(const char* data, int len,
                                          vtkVgNitfEngrda& engrda)
{
  return engrda.Parse(data, len);
}
