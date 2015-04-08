/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvPowerPointTrackSlideGenerator_h
#define __vvPowerPointTrackSlideGenerator_h

#include "vvPowerPointSlideGenerator.h"

#include <qtGlobal.h>

#include <vtkSmartPointer.h>

class vtkVgTrack;

class VV_VTKWIDGETS_EXPORT vvPowerPointTrackSlideGenerator :
  public vvPowerPointSlideGenerator
{
public:
  vvPowerPointTrackSlideGenerator(vtkVgTrack* track);
  virtual ~vvPowerPointTrackSlideGenerator();

  virtual bool generateSlide(
    vvPowerPointWriter& writer, vgVideoRequestor& videoRequestor, int outputId,
    bool generateVideo, QProgressDialog* progress, int& progressCount)
    QTE_OVERRIDE;

  virtual int outputFrameCount(bool generateVideo) QTE_OVERRIDE;

  vtkVgTrack* track() { return this->Track; }

  static int addTrack(vvPowerPointWriter& writer, vtkVgTrack* track,
                      vtkVgTimeStamp startFrame, vtkVgTimeStamp endFrame,
                      const vtkMatrix4x4* transform,
                      const char* itemTemplateName, int imageId);

protected:
  virtual void writeSummary(vvPowerPointWriter& writer,
                            const ImageInfo& imageInfo,
                            int outputId) QTE_OVERRIDE;
  virtual void writeVideoFrame(vvPowerPointWriter& writer,
                               const ImageInfo& imageInfo,
                               int outputId, int frameNumber) QTE_OVERRIDE;
  virtual void createVideo(vvPowerPointWriter& writer,
                           int outputId) QTE_OVERRIDE;

  vtkSmartPointer<vtkVgTrack> Track;
};

#endif
