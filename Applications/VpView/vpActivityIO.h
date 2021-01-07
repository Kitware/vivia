// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
