// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpKwiverImproveTrackWorker_h
#define __vpKwiverImproveTrackWorker_h

#include <qtGlobal.h>

#include <QObject>

#include <memory>

namespace kwiver { namespace vital { class track; } }

class vtkVgTrack;
class vtkVpTrackModel;
class vpKwiverVideoSource;

class vpKwiverImproveTrackWorkerPrivate;

class vpKwiverImproveTrackWorker : public QObject
{
  Q_OBJECT

public:
  vpKwiverImproveTrackWorker(QObject* parent = nullptr);
  ~vpKwiverImproveTrackWorker();

  bool initialize(vtkVgTrack* track, const vtkVpTrackModel* trackModel,
                  std::shared_ptr<vpKwiverVideoSource> videoSource,
                  double videoHeight);

  std::shared_ptr<kwiver::vital::track> execute();

signals:
  void progressRangeChanged(int minimum, int maximum);
  void progressValueChanged(int value);

protected:
  void updateProgress(int current, int maximum);

  QTE_DECLARE_PRIVATE_RPTR(vpKwiverImproveTrackWorker)

private:
  QTE_DECLARE_PRIVATE(vpKwiverImproveTrackWorker)
};

#endif
