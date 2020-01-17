/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgFilRecord.h"

#include <vgCheckArg.h>

namespace // anonymous
{

//-----------------------------------------------------------------------------
bool extract(QByteArray& r, uint& value)
{
  const auto n = r.indexOf(' ');
  CHECK_ARG(n >= 0, false);

  bool result = false;
  value = r.left(n).toUInt(&result);
  r = r.mid(n + 1).trimmed();
  return result;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vgFilRecord::vgFilRecord(const QByteArray& rawRecord)
{
  auto r = rawRecord.trimmed();

  extract(r, this->FrameNumber) &&
  extract(r, this->FrameWidth) &&
  extract(r, this->FrameHeight) &&
  (this->FramePath = r, true);
}

//END vgFilVideo
