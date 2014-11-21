/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpImageSourceFactory_h
#define __vpImageSourceFactory_h

#include <vgAbstractFactory.h>

// C++ includes
#include <string>

#include <vtkVgBaseImageSource.h>

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

//-----------------------------------------------------------------------------
struct RegisterVpImageSource
{
  typedef vtkVgBaseImageSource* (*Creator)();

  RegisterVpImageSource(Creator function)
    {
    vpImageSourceFactory::GetInstance()->Register(function);
    }
};

#endif // __vpImageSourceFactory_h
