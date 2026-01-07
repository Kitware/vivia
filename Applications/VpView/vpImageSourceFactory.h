// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
