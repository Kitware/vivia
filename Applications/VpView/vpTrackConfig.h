// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpTrackConfig_h
#define __vpTrackConfig_h

#include "vpEntityConfig.h"

class QSettings;

class vgTrackType;

class vtkVgTrackTypeRegistry;

class vpTrackConfig : public vpEntityConfig
{
public:
  vpTrackConfig(vtkVgTrackTypeRegistry* registry);
  virtual ~vpTrackConfig();

  virtual int GetNumberOfTypes();

  virtual const vgEntityType& GetEntityType(int id);
  virtual void SetEntityType(int id, const vgEntityType& type);

  virtual void MarkTypeUsed(int index);
  virtual void MarkAllTypesUnused();

  void AddType(const vgTrackType& tt);
  void SetType(int index, const vgTrackType& tt);

  int GetTrackTypeIndex(const char* id);
  const vgTrackType& GetTrackTypeByIndex(int index);

  void LoadFromFile(const char* filename);

protected:
  void ReadTrackTypes(QSettings& settings);
  void WriteTrackTypes();

private:
  class vpTrackConfigInternal;
  vpTrackConfigInternal* Internal;

  vtkVgTrackTypeRegistry* Registry;

private:
  vpTrackConfig(const vpTrackConfig& src);   // Not implemented.
  void operator=(const vpTrackConfig& src);  // Not implemented.
};

#endif // __vpTrackConfig_h
