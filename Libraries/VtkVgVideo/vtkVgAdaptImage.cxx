/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgAdaptImage.h"

#include <vgImage.h>

#include <vtkVgImageBuffer.h>

#include <QDebug>

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkImageData> vtkVgAdapt(const vgImage& image)
{
  if (!image.isValid())
    {
    return {};
    }

  if (image.iCount() <= 0 || image.jCount() <= 0 || image.planeCount() <= 0)
    {
    qDebug() << "vtkVgAdapt<vgImage>: failed to convert image with"
                "non-positive dimensions"
             << image.iCount() << image.iCount() << image.planeCount();
    return {};
    }

  // Set up image buffer for conversion (assume vgImage uses 8-bit pixels)
  vtkVgImageBuffer buffer{VTK_UNSIGNED_CHAR};
  buffer.SetLayout(static_cast<size_t>(image.iCount()),
                   static_cast<size_t>(image.jCount()),
                   static_cast<size_t>(image.planeCount()),
                   image.iStep(), -image.jStep(), image.planeStep());
  buffer.SetData(reinterpret_cast<const char*>(image.constData()));
  return buffer.Convert();
}
