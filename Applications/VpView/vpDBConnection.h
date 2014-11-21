/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
