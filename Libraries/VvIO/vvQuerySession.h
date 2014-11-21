/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvQuerySession_h
#define __vvQuerySession_h

#include <QUrl>

#include <qtThread.h>

#include <vgExport.h>

#include <vvIqr.h>
#include <vvQuery.h>

#include "vvQueryFormulation.h"
#include "vvQueryInstance.h"
#include "vvQueryNotifier.h"

//-----------------------------------------------------------------------------
namespace vvIqr
{
  typedef QHash<long long, vvIqr::Classification> ScoringClassifiers;
}

//-----------------------------------------------------------------------------
class VV_IO_EXPORT vvQuerySession
  : public qtInternallyThreadedObject<vvQueryNotifier>
{
  Q_OBJECT

public:
  vvQuerySession();
  virtual ~vvQuerySession();

public:
  virtual QUrl getFormulationSourceUri(
    const QUrl& oldUri, vvQueryFormulationType type, QWidget* dialogParent);
  virtual QUrl fixupFormulationSourceUri(QUrl, vvQueryFormulationType);

public slots:
  virtual bool formulateQuery(vvProcessingRequest request) = 0;
  virtual bool processQuery(vvQueryInstance query, int workingSetSize) = 0;
  virtual bool requestRefinement(int resultsToScore) = 0;
  virtual bool refineQuery(vvIqr::ScoringClassifiers feedback) = 0;

  virtual void endQuery() = 0;
  virtual void shutdown() = 0;

  virtual bool updateIqrModel(vvQueryInstance& query)
    { Q_UNUSED(query); return false; }

private:
  Q_DISABLE_COPY(vvQuerySession)
};

#endif
