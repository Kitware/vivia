/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpConfigUtils.h"

#include <QColor>
#include <QSettings>

namespace
{

//-----------------------------------------------------------------------------
QString ColorString(const QColor& color)
{
  return QString("%1,%2,%3").arg(color.red())
                            .arg(color.green())
                            .arg(color.blue());
}

} // end anonymous namespace

namespace vpConfigUtils
{

//-----------------------------------------------------------------------------
QColor ReadColor(const QString& key, QSettings& settings)
{
  QColor color;
  QVariant val = settings.value(key);
  if (!val.isValid())
    {
    return color;
    }

  QString vs = val.toString();
  QStringList vsp = vs.split(",");
  if (vsp.count() == 3)
    {
    color.setRed(vsp[0].toInt());
    color.setGreen(vsp[1].toInt());
    color.setBlue(vsp[2].toInt());
    }

  return color;
}

//-----------------------------------------------------------------------------
void WriteColor(const QString& key, const QColor& color, QSettings& settings)
{
  if (color.isValid())
    {
    settings.setValue(key, ColorString(color));
    }
}

} // end vpConfigUtils
