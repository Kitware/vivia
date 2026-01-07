// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
