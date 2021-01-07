// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpPrimitiveConfig_h
#define __vpPrimitiveConfig_h

class QSettings;

// Qt includes
#include <QColor>
#include <QString>

class vpPrimitiveConfig
{
public:
  enum PrimitiveParamType
    {
    PPT_None,
    PPT_Distance,
    PPT_Time
    };

  struct vpPrimitiveType
    {
    vpPrimitiveType() :
      Id(-1),
      Directed(false),
      ParamType(PPT_None),
      ParamDefaultValue(0.0)
      {
      }

    int Id;
    bool Directed;
    QString Name;
    QColor Color;
    PrimitiveParamType ParamType;
    double ParamDefaultValue;
    };

  vpPrimitiveConfig();
  virtual ~vpPrimitiveConfig();

  virtual int getNumberOfTypes();

  int getPrimitiveTypeIndex(int id);
  vpPrimitiveType getPrimitiveTypeByIndex(int index);
  vpPrimitiveType getPrimitiveTypeByName(const QString& name);

  int getPreviousOrderedTypeIndex(int index);
  int getNextOrderedTypeIndex(int index);

  void loadFromFile(const QString& filename);

protected:
  void readPrimitiveTypes(QSettings& settings);
  void writePrimitiveTypes();

  const char* getParamTypeString(PrimitiveParamType type);

private:
  class vpPrimitiveConfigInternal;
  vpPrimitiveConfigInternal* Internal;

private:
  /// Not implemented
  vpPrimitiveConfig(const vpPrimitiveConfig& src);

  /// Not implemented
  void operator=(const vpPrimitiveConfig& src);
};

#endif // __vpPrimitiveConfig_h
