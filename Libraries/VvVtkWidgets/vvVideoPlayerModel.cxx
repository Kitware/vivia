// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvVideoPlayerModel.h"

// VG includes.
#include "vtkVgTrackModel.h"
#include "vtkVgEventModel.h"
#include "vtkVgVideoProviderBase.h"

// VTK includes.
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>

vtkStandardNewMacro(vvVideoPlayerModel);

//-----------------------------------------------------------------------------
vvVideoPlayerModel::vvVideoPlayerModel() : QObject(), vtkVgVideoModel0(),
  SharingSource(0)
{
}

//-----------------------------------------------------------------------------
vvVideoPlayerModel::~vvVideoPlayerModel()
{
}

//-----------------------------------------------------------------------------
int vvVideoPlayerModel::Update(const vtkVgTimeStamp& timeStamp,
                               const vtkVgTimeStamp* referenceFrameTimeStamp/*=0*/)
{
  int retVal = VTK_ERROR;

  if (!this->SharingSource)
    {
    retVal = this->Superclass::Update(timeStamp, referenceFrameTimeStamp);
    }
  return retVal;
}

//-----------------------------------------------------------------------------
void vvVideoPlayerModel::OnFrameAvailable()
{
  this->UseSourceTimeStamp = true;

  int retVal = this->VideoSource->GetCurrentFrame(this->VideoFrameData);

  // Ignore the matrix.
  this->VideoFrameData->VideoMatrix = 0;

  if (retVal == VTK_OK)
    {
    this->UpdateDataRequestTime.Modified();

    this->InvokeEvent(vtkCommand::UpdateDataEvent);

    if (this->TrackModel)
      {
      this->TrackModel->Update(this->VideoFrameData->TimeStamp);
      }

    if (this->EventModel)
      {
      this->EventModel->Update(this->VideoFrameData->TimeStamp);
      }

    emit this->FrameAvailable();
    }
}

//-----------------------------------------------------------------------------
void vvVideoPlayerModel::PrintSelf(ostream& vtkNotUsed(os),
                                   vtkIndent vtkNotUsed(indent))
{
  // \TODO: Implement this.
}
