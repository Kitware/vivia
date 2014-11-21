/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgVideoModel.h"

#include <algorithm>

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkVgVideoModel);

//-----------------------------------------------------------------------------
vtkVgVideoModel::vtkVgVideoModel() :
  VideoSource(0)
{
}

//-----------------------------------------------------------------------------
vtkVgVideoModel::~vtkVgVideoModel()
{
}

//-----------------------------------------------------------------------------
void vtkVgVideoModel::PrintSelf(ostream& os, vtkIndent indent)
{
  // TODO
  vtkVgModelBase::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
vtkVgVideoSourceBase* vtkVgVideoModel::GetVideoSource()
{
  return this->VideoSource;
}

//-----------------------------------------------------------------------------
const vtkVgVideoSourceBase* vtkVgVideoModel::GetVideoSource() const
{
  return this->VideoSource;
}

//-----------------------------------------------------------------------------
void vtkVgVideoModel::SetVideoSource(vtkVgVideoSourceBase* videoSource)
{
  if (videoSource == this->VideoSource)
    {
    // Do nothing if already using specified source
    return;
    }

  this->VideoSource = videoSource;
  this->CurrentVideoFrame = vtkVgVideoFrame();

  this->UpdateChildren(vtkVgTimeStamp());
  this->Modified();
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel::Update(const vtkVgTimeStamp& timeStamp,
                            const vtkVgTimeStamp* referenceFrameTimeStamp)
{
  if (!this->VideoSource)
    {
    // Can't update without video source
    return VTK_ERROR;
    }

  vtkVgTimeStamp newTime = this->VideoSource->ResolveSeek(timeStamp);
  if (this->CurrentTimeStamp == newTime)
    {
    // If frame time has not changed, do nothing
    return VTK_OK;
    }

  this->CurrentVideoFrame = this->VideoSource->GetFrame(timeStamp);
  this->CurrentTimeStamp = this->CurrentVideoFrame.MetaData.Time;

  this->UpdateChildren(this->CurrentTimeStamp, referenceFrameTimeStamp);
  this->Modified();

  return VTK_OK;
}

//-----------------------------------------------------------------------------
const vtkVgVideoFrame& vtkVgVideoModel::GetCurrentVideoFrame() const
{
  return this->CurrentVideoFrame;
}

//-----------------------------------------------------------------------------
unsigned long vtkVgVideoModel::GetUpdateTime()
{
  return this->UpdateTime.GetMTime();
}

//-----------------------------------------------------------------------------
void vtkVgVideoModel::AddChildModel(vtkVgModelBase* model)
{
  if (model && !this->HasChildModel(model))
    {
    this->ChildModels.push_back(model);
    model->Register(this);
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
bool vtkVgVideoModel::HasChildModel(vtkVgModelBase* model)
{
  const size_t count =
   std::count(this->ChildModels.begin(), this->ChildModels.end(), model);
  return count > 0;
}

//-----------------------------------------------------------------------------
size_t vtkVgVideoModel::GetChildModelCount() const
{
  return this->ChildModels.size();
}

//-----------------------------------------------------------------------------
vtkVgModelBase* vtkVgVideoModel::GetChildModel(size_t index) const
{
  if (index >= this->ChildModels.size())
    {
    // Index out of bounds
    return 0;
    }

  return this->ChildModels[index];
}

//-----------------------------------------------------------------------------
bool vtkVgVideoModel::RemoveChildModel(vtkVgModelBase* model)
{
  typedef std::vector<vtkVgModelBase*>::iterator Iterator;
  for (Iterator i = this->ChildModels.begin(), end = this->ChildModels.end();
       i != end; ++i)
    {
    if (*i == model)
      {
      (*i)->UnRegister(this);
      this->ChildModels.erase(i);
      return true;
      }
    }

  // Not found
  return false;
}

//-----------------------------------------------------------------------------
bool vtkVgVideoModel::RemoveChildModel(size_t index)
{
  if (index >= this->ChildModels.size())
    {
    // Index out of bounds
    return false;
    }

  std::vector<vtkVgModelBase*>::iterator i = this->ChildModels.begin() + index;
  (*i)->UnRegister(this);
  this->ChildModels.erase(i);

  return true;
}

//-----------------------------------------------------------------------------
void vtkVgVideoModel::UpdateChildren(
  const vtkVgTimeStamp& timeStamp,
  const vtkVgTimeStamp* referenceFrameTimeStamp)
{
  for (size_t n = 0, k = this->ChildModels.size(); n < k; ++n)
    {
    this->ChildModels[n]->Update(timeStamp, referenceFrameTimeStamp);
    }
}
