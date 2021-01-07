// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vqUtil.h"

//-----------------------------------------------------------------------------
QString vqUtil::uiIqrClassificationString(
  vvIqr::Classification classification, vqUtil::UiStringFlags flags)
{
  const char* bareResult = 0;
  const char* resultWithAccel = 0;

  switch (classification)
    {
    case vvIqr::PositiveExample:
      bareResult = "Relevant";
      resultWithAccel = "&Relevant";
      break;
    case vvIqr::NegativeExample:
      bareResult = "Not Relevant";
      resultWithAccel = "&Not Relevant";
      break;
    default:
      bareResult = "Unrated";
      resultWithAccel = "&Unrated";
      break;
    }

  return (flags.testFlag(vqUtil::UI_IncludeAccelerator)
          ? QString(resultWithAccel)
          : QString(bareResult));
}
