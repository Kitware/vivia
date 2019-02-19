/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpActivityConfig.h"

#include "vgActivityType.h"
#include "vpConfigUtils.h"

#include "vtkVgActivityTypeRegistry.h"

#include <QColor>
#include <QSettings>
#include <QtDebug>

#include <assert.h>
#include <vector>

namespace
{

//-----------------------------------------------------------------------------
struct BuiltInActivityType
{
  const char* Id;
  const char* Name;

  double Color[3];

  bool UseRandomColors;

  int IconIndex;
};

//-----------------------------------------------------------------------------
const BuiltInActivityType BuiltInTypes[] =
{
  { "ACTIVITY", "Unknown", { 1.00, 1.00, 1.00 }, false, -1 }
};

enum { NumBuiltInActivityTypes = sizeof(BuiltInTypes) / sizeof(BuiltInActivityType) };

} // end anonymous namespace

//-----------------------------------------------------------------------------
class vpActivityConfig::vpActivityConfigInternal
{
public:
  QSettings Settings;
  bool Dirty;
};

//-----------------------------------------------------------------------------
vpActivityConfig::vpActivityConfig(vtkVgActivityTypeRegistry* registry)
{
  this->Internal = new vpActivityConfigInternal;
  this->Internal->Dirty = false;
  this->Internal->Settings.beginGroup("ActivityTypes");

  this->ActivityTypeRegistry = registry;
  this->ActivityTypeRegistry->Register(0);

  this->SetupBuiltInTypes();
  this->ReadActivityTypes(this->Internal->Settings);
  this->WriteActivityTypes();
}

//-----------------------------------------------------------------------------
vpActivityConfig::~vpActivityConfig()
{
  // Write out any config changes made by the user.
  if (this->Internal->Dirty)
    {
    this->WriteActivityTypes();
    }

  this->ActivityTypeRegistry->UnRegister(0);
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vpActivityConfig::ReadActivityTypes(QSettings& settings)
{
  int size = settings.beginReadArray("ActivityTypes");

  for (int i = 0; i < size; ++i)
    {
    settings.setArrayIndex(i);

    vgActivityType type;
    type.SetId(qPrintable(settings.value("Id").toString()));
    type.SetName(qPrintable(settings.value("Name").toString()));

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

    type.SetUseRandomColors(settings.value("UseRandomColors").toBool());
    type.SetIconIndex(settings.value("IconIndex", -1).toInt());

    const auto& displayMode =
      settings.value("DisplayMode", vgActivityType::GetDisplayModeString(0));
    type.SetDisplayMode(
      this->GetDisplayModeFromString(qPrintable(displayMode.toString())));

    int id = this->GetIdInternal(type.GetId());
    if (id == -1)
      {
      this->ActivityTypeRegistry->AddType(type);
      }
    else
      {
      this->ActivityTypeRegistry->SetType(id, type);
      }
    }

  settings.endArray();
}

//-----------------------------------------------------------------------------
void vpActivityConfig::WriteActivityTypes()
{
  this->Internal->Settings.remove("");
  this->Internal->Settings.beginWriteArray("ActivityTypes",
                                           this->GetNumberOfTypes());

  for (int i = 0, end = this->ActivityTypeRegistry->GetNumberOfTypes(); i < end;
       ++i)
    {
    const vgActivityType& type = this->ActivityTypeRegistry->GetType(i);
    this->Internal->Settings.setArrayIndex(i);

    this->Internal->Settings.setValue("Id", type.GetId());
    this->Internal->Settings.setValue("Name", type.GetName());

    double c[3];
    QColor color;
    type.GetColor(c[0], c[1], c[2]);
    color.setRgbF(c[0], c[1], c[2]);
    vpConfigUtils::WriteColor("Color", color, this->Internal->Settings);

    if (type.GetHasSecondaryColor())
      {
      type.GetSecondaryColor(c[0], c[1], c[2]);
      color.setRgbF(c[0], c[1], c[2]);
      vpConfigUtils::WriteColor("SecondaryColor", color, this->Internal->Settings);
      }

    this->Internal->Settings.setValue("UseRandomColors", type.GetUseRandomColors());
    this->Internal->Settings.setValue("IconIndex", type.GetIconIndex());

    if (type.GetDisplayMode() > 0)
      {
      this->Internal->Settings.setValue(
        "DisplayMode", type.GetDisplayModeString(type.GetDisplayMode()));
      }
    }

  this->Internal->Settings.endArray();
}

//-----------------------------------------------------------------------------
void vpActivityConfig::SetupBuiltInTypes()
{
  assert(this->ActivityTypeRegistry->GetNumberOfTypes() == 0);

  // Add information for 'built-in' event types. These settings may be
  // overridden by the application event configuration.
  for (int i = 0; i < NumBuiltInActivityTypes; ++i)
    {
    const BuiltInActivityType& baseType = BuiltInTypes[i];

    vgActivityType type;
    type.SetId(baseType.Id);
    type.SetName(baseType.Name);
    type.SetColor(baseType.Color[0],
                  baseType.Color[1],
                  baseType.Color[2]);

    type.SetUseRandomColors(baseType.UseRandomColors);
    type.SetIconIndex(baseType.IconIndex);

    this->ActivityTypeRegistry->AddType(type);
    }
}

//-----------------------------------------------------------------------------
int vpActivityConfig::GetNumberOfTypes()
{
  return this->ActivityTypeRegistry->GetNumberOfTypes();
}

//-----------------------------------------------------------------------------
const vgActivityType& vpActivityConfig::GetActivityType(int id)
{
  return this->ActivityTypeRegistry->GetType(id);
}

//-----------------------------------------------------------------------------
void vpActivityConfig::SetActivityType(int id, const vgActivityType& type)
{
  this->ActivityTypeRegistry->SetType(id, type);
  this->Internal->Dirty = true;
}

//-----------------------------------------------------------------------------
const vgEntityType& vpActivityConfig::GetEntityType(int id)
{
  return this->ActivityTypeRegistry->GetEntityType(id);
}

//-----------------------------------------------------------------------------
void vpActivityConfig::SetEntityType(int id, const vgEntityType& type)
{
  this->ActivityTypeRegistry->SetEntityType(id, type);
  this->Internal->Dirty = true;
}

//-----------------------------------------------------------------------------
void vpActivityConfig::LoadFromFile(const char* filename)
{
  QSettings settings(filename, QSettings::IniFormat);
  settings.beginGroup("ActivityTypes");
  this->ReadActivityTypes(settings);
}

//-----------------------------------------------------------------------------
int vpActivityConfig::GetId(const char* str)
{
  int id = this->GetIdInternal(str);
  if (id == -1)
    {
    qDebug() << "Activity type id is not registered:" << str;
    return 0; // Unregistered types refer to "Unknown" dummy type.
    }
  return id;
}

//-----------------------------------------------------------------------------
void vpActivityConfig::MarkTypeUsed(int index)
{
  this->ActivityTypeRegistry->MarkTypeUsed(index);
}

//-----------------------------------------------------------------------------
void vpActivityConfig::MarkAllTypesUnused()
{
  this->ActivityTypeRegistry->MarkAllTypesUnused();
}

//-----------------------------------------------------------------------------
int vpActivityConfig::GetIdInternal(const char* str)
{
  for (int i = 0, end = this->ActivityTypeRegistry->GetNumberOfTypes(); i < end;
       ++i)
    {
    if (strcmp(str, this->ActivityTypeRegistry->GetType(i).GetId()) == 0)
      {
      return static_cast<int>(i);
      }
    }
  return -1;
}

//-----------------------------------------------------------------------------
int vpActivityConfig::GetDisplayModeFromString(const char* str)
{
  for (int i = 0; i < vgActivityType::NumDisplayModes; ++i)
    {
    if (strcmp(str, vgActivityType::GetDisplayModeString(i)) == 0)
      {
      return i;
      }
    }
  return -1;
}
