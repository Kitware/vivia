/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsEventInfo.h"

#include <QColor>
#include <QSettings>

#include <qtScopedSettingGroup.h>

#include <vgEventType.h>

#include <vvEventSetInfo.h>

#include <vvEventInfo.h>

namespace // anonymous
{

//-----------------------------------------------------------------------------
static const vsEventInfo::Template emptyEventTemplate =
{
  0, 0, { 240, 240, 240, 112, 112, 112 }
};

static const vsEventInfo::Template eventsGeneral[] =
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

static const vsEventInfo::Template eventsUser[] =
{
  emptyEventTemplate
};

//-----------------------------------------------------------------------------
void loadColor(QSettings& settings, vgColor& color, QString key,
               const QColor& defaultColor)
{
  color = vgColor::read(settings, key, defaultColor);
}

//-----------------------------------------------------------------------------
void loadColor(QSettings& settings, vgColor& color, QString key,
               unsigned char defaultColor[])
{
  QColor q(defaultColor[0], defaultColor[1], defaultColor[2]);
  loadColor(settings, color, key, q);
}

//-----------------------------------------------------------------------------
vsEventInfo eventFromSettings(QSettings& settings, int type,
                              vsEventInfo::Template tpl)
{
  qtScopedSettingGroup sg(settings, QString::number(type));

  vsEventInfo ei;

  ei.group = vsEventInfo::eventGroup(type);
  ei.type = type;

  const QString defaultName =
    (tpl.name ? tpl.name : QString("Unknown type %1").arg(type));
  ei.name = settings.value("Name", defaultName).toString();

  loadColor(settings, ei.pcolor, "PenColor", tpl.color + 0);
  loadColor(settings, ei.bcolor, "BackgroundColor", tpl.color + 3);
  loadColor(settings, ei.fcolor, "ForegroundColor", Qt::white);

  return ei;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vgEventType vsEventInfo::toVgEventType() const
{
  vgEventType vget;
  vget.SetId(this->type);
  vget.SetName(qPrintable(this->name));
  vget.SetColor(this->pcolor.constData().array);
  vget.SetLabelForegroundColor(this->fcolor.constData().array);
  vget.SetLabelBackgroundColor(this->bcolor.constData().array);
  return vget;
}

//-----------------------------------------------------------------------------
vsEventInfo vsEventInfo::fromEventSetInfo(
  const vvEventSetInfo& vvInfo, int type)
{
  vsEventInfo vsInfo;

  vsInfo.group = eventGroup(type);
  vsInfo.type = type;
  vsInfo.name = vvInfo.Name;
  vsInfo.pcolor = vvInfo.PenColor;
  vsInfo.bcolor = vvInfo.BackgroundColor;
  vsInfo.fcolor = vvInfo.ForegroundColor;

  return vsInfo;
}

//-----------------------------------------------------------------------------
QList<vsEventInfo> vsEventInfo::events(
  QSettings& settings, const QString& settingsGroup,
  const vsEventInfo::Template* ta)
{
  // Convert bare templates to something we can reference by type
  QHash<int, vsEventInfo::Template> builtinTypes;
  if (ta)
    {
    for (int i = 0; ta[i].type; ++i)
      {
      builtinTypes.insert(ta[i].type, ta[i]);
      }
    }

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
    const vsEventInfo::Template t =
      (builtinTypes.contains(i) ? builtinTypes.value(i) : emptyEventTemplate);
    events.append(eventFromSettings(settings, i, t));
    }

  settings.endGroup();

  return events;
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

  foreach (vvEventInfo vvInfo, vvEventInfo::eventTypes(vvGroups))
    {
    vsEventInfo vsInfo;
    vsInfo.group = eventGroup(vvInfo.Type);
    vsInfo.type = vvInfo.Type;
    vsInfo.name = vvInfo.Name;
    vsInfo.pcolor = vvInfo.PenColor;
    vsInfo.bcolor = vvInfo.BackgroundColor;
    vsInfo.fcolor = vvInfo.ForegroundColor;
    vsEvents.append(vsInfo);
    }

  // Add vsPlay event types
  if (groups.testFlag(vsEventInfo::General))
    {
    QSettings settings;
    settings.beginGroup("EventTypes");
    vsEvents += events(settings, "General", eventsGeneral);
    }
  if (groups.testFlag(vsEventInfo::User))
    {
    QSettings settings;
    settings.beginGroup("EventTypes");
    vsEvents += events(settings, "User", eventsUser);
    }

  return vsEvents;
}

//-----------------------------------------------------------------------------
vsEventInfo::Group vsEventInfo::eventGroup(int eventType, Group hint)
{
  if (hint != Unknown)
    {
    // Always trust the hint, if provided
    return hint;
    }
  else if (eventType > 0)
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
