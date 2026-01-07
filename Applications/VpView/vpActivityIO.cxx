// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpActivityIO.h"

#include <vpActivityConfig.h>

#include <vtkVgActivityManager.h>

#include <assert.h>

//-----------------------------------------------------------------------------
vpActivityIO::vpActivityIO(vtkVgActivityManager* activityManager,
                           vpActivityConfig* activityConfig) :
  ActivityManager(activityManager), ActivityConfig(activityConfig)
{
  assert(activityManager);
  assert(activityConfig);
}
