/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgMultiResJpgImageWriter2.h"
#include "vtkSmartPointer.h"
#include "vtkCommand.h"
#include "vtkErrorCode.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkImageData.h"
#include "vtkJPEGWriter.h"
#include "vtkImageShrink3D.h"
#include "vtkUnsignedCharArray.h"

#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkVgMultiResJpgImageWriter2);

//----------------------------------------------------------------------------
vtkVgMultiResJpgImageWriter2::vtkVgMultiResJpgImageWriter2()
{
  this->FileName = NULL;
  this->NumberOfLevels = 5;
  this->CompressionQuality = 30;
  this->TileDimensions[0] = 128;
  this->TileDimensions[1] = 128;

  this->SetNumberOfOutputPorts(0);
}



//----------------------------------------------------------------------------
vtkVgMultiResJpgImageWriter2::~vtkVgMultiResJpgImageWriter2()
{
  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = NULL;
    }
}


//----------------------------------------------------------------------------
void vtkVgMultiResJpgImageWriter2::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "FileName: " <<
     (this->FileName ? this->FileName : "(none)") << "\n";
}


//----------------------------------------------------------------------------
vtkImageData* vtkVgMultiResJpgImageWriter2::GetInput()
{
  if (this->GetNumberOfInputConnections(0) < 1)
    {
    return 0;
    }
  return vtkImageData::SafeDownCast(
           this->GetExecutive()->GetInputData(0, 0));
}

//----------------------------------------------------------------------------
int vtkVgMultiResJpgImageWriter2::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* vtkNotUsed(outputVector))
{
  this->SetErrorCode(vtkErrorCode::NoError);

  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkImageData* input =
    vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Error checking
  if (input == NULL)
    {
    vtkErrorMacro(<< "Write:Please specify an input!");
    return 0;
    }
  if (!this->FileName)
    {
    vtkErrorMacro(<< "Write:Please specify a FileName");
    this->SetErrorCode(vtkErrorCode::NoFileNameError);
    return 0;
    }

  // Fill in image information.
  int* wExt = inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());
  if (wExt[4] != wExt[5])
    {
    vtkErrorMacro("Two dimensional images only please.");
    return 0;
    }

  // Write
  this->InvokeEvent(vtkCommand::StartEvent);
  this->UpdateProgress(0.0);
  this->WriteImage(input);

  //if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError)
  //  {
  //  this->DeleteFiles();
  //  }

  this->UpdateProgress(1.0);
  this->InvokeEvent(vtkCommand::EndEvent);

  return 1;
}

//----------------------------------------------------------------------------
void vtkVgMultiResJpgImageWriter2::Write()
{
  this->Update();
  vtkImageData* input = this->GetInput();
  this->WriteImage(input);
}

//----------------------------------------------------------------------------
inline size_t fwriteLE(void* data, size_t size, size_t num, FILE* fp)
{
#ifdef VTK_WORDS_BIGENDIAN
  char* temp = new char[size * num];
  memcpy(temp, data, num * size);
  if (size == 4)
    {
    vtkByteSwap::Swap4LERange(temp, num);
    }
  if (size == 8)
    {
    vtkByteSwap::Swap8LERange(temp, num);
    }
  if (size == 2)
    {
    vtkByteSwap::Swap2LERange(temp, num);
    }
  return fwrite(temp, size, num, fp);
#else
  return fwrite(data, size, num, fp);
#endif
}

//----------------------------------------------------------------------------
// Writes all the data from the input.
// Modified from AngelFire format.
int vtkVgMultiResJpgImageWriter2::WriteImage(vtkImageData* input)
{
  // Header and points.
  // Table for starting location for each of the 6 levels.
  // For each level.
  // TileExtent.
  // Table for each grid location for offset and length of tile.
  FILE* fp;

  // TODO ERROR checking.
#ifdef _WIN32
  fopen_s(&fp, this->FileName, "wb");
#else
  fp = fopen(this->FileName, "wb");
#endif
  if (fp == 0)
    {
    vtkErrorMacro("Could not open file " << this->FileName);
    return VTK_ERROR;
    }
  int level;

  // The class name is new.
  // I am concerned with speed so I am going to allocate a fixed 25 characters
  // For the file type.  Terminating character would require a loop.
  fwrite("vtkMultiResJpgImageWriter22    ", 1, 30, fp);

  // For flexibility I will put the start of the level table here.
  // We can then have anysize header for metadata we want.
  // We can just keep adding to the list.
  // We might consider key value pairs for optional meta data.
  long levelTableLocationLocation = ftell(fp);
  fseek(fp, sizeof(vtkTypeUInt32), SEEK_CUR);

  // Create a dataset that has the ortho image dimensions for
  // the whole terrain.  This information allows the client to
  // place the tile texture map in the appropriate spot.
  int* wholeDimensions = input->GetDimensions();
  vtkTypeUInt32 tmp[2];
  tmp[0] = wholeDimensions[0];
  tmp[1] = wholeDimensions[1];
  fwriteLE((void*)tmp, sizeof(vtkTypeUInt32), 2, fp);
  tmp[0] = this->TileDimensions[0];
  tmp[1] = this->TileDimensions[1];
  fwriteLE((void*)tmp, sizeof(int), 2, fp);
  unsigned char numberOfScalarComponents = input->GetNumberOfScalarComponents();
  fwrite((void*)(&numberOfScalarComponents), 1, 1, fp);

  // I am skipping the points.  They were a good idea though.
  // Tile 3d world corner points were used to determine tile visibility.

  // Filter to shrink each level.
  vtkSmartPointer<vtkImageData> tmpImage = vtkSmartPointer<vtkImageData>::New();
  tmpImage->ShallowCopy(input);
  vtkSmartPointer<vtkImageShrink3D> shrink = vtkSmartPointer<vtkImageShrink3D>::New();
  shrink->SetShrinkFactors(2, 2, 1);
  shrink->SetInputData(tmpImage);

  // Create a dataset that has the number of levels as a single element.
  unsigned char numberOfLevels = this->NumberOfLevels;
  if (numberOfLevels > 10)
    {
    cerr << "Too many levels (" << numberOfLevels << ")\n";
    numberOfLevels = 10;
    }
  fwrite((void*)(&numberOfLevels), 1, 1, fp);

  vtkTypeUInt64 levelTable[10]; // hard coded to 10 levels.
  // Skip space for the tile table.
  // Save location to write later.
  vtkTypeUInt32 levelTableLocation = ftell(fp);
  fseek(fp, sizeof(vtkTypeUInt64)*numberOfLevels, SEEK_CUR);

  // Save each level.
  for (level = 0; level < numberOfLevels; ++level)
    {
    levelTable[level] = ftell(fp);
    if (this->WriteLevel(fp, tmpImage) == VTK_ERROR)
      {
      fclose(fp);
      return VTK_ERROR;
      }
    // Lets reduce the resolution after writing each level.
    shrink->Update();
    tmpImage->ShallowCopy(shrink->GetOutput());
    }

  // Go back and write the level table.
  fseek(fp, levelTableLocation, SEEK_SET);
  fwriteLE((void*)(levelTable), sizeof(vtkTypeUInt64), numberOfLevels, fp);
  // Go to the start of the file and record the starting location of the level table.
  fseek(fp, levelTableLocationLocation, SEEK_SET);
  fwriteLE((void*)(&levelTableLocation), sizeof(vtkTypeUInt32), 1, fp);

  fclose(fp);

  return VTK_OK;
}


//----------------------------------------------------------------------------
int vtkVgMultiResJpgImageWriter2::WriteLevel(FILE* fp, vtkImageData* input)
{
  // Angelfire had a tile grid extent.  I am just going to use dimensions.
  int* imageDims = input->GetDimensions();
  int gridDims[2];
  gridDims[0] = (int)(ceil((double)(imageDims[0]) / (double)(this->TileDimensions[0])));
  gridDims[1] = (int)(ceil((double)(imageDims[1]) / (double)(this->TileDimensions[1])));

  // Temporary image holds cropped tile.
  int croppedExt[6];
  croppedExt[4] = croppedExt[5] = 0;
  vtkSmartPointer<vtkImageData> croppedTile = vtkSmartPointer<vtkImageData>::New();
  croppedTile->SetExtent(0, this->TileDimensions[0]-1,
    0, this->TileDimensions[1]-1, 0, 0);
  croppedTile->AllocateScalars(VTK_UNSIGNED_CHAR, input->GetNumberOfScalarComponents());
  int* inputExt = input->GetExtent();

  // Temporary memory used to hold compressed tile.
  vtkUnsignedCharArray* jpgData;
  vtkTypeUInt64 compressedTileSize;

  // Writer used to compress the tile.
  vtkSmartPointer<vtkJPEGWriter> vtkW = vtkSmartPointer<vtkJPEGWriter>::New();
  vtkW->WriteToMemoryOn();
  //valgrind says this needs to be set to ensure vtkW is OK below
  //despite WriteToMemory. look at vtkJPEGWriter.cxx sprintf(this->InternalFileName,NULL,num)
  vtkW->SetFilePattern("Tile_%d");
  vtkW->SetInputData(croppedTile);
  vtkW->SetQuality(this->CompressionQuality);
  vtkW->ProgressiveOff();

  // Add the level meta data to the file.
  vtkTypeUInt64 numTiles = gridDims[0] * gridDims[1];
  // Angelfire had an extent,  I am using dimensions.
  fwriteLE((void*)(gridDims), sizeof(vtkTypeUInt32), 2, fp);
  // Allocate array for tile offsets.
  // Add one extra element to the table.  We use the difference between
  // two table elements to compute the length of the compressed tile.
  vtkTypeUInt64 tileTableLength = numTiles + 1;
  vtkTypeUInt64* tileTable = new vtkTypeUInt64[tileTableLength];
  vtkTypeUInt64 tileTableLocation = ftell(fp);
  // Skip the tile table to write later.
  fseek(fp, static_cast<long int>(sizeof(vtkTypeUInt64))*tileTableLength, SEEK_CUR);
  int tileCount = 0;
  tileTable[tileCount] = ftell(fp);

  // Loop through the tiles.  Save those that have data.
  int xPartial, yPartial;
  for (int y = 0; y < gridDims[1]; ++y)
    {
    croppedExt[2] = y * this->TileDimensions[1];
    croppedExt[3] = croppedExt[2] + this->TileDimensions[1] - 1;
    yPartial = 0;
    if (croppedExt[3] > inputExt[3])
      {
      yPartial = 1;
      croppedExt[3] = inputExt[3];
      }
    for (int x = 0; x < gridDims[0]; ++x)
      {
      // Crop the tile out of the input.
      croppedExt[0] = x * this->TileDimensions[0];
      croppedExt[1] = croppedExt[0] + this->TileDimensions[0] - 1;
      xPartial = 0;
      if (croppedExt[1] > inputExt[1])
        {
        xPartial = 1;
        croppedExt[1] = inputExt[1];
        }
      croppedTile->SetExtent(croppedExt);
      // For tiles that hang over the edge.  We could make them smaller or
      // pad with 0's.  Lets pad.
      if (yPartial || xPartial)
        {
        memset(croppedTile->GetScalarPointer(), 0,
               this->TileDimensions[0]*this->TileDimensions[1]);
        }
      croppedTile->CopyAndCastFrom(input, croppedExt);

      // Compress the tile.
      croppedTile->Modified();
      vtkW->Write();
      jpgData = vtkW->GetResult();
      compressedTileSize = jpgData->GetNumberOfTuples();
      fwriteLE((void*)(jpgData->GetVoidPointer(0)), 1, compressedTileSize, fp);
      tileTable[++tileCount] = ftell(fp);
      }
    }

  // Go back and write the table.
  // Go back and write the level table.
  vtkTypeUInt64 endLocation = ftell(fp);
  fseek(fp, tileTableLocation, SEEK_SET);
  fwriteLE((void*)(tileTable), sizeof(vtkTypeUInt64), tileTableLength, fp);
  fseek(fp, endLocation, SEEK_SET);

  delete [] tileTable;

  return VTK_OK;
}
