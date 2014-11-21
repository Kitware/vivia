/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvDescriptorStyleSetting.h"

#include <QSettings>
#include <QStringList>

#include <qtScopedSettingGroup.h>

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

  qtScopedSettingGroup g(store, this->key());
  foreach_iter (QVariantHash::iterator, iter, map)
    {
    store.setValue(iter.key(), iter.value());
    }

  this->originalValue = this->currentValue;
  this->modified = false;
}

} // namespace vvDescriptorStyle
