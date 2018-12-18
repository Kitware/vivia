/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgJPEGReader.h"

// VTK includes.
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkJPEGReader.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <vtksys/SystemTools.hxx>

// Standard VTK macro to implement New().
vtkStandardNewMacro(vtkVgJPEGReader);

//-----------------------------------------------------------------------------
vtkVgJPEGReader::vtkVgJPEGReader() : vtkVgBaseImageSource(),
  ImageCache(0)
{
  this->Reader = vtkJPEGReader::New();

  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
vtkVgJPEGReader::~vtkVgJPEGReader()
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
void vtkVgJPEGReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
int vtkVgJPEGReader::GetNumberOfLevels() const
{
  return 1;
}

//-----------------------------------------------------------------------------
void vtkVgJPEGReader::GetDimensions(int dim[2])
{
  int* extents = this->Reader->GetDataExtent();
  dim[0] = extents[1] + 1;
  dim[1] = extents[3] + 1;
}

//----------------------------------------------------------------------------
bool vtkVgJPEGReader::CanRead(const std::string& source) const
{
  std::string ciSource = vtksys::SystemTools::LowerCase(source);

  // Check if it is a filename or if the argument is just an extension.
  std::string ext = vtksys::SystemTools::LowerCase(
    vtksys::SystemTools::GetFilenameLastExtension(ciSource));

  if ((ext.compare(".jpeg") == 0) || (ext.compare(".jpg") == 0) ||
      (ciSource.compare("jpeg") == 0) || (ciSource.compare(".jpeg") == 0) ||
      (ciSource.compare("jpg") == 0) || (ciSource.compare(".jpg") == 0))
    {
    return true;
    }

  return false;
}

//----------------------------------------------------------------------------
std::string vtkVgJPEGReader::GetShortDescription() const
{
  return "Reader for JPEG file format";
}

//----------------------------------------------------------------------------
std::string vtkVgJPEGReader::GetLongDescription() const
{
  return this->GetShortDescription();
}

//----------------------------------------------------------------------------
vtkVgBaseImageSource* vtkVgJPEGReader::Create()
{
  return vtkVgJPEGReader::New();
}

//-----------------------------------------------------------------------------
int vtkVgJPEGReader::RequestInformation(vtkInformation* vtkNotUsed(request),
                                       vtkInformationVector** vtkNotUsed(inputVector),
                                       vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if (!outInfo)
    {
    vtkErrorMacro("Invalid output information object.");
    return 0;
    }

  if (!this->FileName)
    {
    vtkErrorMacro("Requires valid input file name.") ;
    return 0;
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

  int     extents[6];
  this->ImageCache->GetExtent(extents);

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               extents, 6);
  outInfo->Set(vtkDataObject::SPACING(), this->Spacing, 3);

  outInfo->Set(vtkDataObject::ORIGIN(),  this->Origin, 3);

  return 1;
}

//-----------------------------------------------------------------------------
int vtkVgJPEGReader::RequestData(vtkInformation* vtkNotUsed(request),
                                vtkInformationVector** vtkNotUsed(inputVector),
                                vtkInformationVector* outputVector)
{
  if (!outputVector)
    {
    vtkErrorMacro("Invalid output information vector.") ;
    return 0;
    }

  if (!this->FileName)
    {
    vtkErrorMacro("Requires valid input file name.") ;
    return 0;
    }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if (!outInfo)
    {
    vtkErrorMacro("Invalid output information object.");
    return 0;
    }

  vtkDataObject* dataObj = outInfo->Get(vtkDataObject::DATA_OBJECT());
  if (!dataObj)
    {
    vtkErrorMacro("Invalid output data object.");
    return 0;
    }

  vtkImageData* outputImage = vtkImageData::SafeDownCast(dataObj);
  if (!outputImage)
    {
    vtkErrorMacro("Output data object is not an image data object.");
    return 0;
    }

  if (!this->ImageCache)
    {
    vtkErrorMacro("Failed to create valid output.");
    return 0;
    }

  outputImage->ShallowCopy(this->ImageCache);

  return 1;
}
