// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpDBConnection_h
#define __vpDBConnection_h

struct vpDBConnection
{
  std::string DatabaseName;
  std::string Driver;
  std::string Password;
  std::string Port;
  std::string Hostname;
  std::string Username;
};

#endif // __vpDBConnection_h
