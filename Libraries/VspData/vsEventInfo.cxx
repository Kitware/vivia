/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsEventInfo.h"

#include <QColor>
#include <QSettings>

#include <qtScopedSettingsGroup.h>

#include <vgEventType.h>

#include <vvEventSetInfo.h>

#include <vvEventInfo.h>

namespace // anonymous
{

//-----------------------------------------------------------------------------
struct EventInfoTemplate
{
  int type;
  const char* name;
  unsigned char color[6];
};

//-----------------------------------------------------------------------------
static const EventInfoTemplate emptyEventTemplate =
{
  0, 0, { 240, 240, 240, 112, 112, 112 }
};

static const EventInfoTemplate eventsGeneral[] =
{
  { vsEventInfo::Tripwire,        "Tripwire",
    { 240, 240, 240, 112, 112, 112 } },
  { vsEventInfo::EnteringRegion,  "Entering Region",
    { 240, 240, 240, 112, 112, 112 } },
  { vsEventInfo::ExitingRegion,   "Exiting Region",
    { 240, 240, 240, 112, 112, 112 } },
  { vsEventInfo::Annotation,      "Annotation",
    { 255, 255, 255, 64, 64, 64 } },
  emptyEventTemplate
};

static const EventInfoTemplate eventsUser[] =
{
  emptyEventTemplate
};

//-----------------------------------------------------------------------------
void loadColor(QSettings& settings, double (&color)[3], QString key,
               const QColor& defaultColor)
{
  vgColor::read(settings, key, defaultColor).fillArray(color);
}

//-----------------------------------------------------------------------------
void loadColor(QSettings& settings, double (&color)[3], QString key,
               unsigned char defaultColor[])
{
  QColor q(defaultColor[0], defaultColor[1], defaultColor[2]);
  loadColor(settings, color, key, q);
}

//-----------------------------------------------------------------------------
vsEventInfo eventFromSettings(QSettings& settings, int type,
                              EventInfoTemplate tpl)
{
  vsEventInfo ei;

  with_expr (qtScopedSettingsGroup{settings, QString::number(type)})
    {

    ei.type = type;
    const QString defaultName =
      (tpl.name ? tpl.name : QString("Unknown type %1").arg(type));
    ei.name = settings.value("Name", defaultName).toString();

    loadColor(settings, ei.pcolor, "PenColor", tpl.color + 0);
    loadColor(settings, ei.bcolor, "BackgroundColor", tpl.color + 3);
    loadColor(settings, ei.fcolor, "ForegroundColor", Qt::white);
    }

  return ei;
}

//-----------------------------------------------------------------------------
QList<vsEventInfo> eventsFromGroup(QSettings& settings,
                                   const QString& settingsGroup,
                                   const EventInfoTemplate* ta)
{
  // Convert bare templates to something we can reference by type
  QHash<int, EventInfoTemplate> builtinTypes;
  for (int i = 0; ta[i].type; ++i)
    builtinTypes.insert(ta[i].type, ta[i]);

  // Get list of user defined event types
  QList<int> types;
  settings.beginGroup(settingsGroup);
  const QVariant typesData = settings.value("Types");
  if (typesData.canConvert(QVariant::Int))
    {
    types.append(typesData.toInt());
    }
  else
    {
    foreach (QVariant v, typesData.toList())
      types.append(v.toInt());
    }

  // Add built-in types
  foreach (const auto i, builtinTypes.keys())
    {
    if (!types.contains(i))
      types.prepend(i);
    }

  // Load each event type
  QList<vsEventInfo> events;
  foreach (const auto i, types)
    {
    EventInfoTemplate t =
      (builtinTypes.contains(i) ? builtinTypes.value(i) : emptyEventTemplate);
    events.append(eventFromSettings(settings, i, t));
    }

  settings.endGroup();

  return events;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vgEventType vsEventInfo::toVgEventType() const
{
  vgEventType vget;
  vget.SetId(this->type);
  vget.SetName(qPrintable(this->name));
  vget.SetColor(this->pcolor[0], this->pcolor[1], this->pcolor[2]);
  vget.SetLabelForegroundColor(this->fcolor[0], this->fcolor[1],
                               this->fcolor[2]);
  vget.SetLabelBackgroundColor(this->bcolor[0], this->bcolor[1],
                               this->bcolor[2]);
  return vget;
}

//-----------------------------------------------------------------------------
vsEventInfo vsEventInfo::fromEventSetInfo(
  const vvEventSetInfo& vvInfo, int type)
{
  vsEventInfo vsInfo;

  vsInfo.type = type;
  vsInfo.name = vvInfo.Name;
  vgColor::fillArray(vvInfo.PenColor, vsInfo.pcolor);
  vgColor::fillArray(vvInfo.BackgroundColor, vsInfo.bcolor);
  vgColor::fillArray(vvInfo.ForegroundColor, vsInfo.fcolor);

  return vsInfo;
}

//-----------------------------------------------------------------------------
QList<vsEventInfo> vsEventInfo::events(vsEventInfo::Groups groups)
{
  vvEventInfo::Groups vvGroups = 0;
  if (groups.testFlag(vsEventInfo::Person))
    vvGroups |= vvEventInfo::Person;
  if (groups.testFlag(vsEventInfo::Vehicle))
    vvGroups |= vvEventInfo::Vehicle;

  QList<vsEventInfo> vsEvents;

  foreach (vvEventInfo vvEvent, vvEventInfo::eventTypes(vvGroups))
    {
    vsEventInfo vsEvent;
    vsEvent.type = vvEvent.Type;
    vsEvent.name = vvEvent.Name;
    vvEvent.PenColor.fillArray(vsEvent.pcolor);
    vvEvent.BackgroundColor.fillArray(vsEvent.bcolor);
    vvEvent.ForegroundColor.fillArray(vsEvent.fcolor);
    vsEvents.append(vsEvent);
    }

  // Add vsPlay event types
  if (groups.testFlag(vsEventInfo::General))
    {
    QSettings settings;
    settings.beginGroup("EventTypes");
    vsEvents += eventsFromGroup(settings, "General", eventsGeneral);
    }
  if (groups.testFlag(vsEventInfo::User))
    {
    QSettings settings;
    settings.beginGroup("EventTypes");
    vsEvents += eventsFromGroup(settings, "User", eventsUser);
    }

  return vsEvents;
}

//-----------------------------------------------------------------------------
vsEventInfo::Group vsEventInfo::eventGroup(int eventType)
{
  if (eventType > 0)
    {
    // Anything positive is considered a classifier (but we don't try to
    // determine a specific group)
    return Classifier;
    }
  else if (eventType <= UserType)
    {
    return User;
    }
  else if (eventType <= QueryAlert)
    {
    return Alert;
    }
  else if (eventType <= Tripwire)
    {
    // Anything starting with the built-in types (-3000) but before QueryAlert
    // is considered General... which includes the vsTrackInfo types, but we
    // don't have a Group for that, so too bad...
    return General;
    }

  return Unknown;
}
