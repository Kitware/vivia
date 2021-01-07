// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgSGIReader.h"

#include <vtkImageAppendComponents.h>
#include <vtkImageData.h>
#include <vtkImageImport.h>
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
bool vtkVgSGIReader::CanRead(const std::string& source) const
{
  std::string ciSource = vtksys::SystemTools::LowerCase(source);

  // Check if it is a filename or if the argument is just an extension.
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(ciSource);
  if (ext == ".sgi" || (ciSource == "sgi" || ciSource == ".sgi"))
    {
    return true;
    }

  return false;
}

//----------------------------------------------------------------------------
std::string vtkVgSGIReader::GetShortDescription() const
{
  return "Reader for SGI file format";
}

//----------------------------------------------------------------------------
std::string vtkVgSGIReader::GetLongDescription() const
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
    return 0;
    }

  if (!this->FileName)
    {
    vtkErrorMacro("Requires valid input file name.");
    return 0;
    }

  // Open file
  std::ifstream in{this->FileName, std::ifstream::in | std::ifstream::binary};
  if (!in.good())
    {
    vtkErrorMacro("Failed to open input file.");
    return 0;
    }

  // Read header
  sgiImageHeader header;
  in.read(reinterpret_cast<char*>(&header), sizeof(header));
  if (!in.good())
    {
    vtkErrorMacro("Failed to read SGI header.");
    return 0;
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
    return 0;
    }

  // Check for supported formats
#define NOT_SUPPORTED(pre, val, post) \
    vtkErrorMacro( \
      "Failed to read SGI file: " << pre << (pre && *pre ? " " : "") << val << \
      post << (post && *post ? " " : "") << " is not supported."); \
    return 0

  if (header.BytesPerPixelChannel < 1 || header.BytesPerPixelChannel > 2)
    {
    NOT_SUPPORTED("", header.NumDimensions, "bytes per pixel channel");
    }
  if (header.StorageFormat != SGI_SF_Normal)
    {
    // TODO also support RLE?
    NOT_SUPPORTED("storage format", static_cast<int>(header.StorageFormat), "");
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
  unsigned numImageBytes = header.XDimension * header.YDimension *
                           header.ZDimension * header.BytesPerPixelChannel;
  char* imageBytes = new char[numImageBytes];

  in.read(imageBytes, numImageBytes);
  if (!in.good())
    {
    vtkErrorMacro("Failed to read SGI file image data.");
    return 0;
    }

  // Byte swap input data if needed
  if (header.BytesPerPixelChannel == 2)
    {
    for (unsigned i = 0; i < numImageBytes; ++i)
      {
      DiskToHardware(*reinterpret_cast<uint16_t*>(imageBytes + i));
      }
    }

  // Convert to vtkImageData
  const int xExtent = header.XDimension - 1;
  const int yExtent = header.YDimension - 1;
  const ptrdiff_t planeStride = header.XDimension * header.YDimension *
                                header.BytesPerPixelChannel;
  const int pdt = (header.BytesPerPixelChannel == 1 ? VTK_UNSIGNED_CHAR
                                                    : VTK_UNSIGNED_SHORT);

  auto components = vtkSmartPointer<vtkImageAppendComponents>::New();
  std::vector<vtkSmartPointer<vtkImageImport>> importers;
  for (int c = 0; c < header.ZDimension; ++c)
    {
    auto importer = vtkSmartPointer<vtkImageImport>::New();

    importer->SetDataScalarType(pdt);
    importer->SetNumberOfScalarComponents(1);
    importer->SetImportVoidPointer(imageBytes + (c * planeStride));
    importer->SetWholeExtent(0, xExtent, 0, yExtent, 0, 0);
    importer->SetDataExtentToWholeExtent();
    importer->Modified();
    importer->Update();

    components->AddInputConnection(importer->GetOutputPort());
    importers.push_back(importer);
    }
  components->Update();

  this->Internal->ImageData = vtkSmartPointer<vtkImageData>::New();
  this->Internal->ImageData->ShallowCopy(components->GetOutput());

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

  if (!this->Internal->ImageData)
    {
    vtkErrorMacro("Failed to create valid output.");
    return 0;
    }

  outputImage->ShallowCopy(this->Internal->ImageData);

  return 1;
}
