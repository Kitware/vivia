/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpImageSourceFactory_h
#define __vpImageSourceFactory_h

#include <vgAbstractFactory.h>

#include <vtkVgBaseImageSource.h>

#include <string>

//-----------------------------------------------------------------------------
class vpImageSourceFactory : public vgAbstractFactory<vtkVgBaseImageSource*, std::string>
{
public:
  static vpImageSourceFactory*    GetInstance();
  static void                     OnApplicationExit();

  virtual vtkVgBaseImageSource*   Create(std::string source);

private:
  static void CleanUp();
  static vpImageSourceFactory* Instance;
};

#endif
