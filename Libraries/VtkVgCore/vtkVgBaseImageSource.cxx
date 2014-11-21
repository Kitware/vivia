/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgBaseImageSource.h"

#include <vtkMatrix4x4.h>

//-----------------------------------------------------------------------------
vtkVgBaseImageSource::vtkVgBaseImageSource() : vtkImageAlgorithm(),
  UseOutputResolution(false),
  Level(0),
  Scale(1.0),
  FileName(0)
{
  this->OutputResolution[0] = this->OutputResolution[1] = -1;

  for (int i = 0; i < 4; ++i) {this->ReadExtents[i] = -1;}
  for (int i = 0; i < 3; ++i) {this->Origin[i] = 0; this->Spacing[i] = 1;}

  this->ImageTimeStamp.Reset();

  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------------
vtkVgBaseImageSource::~vtkVgBaseImageSource()
{
  if (this->FileName)
    {
    this->SetFileName(0);
    }
}

//----------------------------------------------------------------------------
void vtkVgBaseImageSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkVgBaseImageSource::GetOutputResolution(int& w, int& h) const
{
  w = this->OutputResolution[0];
  h = this->OutputResolution[1];
}

//----------------------------------------------------------------------------
int vtkVgBaseImageSource::SetOutputResolution(int w, int h)
{
  if (w < 0)
    {
    // Error.
    return VTK_ERROR;
    }

  if (h < 0)
    {
    // Error.
    return VTK_ERROR;
    }

  this->OutputResolution[0] = w;
  this->OutputResolution[1] = h;

  this->Modified();

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vtkVgBaseImageSource::GetNumberOfLevels() const
{
  return 0;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMatrix4x4> vtkVgBaseImageSource::GetImageToWorldMatrix()
{
  return 0;
}

//-----------------------------------------------------------------------------
int vtkVgBaseImageSource::RequestInformation(vtkInformation* request,
                                             vtkInformationVector** inputVector,
                                             vtkInformationVector* outputVector)
{
  return this->Superclass::RequestInformation(request, inputVector, outputVector);
}

//-----------------------------------------------------------------------------
int vtkVgBaseImageSource::RequestData(vtkInformation* request,
                                      vtkInformationVector** inputVector,
                                      vtkInformationVector* outputVector)
{
  return this->Superclass::RequestData(request, inputVector, outputVector);
}
