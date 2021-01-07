// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkGDALReader.h"

// VTK includes
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkShortArray.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedShortArray.h>

// GDAL includes
#include <gdal_alg.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>

// C/C++ includes
#include <cassert>
#include <iostream>
#include <vector>

vtkStandardNewMacro(vtkGDALReader);

namespace
{
  double Min(double val1, double val2)
    {
    return ((val1 < val2) ? val1 : val2);
    }

  double Max(double val1, double val2)
    {
    return ((val1 > val2) ? val1 : val2);
    }
}

class vtkGDALReader::vtkGDALReaderInternal
{
public:
  typedef void *OGRCoordinateTransformationH;

  vtkGDALReaderInternal(vtkGDALReader* reader);
  ~vtkGDALReaderInternal();

  void ReadMetaData(const std::string& fileName);
  void ReadData(const std::string& fileName);

  template <typename VTK_TYPE, typename RAW_TYPE> void GenericReadData();
  void ReleaseData();

  template <typename VTK_TYPE, typename RAW_TYPE>
  void Convert(std::vector<RAW_TYPE>& rawImageData,
               int targetWidth, int targetHeight, int numberOfUsedBands);

  bool GetGeoCornerPoint(GDALDataset* dataset,
                         OGRCoordinateTransformationH htransform,
                         double x, double y, double* out) const;

  const double* GetGeoCornerPoints();

  int NumberOfBands;
  int NumberOfBytesPerPixel;

  int SourceOffset[2];
  int SourceDimensions[2];

  std::string PrevReadFileName;

  GDALDataset* GDALData;
  GDALDataType TargetDataType;

  // Bad corner point
  double BadCornerPoint;

  // Upper left, lower left, upper right, lower right
  double CornerPoints[8];

  vtkSmartPointer<vtkImageData> ImageData;
  vtkGDALReader* Reader;
};

//-----------------------------------------------------------------------------
vtkGDALReader::vtkGDALReaderInternal::vtkGDALReaderInternal(
  vtkGDALReader* reader) :
  NumberOfBands(0),
  NumberOfBytesPerPixel(0),
  GDALData(0),
  TargetDataType(GDT_Byte),
  BadCornerPoint(-1),
  ImageData(0),
  Reader(reader)
{
  this->SourceOffset[0] = 0;
  this->SourceOffset[1] = 0;
  this->SourceDimensions[0] = 0;
  this->SourceDimensions[1] = 0;

  for (int i = 0; i < 8; ++i)
    {
    this->CornerPoints[i] = this->BadCornerPoint;
    }

  // Enable all the drivers.
  GDALAllRegister();
}

//-----------------------------------------------------------------------------
vtkGDALReader::vtkGDALReaderInternal::~vtkGDALReaderInternal()
{
  this->ReleaseData();
}

//-----------------------------------------------------------------------------
void vtkGDALReader::vtkGDALReaderInternal::ReadMetaData(
  const std::string& fileName)
{
  if (fileName.compare(this->PrevReadFileName) == 0)
    {
    return;
    }

  // Free up the last read data, if any.
  this->ReleaseData();

  this->GDALData = (GDALDataset*) GDALOpen(fileName.c_str(), GA_ReadOnly);

  if (this->GDALData == NULL)
    {
    std::cout << "NO GDALData loaded for file "
              << fileName << std::endl;
    }
  else
    {
    this->PrevReadFileName = fileName;
    this->NumberOfBands = this->GDALData->GetRasterCount();

    // Clear last read metadata
    this->Reader->MetaData.clear();

    this->Reader->RasterDimensions[0] = this->GDALData->GetRasterXSize();
    this->Reader->RasterDimensions[1] = this->GDALData->GetRasterYSize();

    GDALDriverH driver = GDALGetDatasetDriver(this->GDALData);
    this->Reader->DriverShortName = GDALGetDriverShortName(driver);
    this->Reader->DriverLongName = GDALGetDriverLongName(driver);

    char** papszMetaData = GDALGetMetadata(this->GDALData, NULL);
    if (CSLCount(papszMetaData) > 0)
      {
      for (int i = 0; papszMetaData[i] != NULL; ++i)
        {
        this->Reader->MetaData.push_back(papszMetaData[i]);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vtkGDALReader::vtkGDALReaderInternal::ReadData(
  const std::string& fileName)
{
  // If data is not initialized by now, it means that we were unable to read
  // the file.
  if (!this->GDALData)
    {
    std::cerr << "Failed to read: " << fileName << std::endl;
    return;
    }

  for (int i = 1; i <= this->NumberOfBands; ++i)
    {
    GDALRasterBand* rasterBand = this->GDALData->GetRasterBand(i);
    if (this->NumberOfBytesPerPixel == 0)
      {
      this->TargetDataType = rasterBand->GetRasterDataType();
      switch (this->TargetDataType)
        {
        case (GDT_Byte): this->NumberOfBytesPerPixel = 1; break;
        case (GDT_UInt16): this->NumberOfBytesPerPixel = 2; break;
        case (GDT_Int16): this->NumberOfBytesPerPixel = 2; break;
        case (GDT_UInt32): this->NumberOfBytesPerPixel = 4; break;
        case (GDT_Int32): this->NumberOfBytesPerPixel = 4; break;
        case (GDT_Float32): this->NumberOfBytesPerPixel = 4; break;
        case (GDT_Float64): this->NumberOfBytesPerPixel = 8; break;
        default: this->NumberOfBytesPerPixel = 0; break;
        }
      }
    }

  // Initialize
  this->ImageData = vtkSmartPointer<vtkImageData>::New();

  switch (this->TargetDataType)
    {
    case (GDT_UInt16):
      {
      this->Reader->SetDataScalarTypeToUnsignedShort();
      this->GenericReadData<vtkUnsignedShortArray, unsigned short>();
      break;
      }
    case (GDT_Int16):
      {
      this->Reader->SetDataScalarTypeToShort();
      this->GenericReadData<vtkShortArray, short>();
      break;
      }
    case (GDT_UInt32):
      {
      this->Reader->SetDataScalarTypeToUnsignedInt();
      this->GenericReadData<vtkUnsignedIntArray, unsigned int>();
      break;
      }
    case (GDT_Int32):
      {
      this->Reader->SetDataScalarTypeToInt();
      this->GenericReadData<vtkIntArray, int>();
      break;
      }
    case (GDT_Float32):
      {
      this->Reader->SetDataScalarTypeToFloat();
      this->GenericReadData<vtkFloatArray, float>();
      break;
      }
    case (GDT_Float64):
      {
      this->Reader->SetDataScalarTypeToDouble();
      this->GenericReadData<vtkDoubleArray, double>();
      break;
      }
    case (GDT_Byte):
    default:
      {
      this->Reader->SetDataScalarTypeToUnsignedChar();
      this->GenericReadData<vtkUnsignedCharArray, unsigned char>();
      break;
      }
    }
}

//-----------------------------------------------------------------------------
template <typename VTK_TYPE, typename RAW_TYPE>
void vtkGDALReader::vtkGDALReaderInternal::GenericReadData()
{
  // Pixel data.
  std::vector<RAW_TYPE> rawImageData;

  // Possible bands
  GDALRasterBand* redBand = 0;
  GDALRasterBand* greenBand = 0;
  GDALRasterBand* blueBand = 0;
  GDALRasterBand* alphaBand = 0;
  GDALRasterBand* greyBand = 0;

  int numberOfUsedBands = 0;
  for (int i = 1; i <= this->NumberOfBands; ++i)
    {
    GDALRasterBand* rasterBand = this->GDALData->GetRasterBand(i);
    if (this->NumberOfBytesPerPixel == 0)
      {
      this->TargetDataType = rasterBand->GetRasterDataType();
      switch (this->TargetDataType)
        {
        case (GDT_Byte): this->NumberOfBytesPerPixel = 1; break;
        case (GDT_UInt16): this->NumberOfBytesPerPixel = 2; break;
        case (GDT_Int16): this->NumberOfBytesPerPixel = 2; break;
        case (GDT_UInt32): this->NumberOfBytesPerPixel = 4; break;
        case (GDT_Int32): this->NumberOfBytesPerPixel = 4; break;
        case (GDT_Float32): this->NumberOfBytesPerPixel = 4; break;
        case (GDT_Float64): this->NumberOfBytesPerPixel = 8; break;
        default: this->NumberOfBytesPerPixel = 0; break;
        }
      }

    if ((rasterBand->GetColorInterpretation() == GCI_RedBand) ||
        (rasterBand->GetColorInterpretation() == GCI_YCbCr_YBand))
      {
      redBand = rasterBand;
      ++numberOfUsedBands;
      }
    else if ((rasterBand->GetColorInterpretation() == GCI_GreenBand) ||
             (rasterBand->GetColorInterpretation() == GCI_YCbCr_CbBand))
      {
      greenBand = rasterBand;
      ++numberOfUsedBands;
      }
    else if ((rasterBand->GetColorInterpretation() == GCI_BlueBand) ||
             (rasterBand->GetColorInterpretation() == GCI_YCbCr_CrBand))
      {
      blueBand = rasterBand;
      ++numberOfUsedBands;
      }
    else if (rasterBand->GetColorInterpretation() == GCI_AlphaBand)
      {
      alphaBand = rasterBand;
      ++numberOfUsedBands;
      }
    else if (rasterBand->GetColorInterpretation() == GCI_GrayIndex)
      {
      greyBand = rasterBand;
      ++numberOfUsedBands;
      }
    }

  const int& destWidth = this->Reader->TargetDimensions[0];
  const int& destHeight = this->Reader->TargetDimensions[1];

  // GDAL top left is at 0,0
  const int& windowX = this->SourceOffset[0];
  const int& windowY = this->SourceOffset[1];
  const int& windowWidth = this->SourceDimensions[0];
  const int& windowHeight = this->SourceDimensions[1];

  const int& pixelSpace = this->NumberOfBytesPerPixel;
  const int lineSpace = destWidth * pixelSpace;
  const int bandSpace = destWidth * destHeight * this->NumberOfBytesPerPixel;
  CPLErr err;

  // TODO: Support other band types
  if (redBand && greenBand && blueBand)
    {
    if (alphaBand)
      {
      this->Reader->SetNumberOfScalarComponents(4);
      rawImageData.resize(4 * destWidth * destHeight * pixelSpace);

      err = redBand->RasterIO(
              GF_Read, windowX, windowY,  windowWidth, windowHeight,
              (void*)((GByte*)&rawImageData[0] + 0 * bandSpace), destWidth, destHeight,
              this->TargetDataType, pixelSpace, lineSpace);
      err = greenBand->RasterIO(
              GF_Read, windowX, windowY,  windowWidth, windowHeight,
              (void*)((GByte*)&rawImageData[0] + 1 * bandSpace), destWidth, destHeight,
              this->TargetDataType, pixelSpace, lineSpace);
      err = blueBand->RasterIO(
              GF_Read, windowX, windowY,  windowWidth, windowHeight,
              (void*)((GByte*)&rawImageData[0] + 2 * bandSpace), destWidth, destHeight,
              this->TargetDataType, pixelSpace, lineSpace);
      err = alphaBand->RasterIO(
              GF_Read, windowX, windowY,  windowWidth, windowHeight,
              (void*)((GByte*)&rawImageData[0] + 3 * bandSpace), destWidth, destHeight,
              this->TargetDataType, pixelSpace, lineSpace);
      }
    else
      {
      this->Reader->SetNumberOfScalarComponents(3);
      rawImageData.resize(3 * destWidth * destHeight * pixelSpace);

      err = redBand->RasterIO(
              GF_Read, windowX, windowY,  windowWidth, windowHeight,
              (void*)((GByte*)&rawImageData[0] + 0 * bandSpace), destWidth, destHeight,
              this->TargetDataType, 0, 0);
      err = greenBand->RasterIO(GF_Read, windowX, windowY,  windowWidth, windowHeight,
             (void*)((GByte*)&rawImageData[0] + 1 * bandSpace), destWidth, destHeight,
              this->TargetDataType, 0,0);
      err = blueBand->RasterIO(GF_Read, windowX, windowY,  windowWidth, windowHeight,
              (void*)((GByte*)&rawImageData[0] + 2 * bandSpace), destWidth, destHeight,
              this->TargetDataType, 0,0);
      }
    }
  else if (greyBand)
    {
    if (alphaBand)
      {
      // Luminance alpha
      this->Reader->SetNumberOfScalarComponents(2);
      rawImageData.resize(2 * destWidth * destHeight * pixelSpace);

      err = greyBand->RasterIO(
              GF_Read, windowX, windowY,  windowWidth, windowHeight,
              (void*)((GByte*)&rawImageData[0] + 0 * bandSpace), destWidth, destHeight,
              this->TargetDataType, pixelSpace, lineSpace);
      err =  alphaBand->RasterIO(
               GF_Read, windowX, windowY,  windowWidth, windowHeight,
               (void*)((GByte*)&rawImageData[0] + 1 * bandSpace), destWidth, destHeight,
               this->TargetDataType, pixelSpace, lineSpace);
      }
    else
      {
      // Luminance
      this->Reader->SetNumberOfScalarComponents(1);
      rawImageData.resize(destWidth * destHeight * pixelSpace);
      err = greyBand->RasterIO(
              GF_Read, windowX, windowY,  windowWidth, windowHeight,
              (void*)((GByte*)&rawImageData[0] + 0 * bandSpace), destWidth, destHeight,
              this->TargetDataType, pixelSpace, lineSpace);
      }
    }
  else
    {
    std::cerr << "Unknown raster band type \n";
    return;
    }

  assert(err == CE_None);

  // SEt meta data on the image
  this->ImageData->SetExtent(0, (destWidth - 1), 0, (destHeight - 1), 0, 0);
  this->ImageData->SetSpacing(this->Reader->DataSpacing[0],
                              this->Reader->DataSpacing[1],
                              this->Reader->DataSpacing[2]);
  this->ImageData->SetOrigin(this->Reader->DataOrigin[0],
                             this->Reader->DataOrigin[1],
                             this->Reader->DataOrigin[2]);

  this->Convert<VTK_TYPE, RAW_TYPE>(rawImageData, destWidth, destHeight,
                                    numberOfUsedBands);
}

//----------------------------------------------------------------------------
void vtkGDALReader::vtkGDALReaderInternal::ReleaseData()
{
  if (this->GDALData)
    {
    delete this->GDALData;
    this->GDALData = 0;
    }
}

//-----------------------------------------------------------------------------
template <typename VTK_TYPE, typename RAW_TYPE>
void vtkGDALReader::vtkGDALReaderInternal::Convert(
  std::vector<RAW_TYPE>& rawImageData, int targetWidth,
  int targetHeight, int numberOfUsedBands)
{
  if (!this->ImageData)
    {
    return;
    }

  vtkIdType targetIndex;
  vtkIdType sourceIndex;

  vtkSmartPointer<VTK_TYPE> scArr(vtkSmartPointer<VTK_TYPE>::New());
  scArr->SetNumberOfComponents(numberOfUsedBands);
  scArr->SetNumberOfTuples(targetWidth * targetHeight);

  for (int j = 0, k = (targetHeight - 1); j < targetHeight; ++j, --k)
    {
    for (int i = 0; i < targetWidth; ++i)
      {
      // Each band GDALData is stored in width * height size array.
      for (int bandIndex = 0; bandIndex < numberOfUsedBands; ++bandIndex)
        {
        targetIndex = i * numberOfUsedBands +
                      j * targetWidth * numberOfUsedBands + bandIndex;
        sourceIndex = i + k * targetWidth +
                      bandIndex * targetWidth * targetHeight;

        scArr->InsertValue(targetIndex, rawImageData[sourceIndex]);
        }
      }
    }
  this->ImageData->GetPointData()->SetScalars(scArr);
}

//-----------------------------------------------------------------------------
bool vtkGDALReader::vtkGDALReaderInternal::GetGeoCornerPoint(GDALDataset* dataset,
  OGRCoordinateTransformationH htransform, double x, double y, double* out) const
{
  bool retVal = false;

  if (!dataset)
    {
    std::cerr << "Empty GDAL dataset" << std::endl;
    return retVal;
    }

  if (!out)
    {
    return retVal;
    }

  double dfGeoX;
  double dfGeoY;
  double adfGeoTransform[6];

  const char *gcpProj = this->GDALData->GetGCPProjection();
  const GDAL_GCP *gcps = this->GDALData->GetGCPs();

  if (gcpProj == NULL || gcps == NULL)
    {
    // Transform the point into georeferenced coordinates
    if (GDALGetGeoTransform(this->GDALData, adfGeoTransform) == CE_None)
      {
      dfGeoX = adfGeoTransform[0] + adfGeoTransform[1] * x +
        adfGeoTransform[2] * y;
      dfGeoY = adfGeoTransform[3] + adfGeoTransform[4] * x +
        adfGeoTransform[5] * y;

      retVal = true;
      }
    else
      {
      // If an RPC model is available, use it to compute the corner point.
      // The RPCCoefficientTag is always describing a transformation to WGS84.
      char** metadata;
      GDALRPCInfo rpcInfo;
      if ((metadata = GDALGetMetadata(this->GDALData, "RPC")) &&
          GDALExtractRPCInfo(metadata, &rpcInfo))
        {
        // Specifying FALSE for second argument in this call and following
        // GDALRPCTransform() indicates we're transforming from source
        // (pixel/line) to destination (georeferenced).
        void* pTransformArg =
          GDALCreateRPCTransformer(&rpcInfo, FALSE, 0.1, NULL);

        dfGeoX = x;
        dfGeoY = y;
        double dfGeoZ = 0;
        int success;
        GDALRPCTransform(pTransformArg, FALSE, 1, &dfGeoX, &dfGeoY, &dfGeoZ,
                         &success);

        retVal = !!success;
        }
      else
        {
        dfGeoX = x;
        dfGeoY = y;
        retVal = false;
        }
      }
    }
  else
    {
    // 1st pass: we should realy have a call to the reader that returns
    // the homography, but for now, look for mathcing corner and pass back
    // the matching corner point ("0" pixel on input means "0.5" as far as
    // GDAL is concerned)
    bool leftCorner = (x == 0);
    bool upperCorner = (y == 0);
    for (int i = 0; i < 4; ++i)
      {
      bool gcpLeftCorner = (gcps[i].dfGCPPixel == 0.5);
      bool gcpUpperCorner = (gcps[i].dfGCPLine == 0.5);
      if (gcpLeftCorner == leftCorner && gcpUpperCorner == upperCorner)
        {
        dfGeoX = gcps[i].dfGCPX;
        dfGeoY = gcps[i].dfGCPY;
        }
      }
    retVal = true;
    }

  out[0] = dfGeoX;
  out[1] = dfGeoY;

  return retVal;
}

//-----------------------------------------------------------------------------
const double* vtkGDALReader::vtkGDALReaderInternal::GetGeoCornerPoints()
{
  if (!this->GetGeoCornerPoint(this->GDALData, 0,
                               0,
                               0,
                               &this->CornerPoints[0]))
    {
    return 0;
    }

  // If the first call succeeds, each additional call should as well
  // (this->GDALData hasn't changed).
  this->GetGeoCornerPoint(this->GDALData, 0,
                          0,
                          this->Reader->RasterDimensions[1],
                          &this->CornerPoints[2]);
  this->GetGeoCornerPoint(this->GDALData, 0,
                          this->Reader->RasterDimensions[0],
                          this->Reader->RasterDimensions[1],
                          &this->CornerPoints[4]);
  this->GetGeoCornerPoint(this->GDALData, 0,
                          this->Reader->RasterDimensions[0],
                          0,
                          &this->CornerPoints[6]);

  return this->CornerPoints;
}

//-----------------------------------------------------------------------------
vtkGDALReader::vtkGDALReader() :
  FileName(0),
  Implementation(0)
{
  this->Implementation = new vtkGDALReaderInternal(this);

  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);

  this->DataOrigin[0] = 0.0;
  this->DataOrigin[1] = 0.0;
  this->DataOrigin[2] = 0.0;

  this->DataSpacing[0] = 1.0;
  this->DataSpacing[1] = 1.0;
  this->DataSpacing[2] = 1.0;

  this->DataExtent[0] = -1;
  this->DataExtent[1] = -1;
  this->DataExtent[2] = -1;
  this->DataExtent[3] = -1;
  this->DataExtent[4] = -1;
  this->DataExtent[5] = -1;

  this->TargetDimensions[0] = -1;
  this->TargetDimensions[1] = -1;

  this->RasterDimensions[0] = -1;
  this->RasterDimensions[1] = -1;
}

//-----------------------------------------------------------------------------
vtkGDALReader::~vtkGDALReader()
{
  if (this->Implementation)
    {
    delete this->Implementation;
    this->Implementation = 0;
    }

  if (this->FileName)
    {
    this->SetFileName(0);
    }
}

//-----------------------------------------------------------------------------
const char* vtkGDALReader::GetProjectionString() const
{
  return this->Projection.c_str();
}

//-----------------------------------------------------------------------------
const double* vtkGDALReader::GetGeoCornerPoints()
{
  return this->Implementation->GetGeoCornerPoints();
}

//-----------------------------------------------------------------------------
const std::vector<std::string>& vtkGDALReader::GetMetaData()
{
  return this->MetaData;
}

//-----------------------------------------------------------------------------
std::vector<std::string> vtkGDALReader::GetDomainMetaData(
  const std::string& domain)
{
  std::vector<std::string> domainMetaData;

  char** papszMetadata =
    GDALGetMetadata(this->Implementation->GDALData, domain.c_str());

  if (CSLCount(papszMetadata) > 0)
    {
    for (int i = 0; papszMetadata[i] != NULL; ++i)
      {
      domainMetaData.push_back(papszMetadata[i]);
      }
    }

  return domainMetaData;
}

//-----------------------------------------------------------------------------
const std::string& vtkGDALReader::GetDriverShortName()
{
  return this->DriverShortName;
}

//-----------------------------------------------------------------------------
const std::string& vtkGDALReader::GetDriverLongName()
{
  return this->DriverLongName;
}

#ifdef _MSC_VER
#define strdup _strdup
#endif

//-----------------------------------------------------------------------------
int vtkGDALReader::RequestData(vtkInformation* vtkNotUsed(request),
                               vtkInformationVector** vtkNotUsed(inputVector),
                               vtkInformationVector* outputVector)
{
  if (this->TargetDimensions[0] <= 0 || this->TargetDimensions[1] <= 0)
    {
    vtkWarningMacro( << "Invalid target dimensions")
    }

  this->Implementation->ReadData(this->FileName);
  if (!this->Implementation->GDALData)
    {
    vtkErrorMacro("Failed to read "  << this->FileName);
    return 0;
    }

  // Get the projection.
  char* proj = strdup(this->Implementation->GDALData->GetProjectionRef());
  OGRSpatialReference spRef(proj);

  char* projection;
  spRef.exportToProj4(&projection);
  this->Projection = projection;
  CPLFree(projection);

  // Check if file has been changed here.
  // If changed then throw the vtxId time and load a new one.
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if (!outInfo)
    {
    return 0;
    }

  vtkDataObject* dataObj = outInfo->Get(vtkDataObject::DATA_OBJECT());
  if (!dataObj)
    {
    return 0;
    }

  vtkImageData::SafeDownCast(dataObj)->ShallowCopy(this->Implementation->ImageData);
  return 1;
}

//-----------------------------------------------------------------------------
int vtkGDALReader::RequestInformation(vtkInformation * vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if (!outInfo)
    {
    vtkErrorMacro("Invalid output information object");
    return 0;
    }

  if (!this->FileName)
    {
    vtkErrorMacro("Requires valid input file name") ;
    return 0;
    }

  this->Implementation->ReadMetaData(this->FileName);
  if (!this->Implementation->GDALData)
    {
    vtkErrorMacro("Failed to read "  << this->FileName);
    return 0;
    }

  if (this->RasterDimensions[0] <= 0 &&
      this->RasterDimensions[1] <= 0)
    {
    vtkErrorMacro("Invalid image dimensions");
    return 0;
    }

  if (this->TargetDimensions[0] == -1 &&
      this->TargetDimensions[1] == -1)
    {
    this->TargetDimensions[0] = this->RasterDimensions[0];
    this->TargetDimensions[1] = this->RasterDimensions[1];
    }

  if (this->DataExtent[0] == -1)
    {
    this->DataExtent[0] = 0;
    this->DataExtent[1] = this->RasterDimensions[0] - 1;
    this->DataExtent[2] = 0;
    this->DataExtent[3] = this->RasterDimensions[1] - 1;
    this->DataExtent[4] = 0;
    this->DataExtent[5] = 0;
    }

  // GDAL top left is at 0,0
  this->Implementation->SourceOffset[0] = this->DataExtent[0];
  this->Implementation->SourceOffset[1] = this->RasterDimensions[1] -
                                          (this->DataExtent[3] + 1);

  this->Implementation->SourceDimensions[0] =
    this->DataExtent[1] - this->DataExtent[0] + 1;
  this->Implementation->SourceDimensions[1] =
    this->DataExtent[3] - this->DataExtent[2] + 1;

  // Clamp pixel offset and image dimension
  this->Implementation->SourceOffset[0] =
    Min(this->RasterDimensions[0], this->Implementation->SourceOffset[0]);
  this->Implementation->SourceOffset[0] =
    Max(0.0, this->Implementation->SourceOffset[0]);

  this->Implementation->SourceOffset[1] =
    Min(this->RasterDimensions[1], this->Implementation->SourceOffset[1]);
  this->Implementation->SourceOffset[1] =
    Max(0.0, this->Implementation->SourceOffset[1]);

  this->Implementation->SourceDimensions[0] =
    Max(0.0, this->Implementation->SourceDimensions[0]);
  if ((this->Implementation->SourceDimensions[0] +
       this->Implementation->SourceOffset[0]) >
      this->RasterDimensions[0])
    {
    this->Implementation->SourceDimensions[0] =
      this->RasterDimensions[0] - this->Implementation->SourceOffset[0];
    }

  this->Implementation->SourceDimensions[1] =
    Max(0.0, this->Implementation->SourceDimensions[1]);
  if ((this->Implementation->SourceDimensions[1] +
       this->Implementation->SourceOffset[1]) >
      this->RasterDimensions[1])
    {
    this->Implementation->SourceDimensions[1] =
      this->RasterDimensions[1] - this->Implementation->SourceOffset[1];
    }

  this->DataExtent[0] = this->Implementation->SourceOffset[0];
  this->DataExtent[1] = this->DataExtent[0] +
                        this->Implementation->SourceDimensions[0] - 1;
  this->DataExtent[3] = this->RasterDimensions[1] -
                        this->Implementation->SourceOffset[1] - 1;
  this->DataExtent[2] = this->DataExtent[3] -
                        this->Implementation->SourceDimensions[1] + 1;
  this->DataExtent[4] = 0;
  this->DataExtent[5] = 0;

  this->DataSpacing[0] =
    static_cast<double>(this->Implementation->SourceDimensions[0]) /
    this->TargetDimensions[0];
  this->DataSpacing[1] =
    static_cast<double>(this->Implementation->SourceDimensions[1]) /
    this->TargetDimensions[1];
  this->DataSpacing[2] = 1.0;

  this->DataOrigin[0] = this->DataExtent[0];
  this->DataOrigin[1] = this->DataExtent[2];

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               this->DataExtent, 6);
  outInfo->Set(vtkDataObject::SPACING(), this->DataSpacing, 3);
  outInfo->Set(vtkDataObject::ORIGIN(), this->DataOrigin, 3);

  return 1;
}

//-----------------------------------------------------------------------------
int vtkGDALReader::FillOutputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
    {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
    return 1;
    }
  else
    {
    vtkErrorMacro("Port: " << port << " is not a valid port");
    return 0;
    }
}
