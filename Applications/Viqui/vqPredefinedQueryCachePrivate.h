/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqPredefinedQueryCachePrivate_h
#define __vqPredefinedQueryCachePrivate_h

#include <QDialog>
#include <QWeakPointer>

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
  QWeakPointer<QDialog> WaitDialog;

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
