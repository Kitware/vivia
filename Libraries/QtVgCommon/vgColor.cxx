/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgColor.h"

#include <qtMath.h>

#include <QColor>
#include <QSettings>
#include <QString>

namespace
{

//-----------------------------------------------------------------------------
template <typename T>
QVariantList toVariants(const QList<T>& in)
{
  QVariantList out;
  out.reserve(in.size());
  foreach (const auto& item, in)
    {
    out.append(QVariant::fromValue(item));
    }

  return out;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vgColor::vgColor()
{
  this->d.color.red   = qQNaN();
  this->d.color.green = qQNaN();
  this->d.color.blue  = qQNaN();
  this->d.color.alpha = qQNaN();
}

//-----------------------------------------------------------------------------
vgColor::vgColor(const vgColor& other)
{
  *this = other;
}

//-----------------------------------------------------------------------------
vgColor::vgColor(const QColor& qc)
{
  *this = qc;
}

//-----------------------------------------------------------------------------
vgColor::vgColor(int red, int green, int blue, int alpha)
{
  *this = QColor(red, green, blue, alpha);
}

//-----------------------------------------------------------------------------
vgColor::vgColor(double red, double green, double blue, double alpha)
{
  this->d.color.red   = red;
  this->d.color.green = green;
  this->d.color.blue  = blue;
  this->d.color.alpha = alpha;
}

//-----------------------------------------------------------------------------
vgColor::vgColor(const double (&color)[3], double alpha)
{
  this->d.color.red   = color[0];
  this->d.color.green = color[1];
  this->d.color.blue  = color[2];
  this->d.color.alpha = alpha;
}

//-----------------------------------------------------------------------------
vgColor::vgColor(const double (&color)[4])
{
  this->d.color.red   = color[0];
  this->d.color.green = color[1];
  this->d.color.blue  = color[2];
  this->d.color.alpha = color[3];
}

//-----------------------------------------------------------------------------
vgColor& vgColor::operator=(const vgColor& other)
{
  this->d.color.red   = other.d.color.red;
  this->d.color.green = other.d.color.green;
  this->d.color.blue  = other.d.color.blue;
  this->d.color.alpha = other.d.color.alpha;
  return *this;
}

//-----------------------------------------------------------------------------
vgColor& vgColor::operator=(const QColor& qc)
{
  this->d.color.red   = qc.redF();
  this->d.color.green = qc.greenF();
  this->d.color.blue  = qc.blueF();
  this->d.color.alpha = qc.alphaF();
  return *this;
}

//-----------------------------------------------------------------------------
bool vgColor::isValid() const
{
  return (qIsFinite(this->d.array[0]) && qIsFinite(this->d.array[1]) &&
          qIsFinite(this->d.array[2]) && qIsFinite(this->d.array[3]));
}

//-----------------------------------------------------------------------------
vgColor::ComponentData& vgColor::data()
{
  return this->d;
}

//-----------------------------------------------------------------------------
const vgColor::ComponentData& vgColor::constData() const
{
  return this->d;
}

//-----------------------------------------------------------------------------
vgColor::ComponentData vgColor::value() const
{
  return this->d;
}

//-----------------------------------------------------------------------------
void vgColor::fillArray(double (&out)[3]) const
{
  out[0] = this->d.array[0];
  out[1] = this->d.array[1];
  out[2] = this->d.array[2];
}

//-----------------------------------------------------------------------------
void vgColor::fillArray(double (&out)[4]) const
{
  out[0] = this->d.array[0];
  out[1] = this->d.array[1];
  out[2] = this->d.array[2];
  out[3] = this->d.array[3];
}

//-----------------------------------------------------------------------------
void vgColor::fillArray(unsigned char (&out)[3]) const
{
  vgColor::fillArray(this->toQColor(), out);
}

//-----------------------------------------------------------------------------
void vgColor::fillArray(unsigned char (&out)[4]) const
{
  vgColor::fillArray(this->toQColor(), out);
}

//-----------------------------------------------------------------------------
void vgColor::fillArray(const QColor& color, double (&out)[3])
{
  out[0] = color.redF();
  out[1] = color.greenF();
  out[2] = color.blueF();
}

//-----------------------------------------------------------------------------
void vgColor::fillArray(const QColor& color, double (&out)[4])
{
  out[0] = color.redF();
  out[1] = color.greenF();
  out[2] = color.blueF();
  out[3] = color.alphaF();
}

//-----------------------------------------------------------------------------
void vgColor::fillArray(const QColor& color, unsigned char (&out)[3])
{
  out[0] = static_cast<unsigned char>(color.red());
  out[1] = static_cast<unsigned char>(color.green());
  out[2] = static_cast<unsigned char>(color.blue());
}

//-----------------------------------------------------------------------------
void vgColor::fillArray(const QColor& color, unsigned char (&out)[4])
{
  out[0] = static_cast<unsigned char>(color.red());
  out[1] = static_cast<unsigned char>(color.green());
  out[2] = static_cast<unsigned char>(color.blue());
  out[3] = static_cast<unsigned char>(color.alpha());
}

//-----------------------------------------------------------------------------
QList<int> vgColor::toList() const
{
  QList<int> result;

  result.append(qRound(255.0 * this->d.color.red));
  result.append(qRound(255.0 * this->d.color.green));
  result.append(qRound(255.0 * this->d.color.blue));

  if (this->d.color.alpha > 0.0)
    {
    result.append(qRound(255.0 * this->d.color.alpha));
    }

  return result;
}

//-----------------------------------------------------------------------------
QString vgColor::toString(vgColor::StringType t) const
{
  bool haveAlpha = (this->d.color.alpha > 0.0);
  int c[4] =
    {
    qRound(255.0 * this->d.color.red),
    qRound(255.0 * this->d.color.green),
    qRound(255.0 * this->d.color.blue),
    qRound(255.0 * this->d.color.alpha)
    };

  if (t == vgColor::WithAlpha || (t == vgColor::AutoAlpha && haveAlpha))
    {
    QString f("%1,%2,%3,%4");
    return f.arg(c[0]).arg(c[1]).arg(c[2]).arg(c[3]);
    }
  else
    {
    QString f("%1,%2,%3");
    return f.arg(c[0]).arg(c[1]).arg(c[2]);
    }
}

//-----------------------------------------------------------------------------
QColor vgColor::toQColor() const
{
  return QColor::fromRgbF(this->d.color.red,  this->d.color.green,
                          this->d.color.blue, this->d.color.alpha);
}

//-----------------------------------------------------------------------------
vgColor vgColor::read(
  const QSettings& settings, const QString& key, vgColor defaultValue)
{
  defaultValue.read(settings, key);
  return defaultValue;
}

//-----------------------------------------------------------------------------
void vgColor::write(
  QSettings& settings, const QString& key, const vgColor& value)
{
  value.write(settings, key);
}

//-----------------------------------------------------------------------------
bool vgColor::read(const QSettings& settings, const QString& key)
{
  const auto& v = settings.value(key);

  const auto& l = v.toStringList();
  if (l.size() > 1)
    {
    return this->read(l);
    }

  const auto& s = v.toString();
  if (!s.isEmpty())
    {
    return this->read(s.split(','));
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vgColor::read(const QStringList& parts)
{
  int k = parts.count();
  if (k == 3 || k == 4)
    {
    *this = QColor(parts[0].toInt(), parts[1].toInt(), parts[2].toInt(),
                    k == 4 ? parts[3].toInt() : 255);
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
void vgColor::write(QSettings& settings, const QString& key) const
{
  settings.setValue(key, toVariants(this->toList()));
}
