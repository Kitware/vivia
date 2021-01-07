// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
