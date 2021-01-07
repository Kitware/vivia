// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vdfDataSource.h"
#include "vdfNamespace.h"
#include "vdfTrackData.h"
#include "vdfTrackId.h"

#include <vvTrack.h>

#include <vgTimeMap.h>

#include <qtGlobal.h>
#include <qtOnce.h>

#include <QHash>
#include <QMetaType>
#include <QSet>

namespace // anonymous
{

QTE_ONCE(registerMetaTypesOnce);

//-----------------------------------------------------------------------------
void registerMetaTypes()
{
  // vdfDataSource
  QTE_REGISTER_METATYPE(vdfDataSource::Status);

  // vdfTrackSourceInterface
  QTE_REGISTER_METATYPE(vdfTrackId);
  QTE_REGISTER_METATYPE(QList<vvTrackState>);
  QTE_REGISTER_METATYPE(vdfTrackAttributes);
  QTE_REGISTER_METATYPE(vgTimeMap<vdfTrackAttributes>);
  QTE_REGISTER_METATYPE(vdfTrackStateScalars);
  QTE_REGISTER_METATYPE(vdfTrackScalarDataCollection);
  QTE_REGISTER_METATYPE(vvTrackObjectClassification);
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
void vdf::registerMetaTypes()
{
  qtOnce(registerMetaTypesOnce, &::registerMetaTypes);
}
