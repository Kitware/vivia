// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpImageSourceFactory.h"

vpImageSourceFactory* vpImageSourceFactory::Instance = 0;

//-----------------------------------------------------------------------------
vpImageSourceFactory* vpImageSourceFactory::GetInstance()
{
  if (Instance == 0)
    {
    Instance = new vpImageSourceFactory();
    }

  return Instance;
}

//-----------------------------------------------------------------------------
void vpImageSourceFactory::OnApplicationExit()
{
  CleanUp();
}

//-----------------------------------------------------------------------------
vtkVgBaseImageSource* vpImageSourceFactory::Create(std::string source)
{
  for (size_t i = 0; i < this->Creators.size(); ++i)
    {
    vtkVgBaseImageSource* imageSourceInstance = this->Creators[i]();
    if (imageSourceInstance->CanRead(source))
      {
      return imageSourceInstance;
      }
    else
      {
      imageSourceInstance->Delete();
      }
    }

  return 0;
}

//-----------------------------------------------------------------------------
void vpImageSourceFactory::CleanUp()
{
  if (Instance)
    {
    delete Instance;
    Instance = 0;
    }
}
