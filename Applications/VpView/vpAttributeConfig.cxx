// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpAttributeConfig.h"

#include "vpConfigUtils.h"

#include <QColor>
#include <QSettings>
#include <QtDebug>

#include <assert.h>
#include <vector>

//-----------------------------------------------------------------------------
class vpAttributeConfig::vpAttributeConfigInternal
{
public:
  vpAttributeConfigInternal()
    : Dirty(false)
    {
    }

public:
  QSettings Settings;
  QVector<vpAttributeConfig::vpAttributeType> AttributeTypes;
  QHash<int, int> IdToTypeMap;
  bool Dirty;
};

//-----------------------------------------------------------------------------
vpAttributeConfig::vpAttributeConfig()
{
  this->Internal = new vpAttributeConfigInternal;
  this->Internal->Settings.beginGroup("AttributeTypes");
  this->readAttributeTypes(this->Internal->Settings);
  this->writeAttributeTypes();
}

//-----------------------------------------------------------------------------
vpAttributeConfig::~vpAttributeConfig()
{
  // Write out any config changes made by the user.
  if (this->Internal->Dirty)
    {
    this->writeAttributeTypes();
    }

  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vpAttributeConfig::readAttributeTypes(QSettings& settings)
{
  int size = settings.beginReadArray("AttributeTypes");

  for (int i = 0; i < size; ++i)
    {
    settings.setArrayIndex(i);

    vpAttributeType type;
    type.Id = settings.value("Id").toInt();
    type.Name = settings.value("Name").toString();

    int index = this->getAttributeTypeIndex(type.Id);
    if (index == -1)
      {
      // Add new type
      index = this->Internal->AttributeTypes.size();
      this->Internal->AttributeTypes.push_back(type);
      this->Internal->IdToTypeMap.insert(type.Id, index);
      }
    else
      {
      // Update existing type
      this->Internal->AttributeTypes[index] = type;
      }
    }

  settings.endArray();
}

//-----------------------------------------------------------------------------
void vpAttributeConfig::writeAttributeTypes()
{
  this->Internal->Settings.remove("");
  this->Internal->Settings.beginWriteArray(
    "AttributeTypes", this->getNumberOfTypes());

  for (int i = 0, end = this->Internal->AttributeTypes.size(); i < end; ++i)
    {
    const vpAttributeType& type = this->Internal->AttributeTypes[i];

    this->Internal->Settings.setArrayIndex(i);
    this->Internal->Settings.setValue("Id", type.Id);
    this->Internal->Settings.setValue("Name", type.Name);
    }

  this->Internal->Settings.endArray();
}

//-----------------------------------------------------------------------------
int vpAttributeConfig::getNumberOfTypes()
{
  return this->Internal->AttributeTypes.size();
}

//-----------------------------------------------------------------------------
int vpAttributeConfig::getAttributeTypeIndex(int id)
{
  return this->Internal->IdToTypeMap.value(id, -1);
}

//-----------------------------------------------------------------------------
vpAttributeConfig::vpAttributeType vpAttributeConfig::getAttributeTypeByIndex(
  int index)
{
  if (index < this->Internal->AttributeTypes.size())
    {
    return this->Internal->AttributeTypes[index];
    }
  else
    {
    return vpAttributeType();
    }
}

//-----------------------------------------------------------------------------
vpAttributeConfig::vpAttributeType vpAttributeConfig::getAttributeTypeByName(
  const QString& name)

{
  int count = this->getNumberOfTypes();
  for (int i = 0; i < count; ++i)
    {
    if (this->Internal->AttributeTypes[i].Name.compare(
          name, Qt::CaseSensitive) == 0)
      {
      return this->Internal->AttributeTypes[i];
      }
    }
  return vpAttributeType();
}

//-----------------------------------------------------------------------------
void vpAttributeConfig::loadFromFile(const QString& filename)
{
  QSettings settings(filename, QSettings::IniFormat);
  settings.beginGroup("AttributeTypes");
  this->readAttributeTypes(settings);
  this->writeAttributeTypes();
}
