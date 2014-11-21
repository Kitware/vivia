/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqPredefinedQueryCache_h
#define __vqPredefinedQueryCache_h

#include <QHash>
#include <QObject>

#include <qtGlobal.h>

#include <vvQueryInstance.h>

typedef QHash<QString, vvQueryInstance> vqPredefinedQueryList;

class vqPredefinedQueryCachePrivate;

class vqPredefinedQueryCache : public QObject
{
  Q_OBJECT

public:
  virtual ~vqPredefinedQueryCache();

  static void reload();
  static QHash<QString, vvQueryInstance> getAvailableQueryPlans();

protected slots:
  void setAvailableQueryPlans(vqPredefinedQueryList);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vqPredefinedQueryCache)
  vqPredefinedQueryCache(QObject* parent);

  static vqPredefinedQueryCache* instance();

private:
  QTE_DECLARE_PRIVATE(vqPredefinedQueryCache)
  Q_DISABLE_COPY(vqPredefinedQueryCache)
};

#endif
