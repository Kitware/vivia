// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
