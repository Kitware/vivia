// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpEntityConfig_h
#define __vpEntityConfig_h

class vgEntityType;

class vpEntityConfig
{
public:
  virtual ~vpEntityConfig() {}

  virtual int GetNumberOfTypes() = 0;

  virtual const vgEntityType& GetEntityType(int id) = 0;
  virtual void SetEntityType(int id, const vgEntityType& type) = 0;

  virtual void MarkTypeUsed(int index) = 0;
  virtual void MarkAllTypesUnused() = 0;
};

#endif // __vpEntityConfig_h
