/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsVideoPlayerModel.h"

#include <vtkObjectFactory.h>

#include "vsQfVideoSource.h"

vtkStandardNewMacro(vsVideoPlayerModel);

//-----------------------------------------------------------------------------
vsVideoPlayerModel::vsVideoPlayerModel() : vvVideoPlayerModel()
{
}

//-----------------------------------------------------------------------------
vsVideoPlayerModel::~vsVideoPlayerModel()
{
}

//-----------------------------------------------------------------------------
int vsVideoPlayerModel::Play()
{
  this->Superclass::Play();

  vsQfVideoSource* src = vsQfVideoSource::SafeDownCast(this->GetVideoSource());
  if (src)
    {
    src->Play();
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vsVideoPlayerModel::Pause()
{
  this->Superclass::Pause();

  vsQfVideoSource* src = vsQfVideoSource::SafeDownCast(this->GetVideoSource());
  if (src)
    {
    src->Pause();
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vsVideoPlayerModel::Stop()
{
  this->Superclass::Stop();

  vsQfVideoSource* src = vsQfVideoSource::SafeDownCast(this->GetVideoSource());
  if (src)
    {
    src->Stop();
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vsVideoPlayerModel::PrintSelf(ostream& vtkNotUsed(os),
                                   vtkIndent vtkNotUsed(indent))
{
  // \TODO: Implement this.
}
