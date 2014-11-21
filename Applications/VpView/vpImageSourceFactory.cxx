/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
