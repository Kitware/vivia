// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgVideoAnimation_h
#define __vgVideoAnimation_h

#include <qtAbstractAnimation.h>

#include <vgExport.h>

#include "vgVideoSourceRequestor.h"

class vgVideoPlayerPrivate;

class VG_VTKVIDEO_EXPORT vgVideoAnimation : public qtAbstractAnimation
{
  Q_OBJECT

public:
  explicit vgVideoAnimation(vgVideoPlayerPrivate* q);
  virtual ~vgVideoAnimation();

  void setFrameRange(const vtkVgTimeStamp& first, const vtkVgTimeStamp& last);
  void seek(const vtkVgTimeStamp&, vg::SeekMode = vg::SeekNearest);
  void seek(vgVideoSeekRequest);

  bool setState(QAbstractAnimation::State);

protected slots:
  void update();

protected:
  QTE_DECLARE_PUBLIC_PTR(vgVideoPlayerPrivate)

private:
  QTE_DECLARE_PUBLIC(vgVideoPlayerPrivate)
  Q_DISABLE_COPY(vgVideoAnimation)

  virtual double duration() const;
  virtual void updateCurrentTime(double time);

  void updateTimeRange(double start, double end);
  void postUpdate(vgVideoSeekRequest);

  double TimeDuration, TimeOffset;
  vgVideoSeekRequest NextRequest;
  bool BlockUpdates;
};

#endif
