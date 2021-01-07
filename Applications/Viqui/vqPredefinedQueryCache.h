// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
