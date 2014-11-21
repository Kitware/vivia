/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
