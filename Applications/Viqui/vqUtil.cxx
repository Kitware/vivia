/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
