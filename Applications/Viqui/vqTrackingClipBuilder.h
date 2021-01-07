// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqTrackingClipBuilder_h
#define __vqTrackingClipBuilder_h

#include "vtkVQTrackingClip.h"

#include <QMutex>
#include <QThread>

#include <array>
#include <atomic>
#include <vector>

class vtkImageData;

class vqTrackingClipBuilder : public QThread
{
  Q_OBJECT

public:
  using ClipElement = std::pair<int, vtkSmartPointer<vtkVQTrackingClip>>;
  using ClipVector = std::vector<ClipElement>;

  vqTrackingClipBuilder();
  virtual ~vqTrackingClipBuilder();

  void BuildClips(ClipVector::iterator start, ClipVector::iterator end);

  void SkipClip(int id);

  void Shutdown();

signals:
  void ClipAvailable(vtkImageData* clip, int index);

protected:
  virtual void run() override;

private:
  std::atomic<bool> ShutdownRequested{false};

  std::array<ClipVector, 2> Clips;
  std::array<QMutex, 2> ClipsMutex;

  QMutex ClipIdMutex;
};

#endif // __vqTrackingClipBuilder_h
