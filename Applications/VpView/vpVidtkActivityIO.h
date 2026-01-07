// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpVidtkActivityIO_h
#define __vpVidtkActivityIO_h

#include "vpActivityIO.h"

#include <activity_detectors/activity.h>

class vpVidtkReader;
class vtkVgActivity;

class vpVidtkActivityIO : public vpActivityIO
{
public:
  vpVidtkActivityIO(vpVidtkReader& reader,
                    vtkVgActivityManager* activityManager,
                    vpActivityConfig* activityConfig);

  virtual ~vpVidtkActivityIO();

  virtual bool ReadActivities();

private:
  bool SetupActivity(vidtk::activity_sptr& vidtkActivity,
                     vtkVgActivity* vgActivity);

private:
  vpVidtkReader& Reader;
  std::vector<vidtk::activity_sptr> Activities;
  std::map<vtkVgActivity*, vidtk::activity_sptr> ActivityMap;
};

#endif // __vpVidtkActivityIO_h
