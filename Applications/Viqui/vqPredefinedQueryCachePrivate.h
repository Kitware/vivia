// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqPredefinedQueryCachePrivate_h
#define __vqPredefinedQueryCachePrivate_h

#include <QDialog>
#include <QPointer>

#include <qtStatusSource.h>
#include <qtThread.h>

#include "vqPredefinedQueryCache.h"

class vqPredefinedQueryCachePrivate : public qtThread
{
  Q_OBJECT

public:
  vqPredefinedQueryCachePrivate();
  virtual ~vqPredefinedQueryCachePrivate() {}

  void reload();
  vqPredefinedQueryList getAvailableQueryPlans();

  bool PlansReady;
  bool Running;
  vqPredefinedQueryList AvailableQueryPlans;
  QPointer<QDialog> WaitDialog;

  volatile bool Interrupt;

signals:
  void loadingComplete(vqPredefinedQueryList);

protected slots:
  void acceptLoadedQuery(vvQueryInstance);
  void abortQueryLoading(qtStatusSource, QString);

protected:
  virtual void run();

  void loadQuery(const QString& path, const QString& fileName);

  QString CurrentlyLoadingPlan;
  vqPredefinedQueryList LoadedPlans;

private:
  Q_DISABLE_COPY(vqPredefinedQueryCachePrivate)
};

#endif
