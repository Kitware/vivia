/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgAdaptImage.h"

#include <QDebug>

#include <vtkImageAppendComponents.h>
#include <vtkImageData.h>
#include <vtkImageFlip.h>
#include <vtkImageImport.h>

#include <vgImage.h>

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkImageData> vtkVgAdapt(const vgImage& image)
{
  if (!image.isValid())
    {
    return 0;
    }

  const int w  = image.iCount();
  const int h  = image.jCount();
  const int np = image.planeCount();

  if (w <= 0 || h <= 0 || np <= 0)
    {
    qDebug() << "vtkVgAdapt<vgImage>: failed to convert image with"
                "non-positive dimensions" << w << h << np;
    return 0;
    }

  // Allocate the output image (assume vgImage uses 8-bit pixels)
  auto vtkImage = vtkSmartPointer<vtkImageData>::New();
  vtkImage->SetExtent(0, w - 1, 0, h - 1, 0, 0);
  vtkImage->SetSpacing(1.0, 1.0, 1.0);
  vtkImage->AllocateScalars(VTK_UNSIGNED_CHAR, np);

  // Get pointers to input and output image data
  auto* const imageData = image.constData();
  auto* const vtkImagePtr =
      static_cast<unsigned char*>(vtkImage->GetScalarPointer());

  const int iStep = image.iStep();
  const int jStep = image.jStep();
  const int pStep = image.planeStep();

  if (pStep == 1 && iStep == np)
    {
    const ptrdiff_t scanlineWidth = w * np;
    // Image data is packed R[G[B]]R[G[B]]...R[G[B]], so we can import data by
    // scanlines
    int y = image.jCount();
    while (y--)
      {
      // Compute source and destination scanline offsets; destination offset
      // is flipped over Y axis because vtkImageData needs to be Y-up, but
      // source is Y-down
      const ptrdiff_t yos = y * jStep;
      const ptrdiff_t yod = (h - y - 1) * scanlineWidth;
      memcpy(vtkImagePtr + yod, imageData + yos, scanlineWidth);
      }
    }
  else
    {
    // Image data is packed in some other manner; import it the hard way, one
    // byte at a time
    int y = image.jCount();
    while (y--)
      {
      int x = image.iCount();
      // Compute source and destination scanline offsets; destination offset
      // is flipped over Y axis because vtkImageData needs to be Y-up, but
      // source is Y-down
      const ptrdiff_t yos = y * jStep;
      const ptrdiff_t yod = (h - y - 1) * w;
      while (x--)
        {
        const unsigned char* const pixoff = imageData + (x * iStep) + yos;
        const ptrdiff_t xo = (yod + x) * np;
        for (ptrdiff_t p = 0; p < np; ++p)
          {
          vtkImagePtr[xo + p] = *(pixoff + (p * pStep));
          }
        }
      }
    }

  return vtkImage;
}
