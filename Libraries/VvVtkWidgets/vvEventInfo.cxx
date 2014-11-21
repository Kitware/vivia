/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvEventInfo.h"

#include <QSettings>

#include <qtScopedSettingGroup.h>
#include <QColor>

namespace // anonymous
{

//-----------------------------------------------------------------------------
vvEventInfo eventFromSettings(QSettings& settings, int type)
{
  qtScopedSettingGroup sg(settings, QString::number(type));

  vvEventInfo ei;
  ei.Type = type;

  const QString defaultName = QString("Unknown type %1").arg(type);
  ei.Name = settings.value("Name", QString(defaultName)).toString();

  // Set default colors
  ei.PenColor = QColor::fromRgb(240, 240, 240);
  ei.BackgroundColor = QColor::fromRgb(112, 112, 112);
  ei.ForegroundColor = Qt::white;

  // Load colors from settings
  ei.PenColor.read(settings, "PenColor");
  ei.BackgroundColor.read(settings, "BackgroundColor");
  ei.ForegroundColor.read(settings, "ForegroundColor");

  return ei;
}

//-----------------------------------------------------------------------------
QList<vvEventInfo> eventsFromGroup(
  QSettings& settings, const QString& settingsGroup)
{
  qtScopedSettingGroup sg(settings, settingsGroup);

  // Get list of user defined event types
  QList<int> types;
  foreach (QVariant v, settings.value("Types").toList())
    types.append(v.toInt());

  // Load each event type
  QList<vvEventInfo> events;
  foreach (int type, types)
    events.append(eventFromSettings(settings, type));

  return events;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
QList<vvEventInfo> vvEventInfo::eventTypes(vvEventInfo::Groups groups)
{
  QSettings settings;
  settings.beginGroup("EventTypes");

  QList<vvEventInfo> events;

  if (groups.testFlag(vvEventInfo::Person))
    events += eventsFromGroup(settings, "Person");
  if (groups.testFlag(vvEventInfo::Vehicle))
    events += eventsFromGroup(settings, "Vehicle");

  return events;
}
