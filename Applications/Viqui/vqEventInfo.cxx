/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqEventInfo.h"

#include <QSettings>
#include <QStringList>

namespace // anonymous
{

//-----------------------------------------------------------------------------
vqEventInfo eventFromSettings(QSettings& settings, int type)
{
  vqEventInfo ei;
  ei.Type = type;

  const QString defaultName = QString("Unknown type %1").arg(type);

  settings.beginGroup(QString::number(type));
  ei.Name = settings.value("Name", defaultName).toString();
  settings.endGroup();

  return ei;
}

//-----------------------------------------------------------------------------
QList<vqEventInfo> eventsFromGroup(QSettings& settings,
                                   const QString& settingsGroup)
{
  // Load event types
  settings.beginGroup(settingsGroup);
  QList<vqEventInfo> events;
  foreach (QVariant v, settings.value("Types").toList())
    events.append(eventFromSettings(settings, v.toInt()));
  settings.endGroup();

  return events;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
QList<vqEventInfo> vqEventInfo::types(vqEventInfo::Groups groups)
{
  QSettings settings;
  settings.beginGroup("EventTypes");

  QList<vqEventInfo> events;

  if (groups.testFlag(vqEventInfo::Person))
    events += eventsFromGroup(settings, "Person");
  if (groups.testFlag(vqEventInfo::Vehicle))
    events += eventsFromGroup(settings, "Vehicle");

  return events;
}

//-----------------------------------------------------------------------------
QSet<int> vqEventInfo::searchableTypes()
{
  // Generate default value
  QVariantList allTypes;
  foreach (const vqEventInfo& ei, vqEventInfo::types())
    allTypes.append(ei.Type);

  // Get value from settings
  QSettings settings;
  settings.beginGroup("EventTypes");
  QVariantList values = settings.value("SearchableTypes", allTypes).toList();

  // Convert to set
  QSet<int> result;
  foreach (const QVariant& value, values)
    result.insert(value.toInt());
  return result;
}

//-----------------------------------------------------------------------------
int vqEventInfo::staticTypeArraySize()
{
  // Return the size of the fixed-size array containing threshold values for
  // all known event types... really should be a better way to do this!
  QSettings settings;
  settings.beginGroup("EventTypes");
  return settings.value("FixedSize", 0).toInt();
}

//-----------------------------------------------------------------------------
QString vqEventInfo::name(int type)
{
  foreach (const vqEventInfo& ei, vqEventInfo::types())
    {
    if (ei.Type == type)
      return ei.Name;
    }
  return QString("Unknown type %1").arg(type);
}
