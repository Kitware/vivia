/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpAttributeConfig_h
#define __vpAttributeConfig_h

class QSettings;

// Qt includes
#include <QColor>
#include <QString>

class vpAttributeConfig
{
public:
  struct vpAttributeType
    {
    vpAttributeType()
      : Id(-1)
      {
      }

    int Id;
    QString Name;
    };

  vpAttributeConfig();
  virtual ~vpAttributeConfig();

  virtual int getNumberOfTypes();

  int getAttributeTypeIndex(int id);
  vpAttributeType getAttributeTypeByIndex(int index);
  vpAttributeType getAttributeTypeByName(const QString& name);

  void loadFromFile(const QString& filename);

protected:
  void readAttributeTypes(QSettings& settings);
  void writeAttributeTypes();

private:
  class vpAttributeConfigInternal;
  vpAttributeConfigInternal* Internal;

private:
  /// Not implemented
  vpAttributeConfig(const vpAttributeConfig& src);

  /// Not implemented
  void operator=(const vpAttributeConfig& src);
};

#endif // __vpAttributeConfig_h
