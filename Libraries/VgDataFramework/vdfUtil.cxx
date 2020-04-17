/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfDataSource.h"
#include "vdfNamespace.h"
#include "vdfTrackData.h"
#include "vdfTrackId.h"

#include <vvEvent.h>
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
  QTE_REGISTER_METATYPE(QList<vvEvent>);
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
void vdf::registerMetaTypes()
{
  qtOnce(registerMetaTypesOnce, &::registerMetaTypes);
}
