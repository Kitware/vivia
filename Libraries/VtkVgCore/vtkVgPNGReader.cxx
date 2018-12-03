/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgPNGReader.h"

// VTK includes.
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPNGReader.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <vtksys/SystemTools.hxx>

// Standard VTK macro to implement New().
vtkStandardNewMacro(vtkVgPNGReader);

//-----------------------------------------------------------------------------
vtkVgPNGReader::vtkVgPNGReader() : vtkVgBaseImageSource(),
  ImageCache(0)
{
  this->Reader = vtkPNGReader::New();

  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
vtkVgPNGReader::~vtkVgPNGReader()
{
  if (this->ImageCache)
    {
    this->ImageCache->Delete();
    this->ImageCache = 0;
    }

  if (this->Reader)
    {
    this->Reader->Delete();
    this->Reader = 0;
    }
}

//-----------------------------------------------------------------------------
void vtkVgPNGReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
int vtkVgPNGReader::GetNumberOfLevels() const
{
  return 1;
}

//-----------------------------------------------------------------------------
void vtkVgPNGReader::GetDimensions(int dim[2])
{
  int* extents = this->Reader->GetDataExtent();
  dim[0] = extents[1] + 1;
  dim[1] = extents[3] + 1;
}

//----------------------------------------------------------------------------
bool vtkVgPNGReader::CanRead(const std::string& source) const
{
  std::string ciSource = vtksys::SystemTools::LowerCase(source);

  // Check if it is a filename or if the argument is just an extension.
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(ciSource);
  if (ext.compare(".png") == 0 || (ciSource.compare("png") == 0) ||
      (ciSource.compare(".png") == 0))
    {
    return true;
    }

  return false;
}

//----------------------------------------------------------------------------
std::string vtkVgPNGReader::GetShortDescription() const
{
  return "Reader for PNG file format";
}

//----------------------------------------------------------------------------
std::string vtkVgPNGReader::GetLongDescription() const
{
  return this->GetShortDescription();
}

//----------------------------------------------------------------------------
vtkVgBaseImageSource* vtkVgPNGReader::Create()
{
  return vtkVgPNGReader::New();
}

//-----------------------------------------------------------------------------
int vtkVgPNGReader::RequestInformation(vtkInformation* vtkNotUsed(request),
                                       vtkInformationVector** vtkNotUsed(inputVector),
                                       vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if (!outInfo)
    {
    vtkErrorMacro("Invalid output information object.");
    return 1;
    }

  if (!this->FileName)
    {
    vtkErrorMacro("Requires valid input file name.") ;
    return 1;
    }

  if (this->ImageCache)
    {
    this->ImageCache->Delete();
    }

  this->Reader->SetFileName(this->FileName);
  this->Reader->SetDataOrigin(this->Origin);
  this->Reader->SetDataSpacing(this->Spacing);
  this->Reader->Update();
  this->ImageCache = vtkImageData::New();
  this->ImageCache->ShallowCopy(this->Reader->GetOutput());

  int extents[6];
  this->ImageCache->GetExtent(extents);

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               extents, 6);
  outInfo->Set(vtkDataObject::SPACING(), this->Spacing, 3);

  outInfo->Set(vtkDataObject::ORIGIN(),  this->Origin, 3);

  return 1;
}

//-----------------------------------------------------------------------------
int vtkVgPNGReader::RequestData(vtkInformation* vtkNotUsed(request),
                                vtkInformationVector** vtkNotUsed(inputVector),
                                vtkInformationVector* outputVector)
{
  if (!outputVector)
    {
    vtkErrorMacro("Invalid output information vector.") ;
    return 1;
    }

  if (!this->FileName)
    {
    vtkErrorMacro("Requires valid input file name.") ;
    return 1;
    }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if (!outInfo)
    {
    vtkErrorMacro("Invalid output information object.");
    return 1;
    }

  vtkDataObject* dataObj = outInfo->Get(vtkDataObject::DATA_OBJECT());
  if (!dataObj)
    {
    vtkErrorMacro("Invalid output data object.");
    return 1;
    }

  vtkImageData* outputImage = vtkImageData::SafeDownCast(dataObj);
  if (!outputImage)
    {
    vtkErrorMacro("Output data object is not an image data object.");
    return 1;
    }

  if (!this->ImageCache)
    {
    vtkErrorMacro("Failed to create valid output.");
    return 1;
    }

  outputImage->ShallowCopy(this->ImageCache);

  return 1;
}
