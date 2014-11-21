/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvQueryNotifier_h
#define __vvQueryNotifier_h

#include <qtStatusNotifier.h>

#include <vgExport.h>

#include <vvQueryResult.h>

//-----------------------------------------------------------------------------
class VV_IO_EXPORT vvQueryNotifier : public qtStatusNotifier
{
  Q_OBJECT

public:
  virtual ~vvQueryNotifier() {}

signals:
  void formulationComplete(QList<vvDescriptor>,
                           QList<vvTrack> = QList<vvTrack>());

  void resultAvailable(vvQueryResult, bool requestScoring = false);
  void resultSetComplete(bool requestScoring = false);

  void error(qtStatusSource, QString = QString());
  void finished();

protected:
  vvQueryNotifier() {}

  void postError(QString message = QString())
    {
    emit this->progressAvailable(this->statusSource());
    emit this->error(this->statusSource(), message);
    }

private:
  Q_DISABLE_COPY(vvQueryNotifier)
};

//-----------------------------------------------------------------------------
class vvQueryNotifierHelper : public vvQueryNotifier
{
public:
  vvQueryNotifierHelper() {}
  virtual ~vvQueryNotifierHelper() {}

  void postFormulationComplete(QList<vvDescriptor> descriptors,
                               QList<vvTrack> tracks)
    { emit this->formulationComplete(descriptors, tracks); }

  void postResult(vvQueryResult result, bool requestScoring = false)
    { emit this->resultAvailable(result, requestScoring); }

  void postResultSetComplete(bool requestScoring = false)
    { emit this->resultSetComplete(requestScoring); }

  void postStatus(QString message, bool clearProgress = false)
    { vvQueryNotifier::postStatus(message, clearProgress); }

  void postStatus(QString message, qreal progress)
    { vvQueryNotifier::postStatus(message, progress); }

  void postStatus(QString message, int progressValue, int progressSteps)
    { vvQueryNotifier::postStatus(message, progressValue, progressSteps); }

  void clearStatus()
    { vvQueryNotifier::clearStatus(); }

  void postError(QString message = QString())
    { vvQueryNotifier::postError(message); }

  void postFinished()
    { emit this->finished(); }

  void forwardSignal(const char* signal, QObject* receiver)
    { connect(this, signal, receiver, signal); }

private:
  Q_DISABLE_COPY(vvQueryNotifierHelper)
};

#endif
