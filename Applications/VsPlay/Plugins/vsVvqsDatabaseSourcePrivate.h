// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsVvqsDatabaseSourcePrivate_h
#define __vsVvqsDatabaseSourcePrivate_h

#include "vsVvqsDatabaseSource.h"

#include <QUrl>

#include <qtStatusSource.h>
#include <qtThread.h>

#include <vvQuery.h>
#include <vvQueryResult.h>

#include <vsDescriptorInput.h>
#include <vsEvent.h>

class vsVvqsDatabaseTrackSource;

class vsVvqsDatabaseSourcePrivate : public qtThread
{
  Q_OBJECT

public:
  vsVvqsDatabaseSourcePrivate(vsVvqsDatabaseSource* q, vvQuerySession*,
                              const QUrl& request);
  virtual ~vsVvqsDatabaseSourcePrivate();

  QString text(vsDataSource::Status) const;
  QString toolTip(vsDataSource::Status,
                  const QString& sourceTypeSingular,
                  const QString& sourceTypePlural) const;
  QString displayableRequestUri() const;

signals:
  void descriptorsAvailable(vsDescriptorList);
  void eventAvailable(vsEvent);
  void tocAvailable(vsTrackId trackId, vsTrackObjectClassifier toc);

protected slots:
  void processResult(vvQueryResult);
  void finalize();
  void die(qtStatusSource, QString error);

protected:
  QTE_DECLARE_PUBLIC_PTR(vsVvqsDatabaseSource)
  friend class vsVvqsDatabaseTrackSource;

  virtual void run() QTE_OVERRIDE;

  void updateStatus(vsDataSource::Status trackStatus,
                    vsDataSource::Status descriptorStatus);
  void suicide();

  QSharedPointer<vsVvqsDatabaseTrackSource> TrackSource;

  // Access these next two only from public thread
  vsDataSource::Status PublicTrackStatus;
  vsDataSource::Status PublicDescriptorStatus;

  // Access these next two only from private thread
  vsDataSource::Status PrivateTrackStatus;
  vsDataSource::Status PrivateDescriptorStatus;

  QScopedPointer<vvQuerySession> QuerySession;
  vvRetrievalQuery Query;
  const QUrl RequestUri;
  bool RetrieveDescriptors;
  bool ExtractClassifiers;
  int DescriptorBatchSize;

  vsDescriptorList PendingDescriptors;
  vtkIdType NextEventId;

private:
  QTE_DECLARE_PUBLIC(vsVvqsDatabaseSource)
  QTE_DISABLE_COPY(vsVvqsDatabaseSourcePrivate)
};

#endif
