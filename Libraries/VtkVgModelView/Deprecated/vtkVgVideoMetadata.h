/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgVideoMetadata_h
#define __vtkVgVideoMetadata_h

#include <vtkVgInstance.h>
#include <vtkVgTimeStamp.h>

#include <vtkMatrix4x4.h>

struct vtkVgVideoMetadata
{
  vtkVgVideoMetadata() : Gsd(-1.0), HomographyReferenceFrame(-1) {}
  vtkVgVideoMetadata(vtkVgTimeStamp time, double gsd,
                     const vtkMatrix4x4& homography,
                     int homographyReferenceFrame)
    : Gsd(gsd), Time(time), Homography(homography),
      HomographyReferenceFrame(homographyReferenceFrame)
    {}

  double Gsd;
  vtkVgTimeStamp Time;
  vtkVgInstance<vtkMatrix4x4> Homography;
  int HomographyReferenceFrame;
};

#endif // __vtkVgVideoMetadata_h
