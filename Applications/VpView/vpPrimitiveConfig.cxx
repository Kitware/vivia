// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpPrimitiveConfig.h"

#include "vpConfigUtils.h"

#include <QColor>
#include <QSettings>
#include <QtDebug>

#include <assert.h>
#include <vector>

//-----------------------------------------------------------------------------
class vpPrimitiveConfig::vpPrimitiveConfigInternal
{
public:
  vpPrimitiveConfigInternal()
    : Dirty(false)
    {
    }

public:
  QSettings Settings;
  QVector<vpPrimitiveConfig::vpPrimitiveType> PrimitiveTypes;
  QHash<int, int> IdToTypeMap;
  bool Dirty;
};

//-----------------------------------------------------------------------------
vpPrimitiveConfig::vpPrimitiveConfig()
{
  this->Internal = new vpPrimitiveConfigInternal;
  this->Internal->Settings.beginGroup("PrimitiveTypes");
  this->readPrimitiveTypes(this->Internal->Settings);
  this->writePrimitiveTypes();
}

//-----------------------------------------------------------------------------
vpPrimitiveConfig::~vpPrimitiveConfig()
{
  // Write out any config changes made by the user.
  if (this->Internal->Dirty)
    {
    this->writePrimitiveTypes();
    }

  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vpPrimitiveConfig::readPrimitiveTypes(QSettings& settings)
{
  int size = settings.beginReadArray("PrimitiveTypes");

  int prevTypeWithDistanceParam = -1;
  int prevTypeWithTimeParam = -1;

  for (int i = 0; i < size; ++i)
    {
    settings.setArrayIndex(i);

    vpPrimitiveType type;
    type.Id = settings.value("Id").toInt();
    type.Directed = settings.value("Directed").toBool();
    type.Name = settings.value("Name").toString();
    type.Color = vpConfigUtils::ReadColor("Color", settings);

    QVariant paramType = settings.value("ParamType");
    if (paramType.isValid())
      {
      QString paramString = paramType.toString();
      if (paramString == this->getParamTypeString(PPT_Distance))
        {
        type.ParamType = PPT_Distance;
        }
      else if (paramString == this->getParamTypeString(PPT_Time))
        {
        type.ParamType = PPT_Time;
        }
      else if (!paramString.isEmpty())
        {
        qWarning() << "Unrecognized primitive parameter type:" << paramString;
        qWarning() << "Valid types:" << this->getParamTypeString(PPT_Distance)
                   << ',' << this->getParamTypeString(PPT_Time);
        }
      }
    type.ParamDefaultValue = settings.value("ParamDefaultValue", 0.0).toDouble();

    int index = this->getPrimitiveTypeIndex(type.Id);
    if (index == -1)
      {
      // Add new type
      index = this->Internal->PrimitiveTypes.size();
      this->Internal->PrimitiveTypes.push_back(type);
      this->Internal->IdToTypeMap.insert(type.Id, index);
      }
    else
      {
      // Update existing type
      this->Internal->PrimitiveTypes[index] = type;
      }

    // Check ordering of default param value and warn if it doesn't look right
    const vpPrimitiveType* prevType = 0;
    if (type.ParamType == PPT_Distance && prevTypeWithDistanceParam != -1)
      {
      prevType = &this->Internal->PrimitiveTypes[prevTypeWithDistanceParam];
      }
    else if (type.ParamType == PPT_Time && prevTypeWithTimeParam != -1)
      {
      prevType = &this->Internal->PrimitiveTypes[prevTypeWithTimeParam];
      }

    if (prevType && type.ParamDefaultValue < prevType->ParamDefaultValue)
      {
      qWarning() << "Warning: default parameter value for primitive type"
                 << type.Name << '(' << type.ParamDefaultValue << ')'
                 << "is less than that of the previously defined "
                    "primitive type with the same parameter type"
                 << prevType->Name << '(' << prevType->ParamDefaultValue << ')';
      }

    if (type.ParamType == PPT_Distance)
      {
      prevTypeWithDistanceParam = index;
      }
    else if (type.ParamType == PPT_Time)
      {
      prevTypeWithTimeParam = index;
      }
    }

  settings.endArray();
}

//-----------------------------------------------------------------------------
void vpPrimitiveConfig::writePrimitiveTypes()
{
  this->Internal->Settings.remove("");
  this->Internal->Settings.beginWriteArray(
    "PrimitiveTypes", this->getNumberOfTypes());

  for (int i = 0, end = this->Internal->PrimitiveTypes.size(); i < end; ++i)
    {
    const vpPrimitiveType& type = this->Internal->PrimitiveTypes[i];

    this->Internal->Settings.setArrayIndex(i);
    this->Internal->Settings.setValue("Id", type.Id);
    this->Internal->Settings.setValue("Directed", type.Directed);
    this->Internal->Settings.setValue("Name", type.Name);
    vpConfigUtils::WriteColor("Color", type.Color, this->Internal->Settings);
    if (type.ParamType != PPT_None)
      {
      this->Internal->Settings.setValue("ParamType",
                                        this->getParamTypeString(
                                          type.ParamType));
      this->Internal->Settings.setValue("ParamDefaultValue",
                                        type.ParamDefaultValue);
      }
    }

  this->Internal->Settings.endArray();
}

//-----------------------------------------------------------------------------
const char* vpPrimitiveConfig::getParamTypeString(PrimitiveParamType type)
{
  switch (type)
    {
    case PPT_None:     return "";
    case PPT_Distance: return "Distance";
    case PPT_Time:     return "Time";
    }
  return "";
}

//-----------------------------------------------------------------------------
int vpPrimitiveConfig::getNumberOfTypes()
{
  return this->Internal->PrimitiveTypes.size();
}

//-----------------------------------------------------------------------------
int vpPrimitiveConfig::getPrimitiveTypeIndex(int id)
{
  return this->Internal->IdToTypeMap.value(id, -1);
}

//-----------------------------------------------------------------------------
vpPrimitiveConfig::vpPrimitiveType vpPrimitiveConfig::getPrimitiveTypeByIndex(
  int index)
{
  if (index < this->Internal->PrimitiveTypes.size())
    {
    return this->Internal->PrimitiveTypes[index];
    }
  else
    {
    return vpPrimitiveType();
    }
}

//-----------------------------------------------------------------------------
vpPrimitiveConfig::vpPrimitiveType vpPrimitiveConfig::getPrimitiveTypeByName(
  const QString& name)

{
  int count = this->getNumberOfTypes();
  for (int i = 0; i < count; ++i)
    {
    if (this->Internal->PrimitiveTypes[i].Name.compare(
          name, Qt::CaseSensitive) == 0)
      {
      return this->Internal->PrimitiveTypes[i];
      }
    }

  return vpPrimitiveConfig::vpPrimitiveType();
}

//-----------------------------------------------------------------------------
int vpPrimitiveConfig::getPreviousOrderedTypeIndex(int index)
{
  if (index >= this->Internal->PrimitiveTypes.size())
    {
    return -1;
    }

  const vpPrimitiveType& pt = this->Internal->PrimitiveTypes[index];
  if (pt.ParamType == PPT_None)
    {
    return -1;
    }

  while (--index >= 0)
    {
    const vpPrimitiveType& prev = this->Internal->PrimitiveTypes[index];
    if (prev.ParamType == pt.ParamType)
      {
      return index;
      }
    }

  return -1;
}

//-----------------------------------------------------------------------------
int vpPrimitiveConfig::getNextOrderedTypeIndex(int index)
{
  if (index >= this->Internal->PrimitiveTypes.size())
    {
    return -1;
    }

  const vpPrimitiveType& pt = this->Internal->PrimitiveTypes[index];
  if (pt.ParamType == PPT_None)
    {
    return -1;
    }

  while (++index < this->Internal->PrimitiveTypes.size())
    {
    const vpPrimitiveType& next = this->Internal->PrimitiveTypes[index];
    if (next.ParamType == pt.ParamType)
      {
      return index;
      }
    }

  return -1;
}

//-----------------------------------------------------------------------------
void vpPrimitiveConfig::loadFromFile(const QString& filename)
{
  QSettings settings(filename, QSettings::IniFormat);
  settings.beginGroup("PrimitiveTypes");
  this->readPrimitiveTypes(settings);
  this->writePrimitiveTypes();
}
