/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgSGIReader.h"

#include "vtkVgImageBuffer.h"

#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <vtksys/SystemTools.hxx>

#include <type_traits>
#include <vector>

#include <cstdint>

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <netinet/in.h>
#endif

// Standard VTK macro to implement New().
vtkStandardNewMacro(vtkVgSGIReader);

namespace
{

enum SGI_StorageFormat : uint8_t
{
  SGI_SF_Normal = 0,
  SGI_SF_RLE = 1,
};

enum SGI_ColorMap : uint32_t
{
  SGI_CM_Normal = 0,
  SGI_CM_Dithered = 1,
  SGI_CM_Screen = 2,
  SGI_CM_Colormap = 3,
};

struct sgiImageHeader
{
  uint16_t          Magic;
  SGI_StorageFormat StorageFormat;
  uint8_t           BytesPerPixelChannel;
  uint16_t          NumDimensions;
  uint16_t          XDimension;
  uint16_t          YDimension;
  uint16_t          ZDimension;
  uint32_t          MinValue;
  uint32_t          MaxValue;
  uint32_t          Dummy1; // not used
  char              ImageName[80];
  SGI_ColorMap      ColorMap;
  char              Dummy2[404];
};

//-----------------------------------------------------------------------------
void DiskToHardware(uint8_t&) {}
void DiskToHardware(uint16_t& val) { val = ntohs(val); }
void DiskToHardware(uint32_t& val) { val = ntohl(val); }

//-----------------------------------------------------------------------------
template <typename Enum>
void DiskToHardware(Enum& enumValue)
{
  using Underlying = typename std::underlying_type<Enum>::type;
  auto& castedValue = reinterpret_cast<Underlying&>(enumValue);
  DiskToHardware(castedValue);
}

//-----------------------------------------------------------------------------
template <typename T>
vtkSmartPointer<vtkImageData>
readImage(std::ifstream& in, const sgiImageHeader& header, int vtkPixelType,
          std::function<T(T)> convertPixel)
{
  // Allocate storage for image
  vtkVgImageBuffer buffer{vtkPixelType};

  const size_t i = header.XDimension;
  const size_t j = header.YDimension;
  const size_t k = header.ZDimension;
  const ptrdiff_t is = header.BytesPerPixelChannel;
  const ptrdiff_t js = is * static_cast<ptrdiff_t>(i);
  const ptrdiff_t ks = js * static_cast<ptrdiff_t>(j);
  buffer.SetLayout(i, j, k, is, js, ks);
  buffer.Allocate();

  const auto bufferSize = static_cast<std::streamsize>(buffer.GetSize());
  in.read(buffer.GetMutableData(), bufferSize);
  if (!in.good())
    {
    return nullptr;
    }

  // Convert to vtkImageData
  return buffer.Convert(convertPixel);
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vtkVgSGIReader::vtkInternal
{
public:
  vtkSmartPointer<vtkImageData> ImageData;
};

//-----------------------------------------------------------------------------
vtkVgSGIReader::vtkVgSGIReader() : Internal{new vtkInternal}
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
vtkVgSGIReader::~vtkVgSGIReader()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgSGIReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
int vtkVgSGIReader::GetNumberOfLevels() const
{
  return 1;
}

//-----------------------------------------------------------------------------
void vtkVgSGIReader::GetDimensions(int dim[2])
{
  if (this->Internal->ImageData)
    {
    int fulldim[3];
    this->Internal->ImageData->GetDimensions(fulldim);
    dim[0] = fulldim[0];
    dim[1] = fulldim[1];
    }
  else
    {
    dim[0] = dim[1] = 0;
    }
}

//----------------------------------------------------------------------------
bool vtkVgSGIReader::CanRead(const std::string& source)
{
  std::string ciSource = vtksys::SystemTools::LowerCase(source);

  // Check if it is a file name or if the argument is just an extension.
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(ciSource);
  if (ext.compare(".sgi") == 0 || (ciSource.compare("sgi") == 0) ||
      (ciSource.compare(".sgi") == 0))
    {
    return true;
    }

  return false;
}

//----------------------------------------------------------------------------
std::string vtkVgSGIReader::GetShortDescription()
{
  return "Reader for SGI file format";
}

//----------------------------------------------------------------------------
std::string vtkVgSGIReader::GetLongDescription()
{
  return this->GetShortDescription();
}

//----------------------------------------------------------------------------
vtkVgBaseImageSource* vtkVgSGIReader::Create()
{
  return vtkVgSGIReader::New();
}

//-----------------------------------------------------------------------------
int vtkVgSGIReader::RequestInformation(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
{
  // Get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if (!outInfo)
    {
    vtkErrorMacro("Invalid output information object.");
    return 1;
    }

  if (!this->FileName)
    {
    vtkErrorMacro("Requires valid input file name.");
    return 1;
    }

  // Open file
  std::ifstream in{this->FileName, std::ifstream::in | std::ifstream::binary};
  if (!in.good())
    {
    vtkErrorMacro("Failed to open input file.");
    return 1;
    }

  // Read header
  sgiImageHeader header;
  in.read(reinterpret_cast<char*>(&header), sizeof(header));
  if (!in.good())
    {
    vtkErrorMacro("Failed to read SGI header.");
    return 1;
    }
  DiskToHardware(header.Magic);
  DiskToHardware(header.StorageFormat);
  DiskToHardware(header.BytesPerPixelChannel);
  DiskToHardware(header.NumDimensions);
  DiskToHardware(header.XDimension);
  DiskToHardware(header.YDimension);
  DiskToHardware(header.ZDimension);
  DiskToHardware(header.MinValue);
  DiskToHardware(header.MaxValue);
  DiskToHardware(header.ColorMap);
  if (header.Magic != 474)
    {
    vtkErrorMacro("Failed to read SGI file: bad magic.");
    return 1;
    }

  // Check for supported formats
#define NOT_SUPPORTED(pre, val, post) \
    vtkErrorMacro( \
      "Failed to read SGI file: " << pre << (pre && *pre ? " " : "") << val << \
      post << (post && *post ? " " : "") << " is not supported."); \
    return 1

  if (header.StorageFormat != SGI_SF_Normal)
    {
    // TODO also support RLE?
    NOT_SUPPORTED("storage format", header.StorageFormat, "");
    }
  if (header.ColorMap != SGI_CM_Normal)
    {
    // TODO also support dithered?
    NOT_SUPPORTED("color map type", header.ColorMap, "");
    }
  if (header.NumDimensions < 2 || header.NumDimensions > 3)
    {
    NOT_SUPPORTED("dimensionality", header.NumDimensions, "");
    }
  if (header.NumDimensions == 2)
    {
    header.ZDimension = 1;
    }

  // Read image data
  switch (header.BytesPerPixelChannel)
    {
    case 1:
      this->Internal->ImageData =
        readImage<uint8_t>(in, header, VTK_UNSIGNED_CHAR, nullptr);
      break;

    case 2:
      this->Internal->ImageData =
        readImage<uint16_t>(in, header, VTK_UNSIGNED_SHORT, ntohs);
      break;

    default:
      NOT_SUPPORTED("", header.NumDimensions, "bytes per pixel channel");
    }

  if (!this->Internal->ImageData)
    {
    vtkErrorMacro("Failed to read SGI file image data.");
    return 1;
    }

  // Set data on info objects
  int extents[6];
  this->Internal->ImageData->GetExtent(extents);

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               extents, 6);
  outInfo->Set(vtkDataObject::SPACING(), this->Spacing, 3);

  outInfo->Set(vtkDataObject::ORIGIN(),  this->Origin, 3);

  return 1;
}

//-----------------------------------------------------------------------------
int vtkVgSGIReader::RequestData(vtkInformation* vtkNotUsed(request),
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

  if (!this->Internal->ImageData)
    {
    vtkErrorMacro("Failed to create valid output.");
    return 1;
    }

  outputImage->ShallowCopy(this->Internal->ImageData);

  return 1;
}
