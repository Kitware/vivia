// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpKwiverEmbeddedPipelineWorker_h
#define __vpKwiverEmbeddedPipelineWorker_h

#include <qtGlobal.h>

#include <QObject>

#include <memory>

namespace kwiver
{
namespace vital
{
class object_track_set;
}
}

class vpFileDataSource;
class vtkVpTrackModel;
class vtkVgTrackTypeRegistry;

class vpKwiverEmbeddedPipelineWorkerPrivate;

class vpKwiverEmbeddedPipelineWorker : public QObject
{
  Q_OBJECT

public:
  vpKwiverEmbeddedPipelineWorker(QObject* parent = nullptr);
  ~vpKwiverEmbeddedPipelineWorker();

  bool initialize(
    const QString& pipelineFile, vpFileDataSource* dataSource,
    vtkVpTrackModel* trackModel, vtkVgTrackTypeRegistry const* trackTypes,
    double videoHeight);

  std::shared_ptr<kwiver::vital::object_track_set> tracks() const;

public slots:
  void execute();
  void cancel();

signals:
  void progressRangeChanged(int minimum, int maximum);
  void progressValueChanged(int value);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpKwiverEmbeddedPipelineWorker)

private:
  QTE_DECLARE_PRIVATE(vpKwiverEmbeddedPipelineWorker)
};

#endif
