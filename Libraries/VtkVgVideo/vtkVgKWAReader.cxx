/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgKWAReader.h"

#include "vtkVgAdaptImage.h"

#include <vgImage.h>
#include <vgKwaUtil.h>

#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>

#include <qtStlUtil.h>

// Standard VTK macro to implement New().
vtkStandardNewMacro(vtkVgKWAReader);

#define CHECK_OR_DIE(expr, msg) \
  do { if (!(expr)) { vtkErrorMacro(msg); return 0; } } while(0)

//-----------------------------------------------------------------------------
class vtkVgKWAReader::vtkInternal
{
public:
  vgImage Image;
  vtkVgTimeStamp ImageTimeStamp;
};

//-----------------------------------------------------------------------------
vtkVgKWAReader::vtkVgKWAReader() : Internal{new vtkInternal}
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
vtkVgKWAReader::~vtkVgKWAReader()
{
}

//-----------------------------------------------------------------------------
void vtkVgKWAReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
int vtkVgKWAReader::GetNumberOfLevels() const
{
  return 1;
}

//-----------------------------------------------------------------------------
void vtkVgKWAReader::GetDimensions(int dim[2])
{
  if (this->Internal->Image)
  {
    dim[0] = this->Internal->Image.iCount();
    dim[1] = this->Internal->Image.jCount();
  }
  else
  {
    dim[0] = dim[1] = 0;
  }
}

//----------------------------------------------------------------------------
vtkVgTimeStamp vtkVgKWAReader::GetImageTimeStamp()
{
  return this->Internal->ImageTimeStamp;
}

//----------------------------------------------------------------------------
bool vtkVgKWAReader::CanRead(const std::string& source) const
{
  const auto& ciSource = qtString(source).toLower();

  // Check if it is a filename or if the argument is just an extension.
  const auto l = ciSource.length();
  auto n = 0 + l;
  while (n > 0 && ciSource[--n].isDigit());

  if (n > 5 && n + 1 < l && ciSource[n] == '@')
  {
    return (ciSource.mid(n - 5, 5) == QLatin1String(".data"));
  }

  return false;
}

//----------------------------------------------------------------------------
std::string vtkVgKWAReader::GetShortDescription() const
{
  return "Reader for Kitware Video Archive (KWA) frames";
}

//----------------------------------------------------------------------------
std::string vtkVgKWAReader::GetLongDescription() const
{
  return this->GetShortDescription();
}

//----------------------------------------------------------------------------
vtkVgBaseImageSource* vtkVgKWAReader::Create()
{
  return vtkVgKWAReader::New();
}

//-----------------------------------------------------------------------------
int vtkVgKWAReader::RequestInformation(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
{
  // Get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  CHECK_OR_DIE(outInfo, "Invalid output information object.");

  CHECK_OR_DIE(this->FileName, "Requires valid input file name.");

  const auto& pathWithOffset = QString::fromLocal8Bit(this->FileName);
  const auto n = pathWithOffset.lastIndexOf('@');
  CHECK_OR_DIE(n > 0, "Requires valid input file name.");

  const auto& path = pathWithOffset.left(n);
  const auto offset = pathWithOffset.mid(n + 1).toLongLong();

  const auto data = vgKwaUtil::readFrame(path, offset);
  CHECK_OR_DIE(data.second,
               "Failed to read KWA frame at offset " << offset << ".");

  this->Internal->Image = data.second;
  this->Internal->ImageTimeStamp.SetTime(data.first);

  return 1;
}

//-----------------------------------------------------------------------------
int vtkVgKWAReader::RequestData(vtkInformation* vtkNotUsed(request),
                                vtkInformationVector** vtkNotUsed(inputVector),
                                vtkInformationVector* outputVector)
{
  CHECK_OR_DIE(outputVector, "Invalid output information vector.");
  CHECK_OR_DIE(this->FileName, "Requires valid input file name.");

  auto* const outInfo = outputVector->GetInformationObject(0);
  CHECK_OR_DIE(outInfo, "Invalid output information object.");

  auto* const dataObj = outInfo->Get(vtkDataObject::DATA_OBJECT());
  CHECK_OR_DIE(dataObj, "Invalid output data object.");

  auto* const outputImage = vtkImageData::SafeDownCast(dataObj);
  CHECK_OR_DIE(outputImage, "Output data object is not an image data object.");

  CHECK_OR_DIE(this->Internal->Image, "Failed to create valid output.");

  outputImage->ShallowCopy(vtkVgAdapt(this->Internal->Image));

  return 1;
}
