/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgColor_h
#define __vgColor_h

#include <vgExport.h>

class QColor;
class QSettings;
class QString;

class QTVG_COMMON_EXPORT vgColor
{
public:
  typedef union
    {
    double array[4];
    struct { double red, green, blue, alpha; } color;
    } ComponentData;

  vgColor();
  vgColor(const vgColor&);
  vgColor(const QColor&);

  explicit vgColor(int red, int green, int blue, int alpha = 255);
  explicit vgColor(double red, double green, double blue, double alpha = 1.0);

  vgColor& operator=(const vgColor&);
  vgColor& operator=(const QColor&);

  ComponentData& data();
  const ComponentData& constData() const;

  ComponentData value() const;

  void fillArray(double (&)[3]) const;
  void fillArray(double (&)[4]) const;
  void fillArray(unsigned char (&)[3]) const;
  void fillArray(unsigned char (&)[4]) const;

  static void fillArray(const QColor&, double (&)[3]);
  static void fillArray(const QColor&, double (&)[4]);
  static void fillArray(const QColor&, unsigned char (&)[3]);
  static void fillArray(const QColor&, unsigned char (&)[4]);

  enum StringType
    {
    NoAlpha,
    WithAlpha,
    AutoAlpha
    };

  QString toString(StringType = AutoAlpha) const;
  QColor toQColor() const;

  static vgColor read(const QSettings&, const QString& key,
                      vgColor defaultValue);
  static void write(QSettings&, const QString& key, const vgColor& value);

  bool read(const QSettings&, const QString& key);
  void write(QSettings&, const QString& key) const;

protected:
  ComponentData d;
};

#endif
