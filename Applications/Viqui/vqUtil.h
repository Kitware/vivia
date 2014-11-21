/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqUtil_h
#define __vqUtil_h

#include <QString>

#include <vvIqr.h>

namespace vqUtil
{
enum UiStringFlag
{
  UI_Default = 0,
  UI_IncludeAccelerator = 0x1
};
Q_DECLARE_FLAGS(UiStringFlags, UiStringFlag)

QString uiIqrClassificationString(vvIqr::Classification,
                                  UiStringFlags flags = UI_Default);
}

Q_DECLARE_OPERATORS_FOR_FLAGS(vqUtil::UiStringFlags)

#endif
