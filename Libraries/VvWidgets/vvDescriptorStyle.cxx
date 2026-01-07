// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvDescriptorStyle.h"

#include <QApplication>
#include <QStringList>

#include <qtDebugHelper.h>
#include <qtGlobal.h>
#include <qtSettings.h>
#include <qtStlUtil.h>

#include <vvDescriptor.h>

#include "vvDescriptorStyleSetting.h"

Q_DECLARE_METATYPE(vvDescriptorStyle::Map)

namespace vvDescriptorStyle
{

//BEGIN vvDescriptorStyle data members

const char* const settingKey   = "Settings";
const char* const propertyKey  = "DescriptorStyleMap";

//END vvDescriptorStyle data members

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvDescriptorStyle::SettingWrapper

//-----------------------------------------------------------------------------
class SettingWrapper : public qtSettings
{
public:
  SettingWrapper();
  qtSettings_declare(QVariant, map, setMap);
};

//-----------------------------------------------------------------------------
SettingWrapper::SettingWrapper()
{
  this->declareSetting(settingKey, new vvDescriptorStyle::Setting);
}

//-----------------------------------------------------------------------------
QVariant SettingWrapper::map() const
{
  return this->value(settingKey);
}

//-----------------------------------------------------------------------------
void SettingWrapper::setMap(QVariant value)
{
  this->setValue(settingKey, value);
}

//END vvDescriptorStyle::Settings

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvDescriptorStyle::Map

//-----------------------------------------------------------------------------
Map::Map()
{
}

//-----------------------------------------------------------------------------
Map::Map(const vvDescriptorStyle::Map& other) :
  QHash<QString, vvDescriptorStyle::Styles>(other)
{
}

//-----------------------------------------------------------------------------
Map::Map(QHash<QString, Styles> const& other) :
  QHash<QString, vvDescriptorStyle::Styles>(other)
{
}

//-----------------------------------------------------------------------------
Map::Map(const QVariantHash& other)
{
  *this = other;
}

//-----------------------------------------------------------------------------
Map& Map::operator=(const vvDescriptorStyle::Map& other)
{
  this->QHash<QString, Styles>::operator=(other);
  return *this;
}

//-----------------------------------------------------------------------------
Map& Map::operator=(QHash<QString, Styles> const& other)
{
  this->QHash<QString, Styles>::operator=(other);
  return *this;
}

//-----------------------------------------------------------------------------
Map& Map::operator=(const QVariantHash& other)
{
  foreach_iter (QVariantHash::const_iterator, iter, other)
    {
    vvDescriptorStyle::Styles s = vvDescriptorStyle::None;
    if (iter->canConvert<vvDescriptorStyle::Styles>())
      {
      s = iter->value<vvDescriptorStyle::Styles>();
      }
    else if (iter->canConvert<int>())
      {
      s = static_cast<vvDescriptorStyle::Styles>(iter->toInt());
      }
    this->insert(iter.key(), s);
    }

  return *this;
}

//-----------------------------------------------------------------------------
QVariantHash Map::serializable() const
{
  QVariantHash out;

  foreach_iter (Map::const_iterator, iter, *this)
    {
    out.insert(iter.key(), QVariant::fromValue<int>(iter.value()));
    }
  return out;
}

//END vvDescriptorStyle::Map

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvDescriptorStyle member functions

//-----------------------------------------------------------------------------
QString string(StyleFlag style)
{
  switch (style)
    {
    case None:
      return "(None)";
    case Articulation:
      return "Articulation";
    case Trajectory:
      return "Trajectory";
    case Appearance:
      return "Appearance";
    case Color:
      return "Color";
    case Metadata:
      return "Metadata";
    default:
      return QString("<Unknown (%1)>").arg(style);
    }
}

//-----------------------------------------------------------------------------
QString string(Styles styles)
{
  QStringList flagTexts;

  int flag = 1;
  int s = styles;
  while (s)
    {
    if (s & 1)
      {
      StyleFlag style = static_cast<StyleFlag>(flag);
      flagTexts.append(string(style));
      }
    s >>= 1;
    flag <<= 1;
    }

  return (flagTexts.isEmpty() ? "(None)" : flagTexts.join(", "));
}

//-----------------------------------------------------------------------------
Styles styles(const vvDescriptor& descriptor, bool* known)
{
  return styles(descriptor, map(), known);
}

//-----------------------------------------------------------------------------
Styles styles(const vvDescriptor& descriptor, const Map& map, bool* known)
{
  QString key = qtString(descriptor.ModuleName);
  if (map.contains(key))
    {
    known && (*known = true);
    return map[key];
    }

  known && (*known = false);
  return None;
}

//-----------------------------------------------------------------------------
QString styleString(const vvDescriptor& descriptor)
{
  return styleString(descriptor, map());
}

//-----------------------------------------------------------------------------
QString styleString(const vvDescriptor& descriptor, const Map& map)
{
  bool known = false;
  Styles s = styles(descriptor, map, &known);
  return (known ? string(s) : "(Unknown)");
}

//-----------------------------------------------------------------------------
Map map(Scope scope)
{
  // Get value from process scope (QApplication property)
  if (scope.testFlag(Process))
    {
    QVariant data = qApp->property(propertyKey);
    if (!data.isNull() && data.canConvert<Map>())
      {
      return data.value<Map>();
      }
    }

  // Get value from settings scope (qtSettings)
  if (scope.testFlag(Settings))
    {
    SettingWrapper settings;
    QVariant data = settings.map();
    if (!data.isNull() && data.canConvert(QVariant::Hash))
      {
      return Map(data.toHash());
      }
    }

  return Map();
}

//-----------------------------------------------------------------------------
QDEBUG_ENUM_HANDLER_BEGIN(vvDescriptorStyle::ScopeItem)
QDEBUG_HANDLE_ENUM(Process)
QDEBUG_HANDLE_ENUM(Settings)
QDEBUG_HANDLE_ENUM(Any)
QDEBUG_ENUM_HANDLER_END(vvDescriptorStyle)

//-----------------------------------------------------------------------------
void setMap(const vvDescriptorStyle::Map& newMap, ScopeItem where)
{
  switch (where)
    {
    case Process:
      qApp->setProperty(propertyKey, QVariant::fromValue<Map>(newMap));
      break;

    case Settings:
      {
      SettingWrapper settings;
      settings.setMap(newMap.serializable());
      settings.commit();
      break;
      }

    default:
      qWarning() << "vvDescriptorStyle::setMap: invalid operation:"
                 << "cannot set map on scope" << where;
    }
}

//END vvDescriptorStyle member functions

} // namespace vvDescriptorStyle
