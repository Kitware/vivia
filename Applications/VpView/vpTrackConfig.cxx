/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpTrackConfig.h"

#include "vpConfigUtils.h"

#include <vgTrackType.h>

#include <vtkVgTrackTypeRegistry.h>

#include <QColor>
#include <QSettings>

//-----------------------------------------------------------------------------
class vpTrackConfig::vpTrackConfigInternal
{
public:
  vpTrackConfigInternal() : Dirty(false) {}

public:
  QSettings Settings;
  bool Dirty;
};

//-----------------------------------------------------------------------------
vpTrackConfig::vpTrackConfig(vtkVgTrackTypeRegistry* registry)
{
  this->Internal = new vpTrackConfigInternal;
  this->Internal->Settings.beginGroup("TrackTypes");

  this->Registry = registry;
  this->Registry->Register(0);

  this->ReadTrackTypes(this->Internal->Settings);
  this->WriteTrackTypes();
}

//-----------------------------------------------------------------------------
vpTrackConfig::~vpTrackConfig()
{
  // Write out any config changes made by the user.
  if (this->Internal->Dirty)
    {
    this->WriteTrackTypes();
    }

  this->Registry->UnRegister(0);
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vpTrackConfig::ReadTrackTypes(QSettings& settings)
{
  int size = settings.beginReadArray("TrackTypes");

  for (int i = 0; i < size; ++i)
    {
    settings.setArrayIndex(i);

    vgTrackType type;

    QString id = settings.value("Id").toString();
    type.SetId(qPrintable(id));

    QColor color = vpConfigUtils::ReadColor("Color", settings);
    if (color.isValid())
      {
      type.SetColor(color.redF(), color.greenF(), color.blueF());
      }

    int index = this->Registry->GetTypeIndex(type.GetId());

    // Add or assign to track type vector.
    if (index == -1)
      {
      this->Registry->AddType(type);
      }
    else
      {
      this->Registry->SetType(index, type);
      }
    }

  settings.endArray();
}

//-----------------------------------------------------------------------------
void vpTrackConfig::WriteTrackTypes()
{
  this->Internal->Settings.remove("");
  this->Internal->Settings.beginWriteArray("TrackTypes",
                                           this->GetNumberOfTypes());

  int index = 0;
  for (int i = 0, end = this->Registry->GetNumberOfTypes(); i < end; ++i)
    {
    const vgTrackType& type = this->Registry->GetType(i);
    const char* id = type.GetId();
    if (!*id)
      {
      continue;
      }
    this->Internal->Settings.setArrayIndex(index);
    this->Internal->Settings.setValue("Id", id);

    double c[3];
    QColor color;
    type.GetColor(c[0], c[1], c[2]);
    color.setRgbF(c[0], c[1], c[2]);
    vpConfigUtils::WriteColor("Color", color, this->Internal->Settings);
    ++index;
    }

  this->Internal->Settings.endArray();
}

//-----------------------------------------------------------------------------
int vpTrackConfig::GetNumberOfTypes()
{
  return this->Registry->GetNumberOfTypes();
}

//-----------------------------------------------------------------------------
int vpTrackConfig::GetTrackTypeIndex(const char* id)
{
  return this->Registry->GetTypeIndex(id);
}

//-----------------------------------------------------------------------------
const vgTrackType& vpTrackConfig::GetTrackTypeByIndex(int index)
{
  return this->Registry->GetType(index);
}

//-----------------------------------------------------------------------------
const vgEntityType& vpTrackConfig::GetEntityType(int id)
{
  return this->Registry->GetEntityType(id);
}

//-----------------------------------------------------------------------------
void vpTrackConfig::SetEntityType(int id, const vgEntityType& type)
{
  this->Registry->SetEntityType(id, type);
  this->Internal->Dirty = true;
}

//-----------------------------------------------------------------------------
void vpTrackConfig::LoadFromFile(const char* filename)
{
  QSettings settings(filename, QSettings::IniFormat);
  settings.beginGroup("TrackTypes");
  this->ReadTrackTypes(settings);
}

//-----------------------------------------------------------------------------
void vpTrackConfig::MarkTypeUsed(int index)
{
  this->Registry->MarkTypeUsed(index);
}

//-----------------------------------------------------------------------------
void vpTrackConfig::MarkAllTypesUnused()
{
  this->Registry->MarkAllTypesUnused();
}

//-----------------------------------------------------------------------------
void vpTrackConfig::AddType(const vgTrackType& tt)
{
  this->Registry->AddType(tt);
  this->Internal->Dirty = true;
}

//-----------------------------------------------------------------------------
void vpTrackConfig::SetType(int index, const vgTrackType& tt)
{
  this->Registry->SetType(index, tt);
  this->Internal->Dirty = true;
}
