// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvDescriptorStyleSetting.h"

#include <QSettings>
#include <QStringList>

#include <qtEnumerate.h>
#include <qtScopedSettingsGroup.h>

#include "vvDescriptorStyle.h"

namespace vvDescriptorStyle
{

//-----------------------------------------------------------------------------
qtSettings::Scope Setting::scope() const
{
  return qtSettings::UserScope | qtSettings::OrganizationScope;
}

//-----------------------------------------------------------------------------
QString Setting::key() const
{
  return "DescriptorStyles";
}

//-----------------------------------------------------------------------------
void Setting::initialize(const QSettings& store)
{
  vvDescriptorStyle::Map map;

  // Find entries in our group (can't use beginGroup on const QSettings)
  const QString tag = this->key() + '/';
  const int tagSize = tag.size() - 1;
  foreach (QString key, store.allKeys())
    {
    // Test for membership in our group
    if (key.startsWith(tag) && key.lastIndexOf('/') == tagSize)
      {
      // Get raw data
      QVariant data = store.value(key);

      // Convert to styles
      vvDescriptorStyle::Styles styles = vvDescriptorStyle::None;
      if (data.canConvert<vvDescriptorStyle::Styles>())
        {
        styles = data.value<vvDescriptorStyle::Styles>();
        }
      else if (data.canConvert<int>())
        {
        styles = static_cast<vvDescriptorStyle::Styles>(data.toInt());
        }

      // Add entry
      map.insert(key.mid(tagSize + 1), styles);
      }
    }

  // Finish initialization
  this->originalValue = map.serializable();
  qtAbstractSetting::initialize(store);
}

//-----------------------------------------------------------------------------
void Setting::commit(QSettings& store)
{
  QVariantHash map = this->currentValue.toHash();

  with_expr (qtScopedSettingsGroup{store, this->key()})
    {
    for (auto iter : qtEnumerate(map))
      {
      store.setValue(iter.key(), iter.value());
      }
    }

  this->originalValue = this->currentValue;
  this->modified = false;
}

} // namespace vvDescriptorStyle
