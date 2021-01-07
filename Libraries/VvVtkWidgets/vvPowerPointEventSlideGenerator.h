// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvPowerPointEventSlideGenerator_h
#define __vvPowerPointEventSlideGenerator_h

#include "vvPowerPointSlideGenerator.h"

#include <qtGlobal.h>

#include <vtkSmartPointer.h>

class vtkVgEvent;
class vtkVgEventTypeRegistry;

class VV_VTKWIDGETS_EXPORT vvPowerPointEventSlideGenerator :
  public vvPowerPointSlideGenerator
{
public:
  vvPowerPointEventSlideGenerator(
    vtkVgEvent* event, vtkVgEventTypeRegistry* registry);
  virtual ~vvPowerPointEventSlideGenerator();

  virtual bool generateSlide(
    vvPowerPointWriter& writer, vgVideoRequestor& videoRequestor, int outputId,
    bool generateVideo, QProgressDialog* progress, int& progressCount)
    QTE_OVERRIDE;

  virtual int outputFrameCount(bool generateVideo) QTE_OVERRIDE;

  vtkVgEvent* event() { return this->Event; }

protected:
  virtual void writeSummary(vvPowerPointWriter& writer,
                            const ImageInfo& imageInfo,
                            int outputId) QTE_OVERRIDE;
  virtual void writeVideoFrame(vvPowerPointWriter& writer,
                               const ImageInfo& imageInfo,
                               int outputId, int frameNumber) QTE_OVERRIDE;
  virtual void createVideo(vvPowerPointWriter& writer,
                           int outputId) QTE_OVERRIDE;

  static int addEventRegion(vvPowerPointWriter& writer, vtkVgEvent* event,
                            const vtkMatrix4x4* transform,
                            const vtkVgTimeStamp& timeStamp,
                            const char* itemTemplateName, int imageId);

  vtkSmartPointer<vtkVgEvent> Event;
  vtkSmartPointer<vtkVgEventTypeRegistry> Registry;
};

#endif
