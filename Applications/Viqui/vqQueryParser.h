// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqQueryParser_h
#define __vqQueryParser_h

#include <QUrl>
#include <QObject>

#include <vvQueryNotifier.h>
#include <vvQueryResult.h>
#include <vvQuerySession.h>

class vqQueryParser : public vvQueryNotifier
{
  Q_OBJECT

public:
  vqQueryParser(const QUrl& queryServerUri = QUrl(),
                const QUrl& cacheUri = QUrl());
  ~vqQueryParser();

  bool loadQuery(const QUrl& queryUri);
  bool formulateQuery(vvProcessingRequest request, bool bypassCache = false);
  bool processQuery(vvQueryInstance query);
  bool requestScoring(int resultsToScore);
  bool refineQuery(vvIqr::ScoringClassifiers feedback);
  bool updateIqrModel(vvQueryInstance& query);

signals:
  void planAvailable(vvQueryInstance plan);

protected slots:
  bool updateDescriptorCache(QList<vvDescriptor>);
  bool updateTrackAndDescriptorCache(QList<vvDescriptor>, QList<vvTrack>);
  void queryFinished();

  void forwardStatusMessage(qtStatusSource, QString);
  void forwardProgress(qtStatusSource, bool, qreal);
  void forwardProgress(qtStatusSource, bool, int, int);
  void forwardError(qtStatusSource, QString);

protected:
  void forwardSignal(const char* signal);

  bool beginSession();
  bool syncAbort(const QString&);
  bool asyncAbort(const QString&);

  struct Internal;
  QScopedPointer<Internal> internal_;
};

#endif // __vqQueryParser_h
