// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgMultiResJpgImageReader2.h"

#include <vgUtil.h>

#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkByteSwap.h"
#include "vtkImageData.h"
#include "vtkImageShrink3D.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkImageReader2.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "vtksys/SystemTools.hxx"

#include "vtkVgJPEGMemoryReader.h"

vtkStandardNewMacro(vtkVgMultiResJpgImageReader2);

//----------------------------------------------------------------------------
vtkVgMultiResJpgImageReader2::vtkVgMultiResJpgImageReader2() :
  vtkVgBaseImageSource()
{
  this->ExactExtent = 1;
//  this->FilePattern = 0;
  this->Scale = 1.0;
  this->Frame = 0;
  this->NumberOfLevels = 0;
  this->Dimensions[0] = this->Dimensions[1] = 0;

  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkVgMultiResJpgImageReader2::~vtkVgMultiResJpgImageReader2()
{
//  this->SetFilePattern(0);
}

//----------------------------------------------------------------------------
void vtkVgMultiResJpgImageReader2::PrintSelf(ostream& os, vtkIndent indent)
{
//  os << indent << "FilePattern: " << this->FilePattern << endl;
  os << indent << "ExactExtent: " << this->ExactExtent << endl;
}

//----------------------------------------------------------------------------
void vtkVgMultiResJpgImageReader2::GetDimensions(int dims[2])
{
  dims[0] = this->Dimensions[0];
  dims[1] = this->Dimensions[1];
}

//----------------------------------------------------------------------------
void vtkVgMultiResJpgImageReader2::GetTileDimensions(int dims[2])
{
  dims[0] = this->TileDimensions[0];
  dims[1] = this->TileDimensions[1];
}

//----------------------------------------------------------------------------
void vtkVgMultiResJpgImageReader2::ComputeLevel()
{
  double level = log(this->Scale) / log(2.0);

  // Delay going to a higher level by a bit.
  level = level * 1.1;

  if (this->Level == -1)
    {
    this->Level = (int)(level);
    }
  if (this->Level < 0)
    {
    this->Level = 0;
    }
  if (this->Level >= this->NumberOfLevels)
    {
    this->Level = this->NumberOfLevels - 1;
    }
}

//----------------------------------------------------------------------------
int vtkVgMultiResJpgImageReader2::GetNumberOfLevels() const
{
  return static_cast<int>(this->NumberOfLevels);
}

//----------------------------------------------------------------------------
bool vtkVgMultiResJpgImageReader2::CanRead(const std::string& source) const
{
  std::string ciSource = vtksys::SystemTools::LowerCase(source);

  // Check if it is a filename or if the argument is just an extension.
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(ciSource);
  if ((ext.compare(".mrj") == 0) || (ciSource.compare("mrj") == 0) ||
      (ciSource.compare(".mrj") == 0))
    {
    return true;
    }

  return false;
}

//----------------------------------------------------------------------------
std::string vtkVgMultiResJpgImageReader2::GetShortDescription() const
{
  return "Reader for MRJ file format";
}

//----------------------------------------------------------------------------
std::string vtkVgMultiResJpgImageReader2::GetLongDescription() const
{
  return this->GetShortDescription();
}

//----------------------------------------------------------------------------
// This method returns the largest data that can be generated.
int vtkVgMultiResJpgImageReader2::RequestInformation(
  vtkInformation*        vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
{
//  char* fileName = new char[strlen(this->FilePattern)+1000];
//  sprintf(fileName, this->FilePattern, this->Frame);
//  this->SetFileName(fileName);
//  delete [] fileName;
  FILE* fp;
#ifdef _WIN32
  fopen_s(&fp, this->FileName, "rb");
#else
  fp = fopen(this->FileName, "rb");
#endif
  if (! fp)
    {
    cerr << "File " << this->FileName << " does not exist.\n";
    return -1.0;
    }

  // Skip the class name.  Do not bother verifying class name yet.
  fseek(fp, 30, SEEK_SET);
  // read to location of the level table.
  fread((void*)(&this->LevelTableLocation), sizeof(vtkTypeUInt32), 1, fp);
  vtkByteSwap::Swap4LE(&this->LevelTableLocation);

  // Read the meta data.
  fread((void*)(this->Dimensions), sizeof(vtkTypeUInt32), 2, fp);
  vtkByteSwap::Swap4LERange(this->Dimensions, 2);

  fread((void*)(&this->TileDimensions), sizeof(vtkTypeUInt32), 2, fp);
  vtkByteSwap::Swap4LERange(this->TileDimensions, 2);

  unsigned char numScalarComponents;
  fread((void*)(&numScalarComponents), sizeof(unsigned char), 1, fp);

  unsigned char numLevels;
  fread((void*)(&numLevels), sizeof(unsigned char), 1, fp);
  this->NumberOfLevels = numLevels;
  this->ComputeLevel();

  // Skip any other meta data
  fclose(fp);

  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  int ext[6];
  ext[4] = ext[5] = 0;
  // Copy the extent from the request.
  ext[0] = this->ReadExtents[0];
  ext[1] = this->ReadExtents[1];
  ext[2] = this->ReadExtents[2];
  ext[3] = this->ReadExtents[3];

  if (ext[0] == -1 && ext[1] == -1 && ext[2] == -1 && ext[3] == -1)
    {
    ext[0] = ext[2] = 0;
    ext[1] = this->Dimensions[0] - 1;
    ext[3] = this->Dimensions[1] - 1;
    }
  else if (ext[0] >= static_cast<int>(this->Dimensions[0]) || ext[1] < 0 ||
           ext[2] >= static_cast<int>(this->Dimensions[1]) || ext[3] < 0)
    {
    // Extent does not overlap image; set the information to indicate that
    // there is nothing to do
    ext[0] = ext[2] = ext[4] = -1;
    ext[1] = ext[3] = ext[5] = -1;
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext, 6);

    int scalarType = VTK_UNSIGNED_CHAR;
    vtkDataObject::SetPointDataActiveScalarInfo(outInfo, scalarType, numScalarComponents);

    return 1;
    }
  else
    {
    // Crop with the image extent in the file.
    vgTruncateLowerBoundary(ext[0], 0);
    vgTruncateLowerBoundary(ext[2], 0);
    vgTruncateUpperBoundary(ext[1], static_cast<int>(this->Dimensions[0]) - 1);
    vgTruncateUpperBoundary(ext[3], static_cast<int>(this->Dimensions[1]) - 1);
    }

  // Change the resolution to the correct level
  if (this->Level > 0)
    {
    const int k = 1 << this->Level;
    ext[0] /= k;
    ext[2] /= k;
    ext[1] = ((ext[1] + 1) / k) - 1;
    ext[3] = ((ext[3] + 1) / k) - 1;

    // Ensure reduction doesn't reduce dimensions to 0
    vgExpandUpperBoundary(ext[1], 0);
    vgExpandUpperBoundary(ext[3], 0);
    vgExpandLowerBoundary(ext[0], ext[1]);
    vgExpandLowerBoundary(ext[2], ext[3]);
    }
  if (!this->ExactExtent)
    {
    // Extend extent to cover whole tiles
    ext[0] = (ext[0] / this->TileDimensions[0]) * this->TileDimensions[0];
    ext[1] = ((ext[1] / this->TileDimensions[0]) + 1) * this->TileDimensions[0] - 1;
    ext[2] = (ext[2] / this->TileDimensions[1]) * this->TileDimensions[1];
    ext[3] = ((ext[3] / this->TileDimensions[1]) + 1) * this->TileDimensions[1] - 1;
    }

  double spacing[3];
  spacing[0] = static_cast<int>(this->Spacing[0]) << this->Level;
  spacing[1] = static_cast<int>(this->Spacing[1]) << this->Level;
  spacing[2] = static_cast<int>(this->Spacing[2]) << this->Level;

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext, 6);
  outInfo->Set(vtkDataObject::SPACING(), spacing, 3);
  outInfo->Set(vtkDataObject::ORIGIN(),  this->Origin, 3);

  int scalarType = VTK_UNSIGNED_CHAR;
  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, scalarType, numScalarComponents);
  //fclose(fp);

  return 1;
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
void vtkVgMultiResJpgImageReader2::ExecuteDataWithInformation(
  vtkDataObject* output, vtkInformation* outInfo)
{
  vtkImageData* image = vtkImageData::SafeDownCast(output);
  if (image == 0)
    {
    vtkErrorMacro("Expecting an image.");
    return;
    }

  int* const outExt = this->GetUpdateExtent();
  if (outExt[0] < 0 || outExt[1] < 0 || outExt[2] < 0 || outExt[3] < 0)
    {
    // Any negative output extents are not valid... also, RequestInformation
    // sets all extents to -1 when there is nothing to do, so we want to catch
    // that and bail out early
    return;
    }

  vtkSmartPointer<vtkVgJPEGMemoryReader> reader = vtkSmartPointer<vtkVgJPEGMemoryReader>::New();

  FILE* fp;
#ifdef _WIN32
  fopen_s(&fp, this->FileName, "rb");
#else
  fp = fopen(this->FileName, "rb");
#endif
  if (! fp)
    {
    cerr << "File " << this->FileName << " does not exist.\n";
    return;
    }

  // Skip the metadata and go directly to the level table.
  // Read the location of the level we are reading.
  fseek(fp, this->LevelTableLocation + (sizeof(vtkTypeUInt64)*this->Level), SEEK_SET);
  vtkTypeUInt64 levelLocation;
  fread((void*)(&levelLocation), sizeof(vtkTypeUInt64), 1, fp);
  vtkByteSwap::Swap8LE(&levelLocation);
  fseek(fp, levelLocation, SEEK_SET);

  // Read level metadata.
  vtkTypeUInt32 gridDims[2];
  fread((void*)(gridDims), sizeof(vtkTypeUInt32), 2, fp);
  vtkByteSwap::Swap8LERange(gridDims, 2);

  // Read the whole tileTable.
  size_t tableLength = (gridDims[0] * gridDims[1]) + 1;
  vtkTypeUInt64* tileTable = new vtkTypeUInt64[tableLength];
  fread((void*)(tileTable), sizeof(vtkTypeUInt64), tableLength, fp);
  vtkByteSwap::Swap8LERange(tileTable, tableLength);

  // Figure out which tiles to load.
  image->SetExtent(outExt);
  image->AllocateScalars(outInfo);

  int tileExt[6];
  size_t gridExt[4];
  // Grid in a level.
  gridExt[0] = outExt[0] / this->TileDimensions[0];
  gridExt[1] = outExt[1] / this->TileDimensions[0];
  gridExt[2] = outExt[2] / this->TileDimensions[1];
  gridExt[3] = outExt[3] / this->TileDimensions[1];

  // Guess at an initial buffer size that seems to work in most cases... this
  // assumes an average compression factor of about C:1 where C is the number
  // of components in the image (i.e. usually around 65% for RGB)
  size_t tileBufLength =
    (this->TileDimensions[0] * this->TileDimensions[1]) + 256;
  unsigned char* tileBuf = new unsigned char[tileBufLength];

  // The output is already allocated.
  // Loop through all the tiles that we are going to load.
  size_t yTileIdx, tileIdx;
  vtkTypeUInt64 tileLocation, tileLength;

  bool error = false;

  for (size_t y = gridExt[2]; y <= gridExt[3]; ++y)
    {
    yTileIdx = y * gridDims[0];
    for (size_t x = gridExt[0]; x <= gridExt[1]; ++x)
      {

      if (error)
        {
        break;
        }

      tileIdx = yTileIdx + x;

      if (tileIdx + 1 >= tableLength)
        {
        error = true;
        break;
        }

      tileLocation = tileTable[tileIdx];

      tileLength = tileTable[tileIdx + 1] - tileLocation;
      if (tileLength > tileBufLength)
        {
        // If the buffer is not large enough, reallocate a larger one that is
        // the needed size plus a bit of fudge factor
        tileBufLength = static_cast<size_t>(1.2 * tileLength);
        delete [] tileBuf;
        tileBuf = new unsigned char[tileBufLength];
        }

      fseek(fp, tileLocation, SEEK_SET);
      fread((void*)(tileBuf), 1, tileLength, fp);

      // Decompress the tile.
      reader->SetMemoryBuffer(tileBuf, tileLength);
      reader->Update();
      // Shift image so we can use CopyAndCast.
      reader->GetOutput()->GetExtent(tileExt);
      const int xOffset = x * this->TileDimensions[0];
      tileExt[0] += xOffset;
      tileExt[1] += xOffset;
      const int yOffset = y * this->TileDimensions[1];
      tileExt[2] += yOffset;
      tileExt[3] += yOffset;
      reader->GetOutput()->SetExtent(tileExt);
      // Take the intersection of the tile and output.
      vgTruncateLowerBoundary(tileExt[0], outExt[0]);
      vgTruncateUpperBoundary(tileExt[1], outExt[1]);
      vgTruncateLowerBoundary(tileExt[2], outExt[2]);
      vgTruncateUpperBoundary(tileExt[3], outExt[3]);
      image->CopyAndCastFrom(reader->GetOutput(), tileExt);
      }
    }

  delete [] tileTable;
  delete [] tileBuf;
  fclose(fp);
}

//----------------------------------------------------------------------------
vtkVgBaseImageSource* vtkVgMultiResJpgImageReader2::Create()
{
  return vtkVgMultiResJpgImageReader2::New();
}
