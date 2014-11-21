/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpActivityIO_h
#define __vpActivityIO_h

class vpActivityConfig;
class vtkVgActivityManager;

class vpActivityIO
{
public:
  vpActivityIO(vtkVgActivityManager* activityManager,
               vpActivityConfig* activityConfig);

  virtual ~vpActivityIO() {}

  virtual bool ReadActivities() = 0;

protected:
  vtkVgActivityManager* ActivityManager;
  vpActivityConfig* ActivityConfig;
};

#endif // __vpActivityIO_h
