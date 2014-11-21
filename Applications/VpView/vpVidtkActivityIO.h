/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
  vcl_vector<vidtk::activity_sptr> Activities;
  vcl_map<vtkVgActivity*, vidtk::activity_sptr> ActivityMap;
};

#endif // __vpVidtkActivityIO_h
