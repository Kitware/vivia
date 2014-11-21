/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgVideoFrameMetaData_h
#define __vtkVgVideoFrameMetaData_h

#include <vtkImageData.h>

#include <vgExport.h>

#include "vtkVgInstance.h"
#include "vtkVgTimeStamp.h"
#include "vtkVgVideoFrameCorners.h"

//-----------------------------------------------------------------------------
struct VTKVG_CORE_EXPORT vtkVgVideoFrameMetaData
{
  vtkVgVideoFrameMetaData();

  vtkVgPythonAttribute(double, Gsd);
  vtkVgPythonAttribute(vtkVgTimeStamp, Time);
  vtkVgPythonAttribute(vtkVgVideoFrameCorners, WorldLocation);

  vtkVgInstance<vtkMatrix4x4> Homography;
  vtkVgPythonAttribute(int, HomographyReferenceFrame);

  vtkVgPythonAttribute(int, Width);
  vtkVgPythonAttribute(int, Height);

  bool AreCornerPointsValid() const;
  vtkSmartPointer<vtkMatrix4x4> MakeImageToLatLonMatrix() const;

  void SetWidthAndHeight(int width, int height)
    {
    this->Width = width;
    this->Height = height;
    }

  // Accessors for Python
  vtkMatrix4x4* GetHomography() { return this->Homography; }
  vtkMatrix4x4 const* GetHomography() const { return this->Homography; }
};

#endif
