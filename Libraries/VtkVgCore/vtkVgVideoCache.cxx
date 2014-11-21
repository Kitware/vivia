/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// VTK includes.
#include "vtkVgVideoCache.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkXMLImageDataReader.h"
#include "vtkPointData.h"
#include "vtkImageClip.h"

// Standard VTK macro.
vtkStandardNewMacro(vtkVgVideoCache);


//----------------------------------------------------------------------------
vtkVgVideoCache::vtkVgVideoCache()
{
  this->HasData = 0;
  for (int i = 0; i < 6; ++i)
    {
    this->WholeExtent[i] = 0;
    }
  for (int i = 0; i < 6; ++i)
    {
    this->Extent[i] = 0;
    }
  this->ImageData = NULL;
  this->CacheReader = NULL;
  this->Resolution = 0;
}

//----------------------------------------------------------------------------
vtkVgVideoCache::~vtkVgVideoCache()
{
}

//----------------------------------------------------------------------------
void vtkVgVideoCache::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  // @TODO: Implement this.
}

//----------------------------------------------------------------------------
double vtkVgVideoCache::GetResolution()
{
  return this->Resolution;
}

//----------------------------------------------------------------------------
int vtkVgVideoCache::ContainsRequest(unsigned int e[6])
{
  if (!this->HasData)
    {
    return 0;
    }

  for (int i = 0; i < 3; ++i)
    {
    if (e[2 * i] < this->Extent[2 * i] || e[2 * i + 1] > this->Extent[2 * i + 1])
      {
      return 0;
      }
    }

  return 1;
}


//----------------------------------------------------------------------------
void vtkVgVideoCache::
GetData(unsigned int e[6], vtkSmartPointer<vtkImageData> image)
{
  // If no data then configure the image accordingly.
  if (!this->HasData || this->ImageData == NULL)
    {
    image->SetWholeExtent(0, 0, 0, 0, 0, 0);
    image->SetExtent(0, 0, 0, 0, 0, 0);
    image->GetPointData()->SetScalars(NULL);
    return;
    }

  // Okay we assume the request e[6] is in the cache (since we should have checked).
  // We need to set up the image appropriately.
  unsigned int cE[6];
  vtkSmartPointer<vtkImageClip> clip = vtkSmartPointer<vtkImageClip>::New();
  clip->SetInputData(this->ImageData);
  this->ConvertWholeExtentsToCacheExtents(e, cE);
  clip->SetOutputWholeExtent(cE[0], cE[1], cE[2], cE[3], cE[4], cE[5]);
  clip->ClipDataOn();
  clip->Update();
  image->ShallowCopy(clip->GetOutput());

  return;
}


//----------------------------------------------------------------------------
void vtkVgVideoCache::RequestData(unsigned int vtkNotUsed(e)[6])
{
  //The cache needs to talk to the reader
}

//----------------------------------------------------------------------------
void vtkVgVideoCache::LoadData(unsigned int vtkNotUsed(e)[6],
                               vtkSmartPointer<vtkImageData> vtkNotUsed(image))
{
  //The reader should be invoking this method in response to a RequestData()
}

//----------------------------------------------------------------------------
void vtkVgVideoCache::LoadCacheFromFile(char* file,
  int wholeExtentFlag, unsigned int e[6])
{
  // First thing to do is clean out the existing data
  this->ImageData = vtkSmartPointer<vtkImageData>::New();

  // Okay read the data
  this->CacheReader = vtkSmartPointer<vtkXMLImageDataReader>::New();
  this->CacheReader->SetFileName(file);
  this->CacheReader->Update();

  // If successful read
  this->HasData = 1;
  this->ImageData = this->CacheReader->GetOutput();
  this->CacheReader = NULL;

  // If the wholeExtent flag is set then this cache data covers
  // the whole data extent otherwise it satisfies the request e[6].
  if (wholeExtentFlag)
    {
    for (int i = 0; i < 6; ++i)
      {
      this->Extent[i] = this->WholeExtent[i];
      }
    }
  else
    {
    int i;
    for (i = 0; i < 6; ++i)
      {
      this->Extent[i] = e[i];
      }
    for (i = 0; i < 3; ++i) //crop to whole extent
      {
      if (this->Extent[2 * i] < this->WholeExtent[2 * i])
        {
        this->Extent[2 * i] = this->WholeExtent[2 * i];
        }
      if (this->Extent[2 * i + 1] > this->WholeExtent[2 * i + 1])
        {
        this->Extent[2 * i + 1] = this->WholeExtent[2 * i + 1];
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkVgVideoCache::
ConvertWholeExtentsToCacheExtents(unsigned int wE[6], unsigned int cE[6])
{
  // note that the extent should be contained in the cache

  // Get the size of the cache (i.e., dimensions)
  int cExtent[6], cDims[3];
  this->ImageData->GetDimensions(cDims);
  this->ImageData->GetExtent(cExtent);

  // Perform the coordinate conversion. Pad out a tad.
  for (int i = 0; i < 3; ++i)
    {
    cE[2 * i] = cExtent[2 * i] + cDims[i]
      * static_cast<double>(wE[2 * i] - this->Extent[2 * i] + 1)
      / (this->Extent[2 * i + 1] - this->Extent[2 * i] + 1) - 1;
    cE[2 * i + 1] = cExtent[2 * i] + cDims[i]
      * static_cast<double>(wE[2 * i + 1] - this->Extent[2 * i] + 1)
      / (this->Extent[2 * i + 1] - this->Extent[2 * i] + 1) + 1;

    if (cE[2 * i] < static_cast<unsigned int>(cExtent[2 * i]))
      {
      cE[2 * i] = cExtent[2 * i];
      }
    if (cE[2 * i + 1] > static_cast<unsigned int>(cExtent[2 * i + 1]))
      {
      cE[2 * i + 1] = cExtent[2 * i + 1];
      }
    }

}
