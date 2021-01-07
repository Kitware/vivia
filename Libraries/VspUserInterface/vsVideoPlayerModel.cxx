// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
