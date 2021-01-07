// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvPowerPointSlideGenerator_h
#define __vvPowerPointSlideGenerator_h

#include "vvPowerPointWriter.h"

#include <vgVideoRequestor.h>

#include <vtkVgTimeStamp.h>

#include <vgExport.h>

class QProgressDialog;

class VV_VTKWIDGETS_EXPORT vvPowerPointSlideGenerator
{
public:
  vvPowerPointSlideGenerator() {}
  virtual ~vvPowerPointSlideGenerator() {}

  virtual bool generateSlide(
    vvPowerPointWriter& writer, vgVideoRequestor& videoRequestor, int outputId,
    bool generateVideo, QProgressDialog* progress, int& progressCount);

  virtual int outputFrameCount(bool generateVideo)
    {
    Q_UNUSED(generateVideo)
    return 0;
    }

  void setVideoStartFrame(vtkVgTimeStamp startFrame)
    { this->VideoStartFrame = startFrame; }
  vtkVgTimeStamp videoStartFrame() { return this->VideoStartFrame; }

  void setVideoEndFrame(vtkVgTimeStamp endFrame)
    { this->VideoEndFrame = endFrame; }
  vtkVgTimeStamp videoEndFrame() { return this->VideoEndFrame; }

  void setRequestedStillFrame(vtkVgTimeStamp requestedStillFrame)
    { this->RequestedStillFrame = requestedStillFrame; }
  vtkVgTimeStamp requestedStillFrame() { return this->RequestedStillFrame; }

protected:
  struct ImageInfo
    {
    explicit ImageInfo(const vtkMatrix4x4* transform = 0)
      : Data(0), Transform(transform) {}

    vtkImageData* Data;
    vtkVgTimeStamp TimeStamp;
    const vtkMatrix4x4* Transform;
    };

  virtual void writeSummary(vvPowerPointWriter& writer,
                            const ImageInfo& imageInfo,
                            int outputId) = 0;
  virtual void writeVideoFrame(vvPowerPointWriter& writer,
                               const ImageInfo& imageInfo,
                               int outputId, int frameNumber) = 0;
  virtual void createVideo(vvPowerPointWriter& writer, int outputId) = 0;

  /// Invert input homography.
  ///
  /// This copies the inverse of \p in to \p out.
  static void invertHomography(vtkMatrix4x4* in, vtkMatrix4x4* out);

  /// Invert input homography and apply Y flip.
  ///
  /// This populates \p out with a homography that is the inverse of \p in with
  /// an additional operation to transmute the Y value of transformed
  /// coordinates so as to effect a vertical flip of a rectangle of \p height.
  /// This is normally used to convert between Y-up and Y-down image
  /// coordinates for an image of the specified \p height.
  static void invertHomography(vtkMatrix4x4* in, vtkMatrix4x4* out,
                               int height);

  static int numberOfOutputVideoFrames(vtkVgTimeStamp start,
                                       vtkVgTimeStamp end);

  vtkVgTimeStamp VideoStartFrame;
  vtkVgTimeStamp VideoEndFrame;
  vtkVgTimeStamp RequestedStillFrame;
};

#endif
