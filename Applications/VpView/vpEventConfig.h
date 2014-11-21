/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpEventConfig_h
#define __vpEventConfig_h

#include "vpEntityConfig.h"

class QColor;
class QSettings;
class QString;

class vgEventType;

class vtkVgEventTypeRegistry;

class vpEventConfig : public vpEntityConfig
{
public:
  vpEventConfig(vtkVgEventTypeRegistry* registry);
  virtual ~vpEventConfig();

  virtual int GetNumberOfTypes();

  virtual const vgEntityType& GetEntityType(int id);
  virtual void SetEntityType(int id, const vgEntityType& type);

  virtual void MarkTypeUsed(int index);
  virtual void MarkAllTypesUnused();

  int GetEventTypeIndex(int id);
  const vgEventType& GetEventTypeById(int id);
  const vgEventType& GetEventTypeByIndex(int index);

  static int GetIdFromString(const char* str);
  static const char* GetStringFromId(int id);

  static int GetDisplayModeFromString(const char* str);

  void LoadFromFile(const char* filename);

protected:
  void ReadEventTypes(QSettings& settings);
  void WriteEventTypes();

private:
  class vpEventConfigInternal;
  vpEventConfigInternal* Internal;

  vtkVgEventTypeRegistry* Registry;

private:
  vpEventConfig(const vpEventConfig& src);   // Not implemented.
  void operator=(const vpEventConfig& src);  // Not implemented.
};

#endif // __vpEventConfig_h
