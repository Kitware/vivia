#ifndef __vpDBSessionMetaData_h
#define __vpDBSessionMetaData_h

#include <map>
#include <string>

struct vpDBSessionMetaData
{
  long SessionId;

  std::string SessionType;
  int TracksSessionId;
  int EventsSessionId;
  int ActivitiesSessionId;
  int AOIId;
  std::string DateCreated;

  std::map<std::string, std::string> Parameters;
};

#endif // __vpDBSessionMetaData_h
