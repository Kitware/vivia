/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
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

  // Currently we handle images with 1 - 3 components. (R, RG, RGB).
  if (np < 1 || np > 3)
    {
    qDebug() << "vtkVgAdapt<vgImage>: failed to convert image with"
             << np << "planes; only 1-, 2- or 3-plane images are supported";
    return 0;
    }

  if (w == 0 || h == 0)
    {
    qDebug()
      << "vtkVgAdapt<vgImage>: failed to convert image with dimension 0";
    return 0;
    }

  // Assume vgImage uses 8-bit pixels
  const int pdt = VTK_UNSIGNED_CHAR, pds = 1;

  vtkSmartPointer<vtkImageAppendComponents>
    components(vtkSmartPointer<vtkImageAppendComponents>::New());
  components->SetNumberOfThreads(1);
  const int ps = image.planeStep();

  // Get data pointer... do this up front instead of when needed because VTK is
  // not const-safe, and so we must get rid of const qualifier on the pointer
  unsigned char* const imageData =
    const_cast<unsigned char*>(image.constData());

  if (ps > pds)
    {
    // Image data is packed RRRR...RR[GGGG...GG[BBBB...BB]], so we have to
    // import each channel independently
    vtkSmartPointer<vtkImageImport> importerRed(0);
    vtkSmartPointer<vtkImageImport> importerGreen(0);
    vtkSmartPointer<vtkImageImport> importerBlue(0);

    importerRed = vtkSmartPointer<vtkImageImport>::New();
    importerRed->SetDataScalarType(pdt);
    importerRed->SetNumberOfScalarComponents(1);
    importerRed->SetImportVoidPointer(imageData + (0 * ps));
    importerRed->SetWholeExtent(0, w - 1, 0, h - 1, 0, 0);
    importerRed->SetDataExtentToWholeExtent();
    importerRed->Modified();
    importerRed->Update();

    if (np > 1)
      {
      importerGreen = vtkSmartPointer<vtkImageImport>::New();
      importerGreen->SetDataScalarType(pdt);
      importerGreen->SetNumberOfScalarComponents(1);
      importerGreen->SetImportVoidPointer(imageData + (1 * ps));
      importerGreen->SetWholeExtent(0, w - 1, 0, h - 1, 0, 0);
      importerGreen->SetDataExtentToWholeExtent();
      importerGreen->Modified();
      importerGreen->Update();
      }

    if (np > 2)
      {
      importerBlue = vtkSmartPointer<vtkImageImport>::New();
      importerBlue->SetDataScalarType(pdt);
      importerBlue->SetNumberOfScalarComponents(1);
      importerBlue->SetImportVoidPointer(imageData + (2 * ps));
      importerBlue->SetWholeExtent(0, w - 1, 0, h - 1, 0, 0);
      importerBlue->SetDataExtentToWholeExtent();
      importerBlue->Modified();
      importerBlue->Update();
      }

    components->SetInputConnection(importerRed->GetOutputPort());
    if (importerGreen)
      {
      components->AddInputConnection(importerGreen->GetOutputPort());
      if (importerBlue)
        {
        components->AddInputConnection(importerBlue->GetOutputPort());
        }
      }

    }
  else if (ps == 1 && image.iStep() == np && image.jStep() == w * np)
    {
    // Image data is packed R[G[B]]R[G[B]]...R[G[B]], so we can import all
    // channels directly
    vtkSmartPointer<vtkImageImport> importer =
      vtkSmartPointer<vtkImageImport>::New();
    importer->SetDataScalarType(pdt);
    importer->SetNumberOfScalarComponents(np);
    importer->SetImportVoidPointer(imageData + (0 * ps));
    importer->SetWholeExtent(0, w - 1, 0, h - 1, 0, 0);
    importer->SetDataExtentToWholeExtent();
    importer->Modified();
    importer->Update();

    components->SetInputConnection(importer->GetOutputPort());
    }
  else
    {
    // Image data is plane-interlaced, but either has additional padding and/or
    // the channels are stored in memory backward (ps < 0); e.g. 32-bit pixels
    // in BGR order with an unused byte
    vtkSmartPointer<vtkImageData> vtkimage =
      vtkSmartPointer<vtkImageData>::New();
    vtkimage->SetExtent(0, w - 1, 0, h - 1, 0, 0);
    vtkimage->SetSpacing(1.0, 1.0, 1.0);
    vtkimage->AllocateScalars(VTK_UNSIGNED_CHAR, np);

    unsigned char* vtkimagePtr =
      static_cast<unsigned char*>(vtkimage->GetScalarPointer());

    const int iStep = image.iStep();
    const int jStep = image.jStep();

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
        const unsigned char r = *(pixoff + (0 * ps));
        const unsigned char g = *(pixoff + (1 * ps));
        const unsigned char b = *(pixoff + (2 * ps));

        const ptrdiff_t xo = (yod + x) * 3;
        vtkimagePtr[xo + 0] = r;
        vtkimagePtr[xo + 1] = g;
        vtkimagePtr[xo + 2] = b;
        }
      }

      return vtkimage;
    }

  components->Update();

  vtkSmartPointer<vtkImageFlip> flip(vtkSmartPointer<vtkImageFlip>::New());
  flip->SetInputConnection(components->GetOutputPort());
  flip->SetFlipAboutOrigin(false);
  flip->SetFilteredAxes(1);
  flip->SetNumberOfThreads(1);
  flip->Update();

  vtkSmartPointer<vtkImageData> img = vtkSmartPointer<vtkImageData>::New();
  img->ShallowCopy(flip->GetOutput());
  return img;
}
