/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgGDALReader.h"

#include "vtkGDALReader.h"
#include "vtkVgNitfMetaDataParser.h"

#include <vtkVgCoordinateTransformUtil.h>

#include <vgStringUtils.h>

// VTK includes.
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <vtksys/SystemTools.hxx>

// C++ includes
#include <sstream>

// Standard VTK macro to implement New().
vtkStandardNewMacro(vtkVgGDALReader);

//-----------------------------------------------------------------------------
vtkVgGDALReader::vtkVgGDALReader() : vtkVgBaseImageSource(),
  ImageCache(0)
{
  this->Reader = vtkGDALReader::New();

  this->OutputResolution[0] = 1280;
  this->OutputResolution[1] = 1024;

  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
vtkVgGDALReader::~vtkVgGDALReader()
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

//----------------------------------------------------------------------------
int vtkVgGDALReader::SetOutputResolution(int w, int h)
{
  this->OutputResolution[0] = w;
  this->OutputResolution[1] = h;

  this->Modified();

  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vtkVgGDALReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
int vtkVgGDALReader::GetNumberOfLevels() const
{
  return 1;
}

//-----------------------------------------------------------------------------
const double* vtkVgGDALReader::GetGeoCornerPoints()
{
  if (this->Reader)
    {
    return this->Reader->GetGeoCornerPoints();
    }

  return 0;
}

//-----------------------------------------------------------------------------
bool vtkVgGDALReader::GetRasterDimensions(int dim[2])
{
  this->Reader->GetRasterDimensions(dim);
  return true;
}

//-----------------------------------------------------------------------------
void vtkVgGDALReader::GetDimensions(int dim[2])
{
  int* extents = this->Reader->GetDataExtent();
  dim[0] = extents[1] - extents[0] + 1;
  dim[1] = extents[3] - extents[2] + 1;
}

//----------------------------------------------------------------------------
vtkVgTimeStamp vtkVgGDALReader::GetImageTimeStamp()
{
  return this->ImageTimeStamp;
}

//----------------------------------------------------------------------------
bool vtkVgGDALReader::CanRead(const std::string& source) const
{
  std::string ciSource = vtksys::SystemTools::LowerCase(source);

  // Check if it is a filename or if the argument is just an extension.
  std::string ext = vtksys::SystemTools::LowerCase(
                      vtksys::SystemTools::GetFilenameLastExtension(ciSource));

  // Do not read PNG, JPEG, MRJ
  if ((ext.compare(".jpeg") == 0) || (ext.compare(".jpg") == 0) ||
      (ciSource.compare("jpeg") == 0) || (ciSource.compare(".jpeg") == 0) ||
      (ciSource.compare("jpg") == 0) || (ciSource.compare(".jpg") == 0) ||
      (ext.compare(".png") == 0) || (ciSource.compare("png") == 0) ||
      (ciSource.compare(".png") == 0) ||
      (ext.compare(".mrj") == 0) || (ciSource.compare("mrj") == 0) ||
      (ciSource.compare(".mrj") == 0))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
std::string vtkVgGDALReader::GetShortDescription() const
{
  return "Generic image reader implemented using GDAL library";
}

//----------------------------------------------------------------------------
std::string vtkVgGDALReader::GetLongDescription() const
{
  return this->GetShortDescription();
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkMatrix4x4> vtkVgGDALReader::GetImageToWorldMatrix()
{
  int imageDimensions[2];
  this->Reader->GetRasterDimensions(imageDimensions);
  const double* geoCornerPts = this->Reader->GetGeoCornerPoints();

  // In GDAL 0,0 refers to the corner of a pixel while in VTK it refers to the
  // center, hence the offsets of 0.5 need to be applied here.
  return vtkVgCoordinateTransformUtil::ComputeImageToLatLonMatrix(
    -0.5, -0.5, imageDimensions[0] - 0.5, imageDimensions[1] - 0.5,
    geoCornerPts[2], geoCornerPts[3],
    geoCornerPts[4], geoCornerPts[5],
    geoCornerPts[6], geoCornerPts[7],
    geoCornerPts[0], geoCornerPts[1]);
}

//----------------------------------------------------------------------------
vtkVgBaseImageSource* vtkVgGDALReader::Create()
{
  return vtkVgGDALReader::New();
}

//-----------------------------------------------------------------------------
int vtkVgGDALReader::RequestInformation(
  vtkInformation* vtkNotUsed(request),
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

  int extents[6] =
    {
    this->ReadExtents[0],
    this->ReadExtents[1],
    this->ReadExtents[2],
    this->ReadExtents[3],
    0,
    0
    };

  this->Reader->SetFileName(this->FileName);
  this->Reader->SetDataExtents(extents);
  this->Reader->SetTargetDimensions(this->OutputResolution[0],
                                    this->OutputResolution[1]);
  this->Reader->UpdateInformation();

  this->Reader->GetDataSpacing(this->Spacing);
  this->Reader->GetDataOrigin(this->Origin);

  int* ext = this->Reader->GetDataExtent();
  extents[0] = ext[0];
  extents[1] = ext[1];
  extents[2] = ext[2];
  extents[3] = ext[3];
  extents[4] = ext[4];
  extents[5] = ext[5];

  // Now compute image timestamp if the information is available
  this->ComputeImageTimeStamp();

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               extents, 6);
  outInfo->Set(vtkDataObject::SPACING(), this->Spacing, 3);

  outInfo->Set(vtkDataObject::ORIGIN(),  this->Origin, 3);

  return 1;
}

//-----------------------------------------------------------------------------
int vtkVgGDALReader::RequestData(vtkInformation* vtkNotUsed(request),
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

  if (this->ImageCache)
    {
    this->ImageCache->Delete();
    }

  this->ImageCache = vtkImageData::New();
  this->Reader->Update();
  this->ImageCache->ShallowCopy(this->Reader->GetOutput());

  if (!this->ImageCache)
    {
    vtkErrorMacro("Failed to create valid output.");
    return 1;
    }

  this->Reader->GetDataSpacing(this->Spacing);
  this->Reader->GetDataOrigin(this->Origin);
  this->ImageCache->SetSpacing(this->Spacing);
  this->ImageCache->SetOrigin(this->Origin);

  outputImage->ShallowCopy(this->ImageCache);

  return 1;
}

//-----------------------------------------------------------------------------
void vtkVgGDALReader::ComputeImageTimeStamp()
{
  if (this->Reader->GetMTime() > this->GetMTime())
    {
    std::string ext =
      vtksys::SystemTools::GetFilenameLastExtension(this->FileName);
    ext = vtksys::SystemTools::LowerCase(ext);

    // TODO Use regex
    if (ext.compare(".nitf") == 0 ||
        ext.compare(".ntf") == 0 ||
        ext.compare(".r0") == 0)
      {
      vtkVgNitfMetaDataParser::ParseDateTime(
        this->Reader->GetMetaData(), this->Reader->GetDomainMetaData("TRE"),
        this->ImageTimeStamp);
      }
    }
}
