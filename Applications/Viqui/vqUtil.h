// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
