/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
