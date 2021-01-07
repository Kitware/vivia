// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvScoreGradientSetting.h"

#include <QSet>
#include <QSettings>
#include <QStringList>

#include <qtMath.h>
#include <qtScopedSettingsGroup.h>

#include <vgColor.h>

#include "vvScoreGradient.h"

//-----------------------------------------------------------------------------
qtSettings::Scope vvScoreGradientSetting::scope() const
{
  return qtSettings::UserScope | qtSettings::ApplicationScope;
}

//-----------------------------------------------------------------------------
QString vvScoreGradientSetting::key() const
{
  return "ScoreColors";
}

//-----------------------------------------------------------------------------
void vvScoreGradientSetting::initialize(const QSettings& store)
{
  QSet<QString> stopIds;
  QList<vvScoreGradient::Stop> stops;

  // Find entries in our group (can't use beginGroup on const QSettings)
  const QString tag = this->key() + '/';
  foreach (QString key, store.allKeys())
    {
    // Test for membership in our group
    if (key.startsWith(tag) && key.count('/') == 2)
      {
      // Extract stop key
      const QString sid = key.left(key.lastIndexOf('/') + 1);

      // Read stop
      if (!stopIds.contains(sid))
        {
        vvScoreGradient::Stop stop;

        stop.text = store.value(sid + "Name").toString();
        stop.threshold = store.value(sid + "Threshold", -1).toReal();
        const vgColor noColor(Qt::transparent);
        stop.color = vgColor::read(store, sid + "Color", noColor).toQColor();

        // Check if stop is valid
        if (!stop.text.isEmpty() && stop.color != Qt::transparent &&
            qIsFinite(stop.threshold) && stop.threshold >= 0.0)
          {
          // Stop is valid; add to list
          stops.append(stop);
          }

        // Don't try to read this stop again
        stopIds.insert(sid);
        }
      }
    }

  // Generate gradient (initialized to default in case we didn't read anything)
  vvScoreGradient gradient;
  if (!stops.empty())
    {
    // At least one valid stop was read from the store; replace default
    // gradient with one generated from the read stops
    gradient.setStops(stops);
    }

  // Finish initialization
  this->originalValue = QVariant::fromValue(gradient);
  qtAbstractSetting::initialize(store);
}

//-----------------------------------------------------------------------------
bool vvScoreGradientSetting::isModified()
{
  if (this->currentValue.canConvert<vvScoreGradient>() &&
      this->originalValue.canConvert<vvScoreGradient>())
    {
    const vvScoreGradient original =
      this->originalValue.value<vvScoreGradient>();
    const vvScoreGradient current =
      this->currentValue.value<vvScoreGradient>();
    return original != current;
    }

  return qtAbstractSetting::isModified();
}

//-----------------------------------------------------------------------------
void vvScoreGradientSetting::commit(QSettings& store)
{
  vvScoreGradient gradient = this->currentValue.value<vvScoreGradient>();

  // Erase old value
  store.remove(this->key());
  with_expr (qtScopedSettingsGroup{store, this->key()})
    {
    // Write new stops
    int counter = 0;
    foreach (const vvScoreGradient::Stop& stop, gradient.stops())
      {
      with_expr (qtScopedSettingsGroup{store, QString::number(++counter)});
        {
        store.setValue("Name", stop.text);
        store.setValue("Threshold", stop.threshold);
        vgColor(stop.color).write(store, "Color");
        }
      }
    }

  // Mark as committed
  this->originalValue = this->currentValue;
  this->modified = false;
}
