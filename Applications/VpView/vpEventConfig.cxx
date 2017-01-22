/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpEventConfig.h"

#include "vpConfigUtils.h"
#include "vgEventType.h"

#include "vtkVgEventTypeRegistry.h"

#if defined(VISGUI_USE_VIDTK) && 0
#include <event_detectors/event_types.h>
#endif

#include <QColor>
#include <QSettings>
#include <QtDebug>

#include <assert.h>
#include <vector>

namespace { enum { MaxEventTypes = 1 }; }

//-----------------------------------------------------------------------------
class vpEventConfig::vpEventConfigInternal
{
public:
  vpEventConfigInternal()
    : Dirty(false)
    {
    std::fill(this->IdToEventTypeIndex,
              this->IdToEventTypeIndex + MaxEventTypes,
              -1);
    }

public:
  QSettings Settings;
  int IdToEventTypeIndex[MaxEventTypes];

  bool Dirty;
};

//-----------------------------------------------------------------------------
vpEventConfig::vpEventConfig(vtkVgEventTypeRegistry* registry)
{
  this->Internal = new vpEventConfigInternal;
  this->Internal->Settings.beginGroup("EventTypes");

  this->Registry = registry;
  this->Registry->Register(0);

  this->ReadEventTypes(this->Internal->Settings);
  this->WriteEventTypes();
}

//-----------------------------------------------------------------------------
vpEventConfig::~vpEventConfig()
{
  // Write out any config changes made by the user.
  if (this->Internal->Dirty)
    {
    this->WriteEventTypes();
    }

  this->Registry->UnRegister(0);
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vpEventConfig::ReadEventTypes(QSettings& settings)
{
  int size = settings.beginReadArray("EventTypes");

  for (int i = 0; i < size; ++i)
    {
    settings.setArrayIndex(i);
    QString idStr = settings.value("Id").toString();

    // Look up the numeric id based on the type id string.
    int id = this->GetIdFromString(idStr.toAscii());
    if (id == -1)
      {
      qDebug() << "Event type id not found:" << idStr;
      continue;
      }

    vgEventType type;
    type.SetId(id);
    type.SetName(settings.value("Name").toString().toAscii());

    type.SetMinTracks(settings.value("MinTracks", -1).toInt());
    type.SetMaxTracks(settings.value("MaxTracks", -1).toInt());

    // If min/max number of tracks was not given, assume a single-track event.
    if (type.GetMinTracks() == -1)
      {
      type.SetMinTracks(1);
      type.SetMaxTracks(1);
      }

    QColor color = vpConfigUtils::ReadColor("Color", settings);
    type.SetColor(color.redF(), color.greenF(), color.blueF());

    QColor colorSecondary = vpConfigUtils::ReadColor("SecondaryColor", settings);
    if (colorSecondary.isValid())
      {
      type.SetHasSecondaryColor(true);
      type.SetSecondaryColor(colorSecondary.redF(), colorSecondary.greenF(),
                             colorSecondary.blueF());
      }
    else
      {
      type.SetHasSecondaryColor(false);
      }

    type.SetIconIndex(settings.value("IconIndex", -1).toInt());
    type.SetSecondaryIconIndex(settings.value("SecondaryIconIndex", -1).toInt());

    type.SetDisplayMode(
      this->GetDisplayModeFromString(
        settings.value("DisplayMode", vgEventType::GetDisplayModeString(0))
        .toString().toAscii()));

    // Add or assign to event type vector.
    if (id < 0 || id >= MaxEventTypes)
      {
      qDebug() << "Event type id is out of range - ignoring.";
      }
    else if (this->Internal->IdToEventTypeIndex[id] == -1)
      {
      int end = this->Registry->GetNumberOfTypes();
      this->Registry->AddType(type);
      this->Internal->IdToEventTypeIndex[id] = end;
      }
    else
      {
      this->Registry->SetType(
        static_cast<int>(this->Internal->IdToEventTypeIndex[id]), type);
      }
    }

  settings.endArray();
}

//-----------------------------------------------------------------------------
void vpEventConfig::WriteEventTypes()
{
  this->Internal->Settings.remove("");
  this->Internal->Settings.beginWriteArray("EventTypes",
                                           this->GetNumberOfTypes());

  for (int i = 0, end = this->Registry->GetNumberOfTypes(); i < end; ++i)
    {
    const vgEventType& type = this->Registry->GetType(i);
    this->Internal->Settings.setArrayIndex(i);

#if defined(VISGUI_USE_VIDTK) && 0
    this->Internal->Settings.setValue("Id",
                                      vidtk::event_types::event_names
                                      [type.GetId()]);
#else
    this->Internal->Settings.setValue("Id", "Unknown");
#endif

    this->Internal->Settings.setValue("Name", type.GetName());

    if (type.GetMinTracks() != 1 || type.GetMaxTracks() != 1)
      {
      this->Internal->Settings.setValue("MinTracks", type.GetMinTracks());
      if (type.GetMaxTracks() != -1)
        {
        this->Internal->Settings.setValue("MaxTracks", type.GetMaxTracks());
        }
      }

    double c[3];

    QColor color;
    type.GetColor(c[0], c[1], c[2]);
    color.setRgbF(c[0], c[1], c[2]);
    vpConfigUtils::WriteColor("Color", color, this->Internal->Settings);

    if (type.GetHasSecondaryColor())
      {
      type.GetSecondaryColor(c[0], c[1], c[2]);
      color.setRgbF(c[0], c[1], c[2]);
      vpConfigUtils::WriteColor("SecondaryColor", color,
                                this->Internal->Settings);
      }

    this->Internal->Settings.setValue("IconIndex", type.GetIconIndex());
    if (type.GetSecondaryIconIndex() != -1)
      {
      this->Internal->Settings.setValue("SecondaryIconIndex",
                                        type.GetSecondaryIconIndex());
      }

    if (type.GetDisplayMode() > 0)
      {
      this->Internal->Settings.setValue(
        "DisplayMode", type.GetDisplayModeString(type.GetDisplayMode()));
      }
    }

  this->Internal->Settings.endArray();
}

//-----------------------------------------------------------------------------
int vpEventConfig::GetNumberOfTypes()
{
  return this->Registry->GetNumberOfTypes();
}

//-----------------------------------------------------------------------------
int vpEventConfig::GetEventTypeIndex(int id)
{
#if defined(VISGUI_USE_VIDTK) && 0
  if (id < 0 || id >= MaxEventTypes)
    {
    return -1;
    }
  return static_cast<int>(this->Internal->IdToEventTypeIndex[id]);
#else
  return -1;
#endif
}

//-----------------------------------------------------------------------------
const vgEventType& vpEventConfig::GetEventTypeByIndex(int index)
{
  return this->Registry->GetType(index);
}

//-----------------------------------------------------------------------------
const vgEventType& vpEventConfig::GetEventTypeById(int id)
{
  return this->Registry->GetTypeById(id);
}

//-----------------------------------------------------------------------------
const vgEntityType& vpEventConfig::GetEntityType(int id)
{
  return this->Registry->GetEntityType(id);
}

//-----------------------------------------------------------------------------
void vpEventConfig::SetEntityType(int id, const vgEntityType& type)
{
  this->Registry->SetEntityType(id, type);
  this->Internal->Dirty = true;
}

//-----------------------------------------------------------------------------
void vpEventConfig::LoadFromFile(const char* filename)
{
  QSettings settings(filename, QSettings::IniFormat);
  settings.beginGroup("EventTypes");
  this->ReadEventTypes(settings);
}

//-----------------------------------------------------------------------------
int vpEventConfig::GetIdFromString(const char* str)
{
#if defined(VISGUI_USE_VIDTK) && 0
  for (size_t i = 0; i < MaxEventTypes; ++i)
    {
    if (strcmp(str, vidtk::event_types::event_names[i]) == 0)
      {
      return static_cast<int>(i);
      }
    }
#endif
  return -1;
}

//-----------------------------------------------------------------------------
const char* vpEventConfig::GetStringFromId(int id)
{
#if defined(VISGUI_USE_VIDTK) && 0
  assert(id >= 0 && id < MaxEventTypes);
  return vidtk::event_types::event_names[id];
#else
  return "Unknown";
#endif
}

//-----------------------------------------------------------------------------
int vpEventConfig::GetDisplayModeFromString(const char* str)
{
  for (int i = 0; i < vgEventType::NumDisplayModes; ++i)
    {
    if (strcmp(str, vgEventType::GetDisplayModeString(i)) == 0)
      {
      return i;
      }
    }
  return -1;
}

//-----------------------------------------------------------------------------
void vpEventConfig::MarkTypeUsed(int index)
{
  this->Registry->MarkTypeUsed(index);
}

//-----------------------------------------------------------------------------
void vpEventConfig::MarkAllTypesUnused()
{
  this->Registry->MarkAllTypesUnused();
}
