/*ckwg +5
 * Copyright 2017-2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvKipQuerySession_h
#define __vvKipQuerySession_h

#include "vvQuerySession.h"

class vvKipQuerySessionPrivate;

class vvKipQuerySession : public vvQuerySession
{
  Q_OBJECT

public:
  vvKipQuerySession(QUrl server);
  ~vvKipQuerySession();

public slots:
  virtual bool formulateQuery(vvProcessingRequest request);
  virtual bool processQuery(vvQueryInstance query, int workingSetSize);
  virtual bool requestRefinement(int resultsToScore);
  virtual bool refineQuery(vvIqr::ScoringClassifiers feedback);
  virtual bool updateIqrModel(vvQueryInstance& query);

  virtual void endQuery();
  virtual void shutdown();

protected slots:
  virtual void notify();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvKipQuerySession)

  virtual void run();

private:
  QTE_DECLARE_PRIVATE(vvKipQuerySession)
};


#endif
