/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvFakeQuerySession_h
#define __vvFakeQuerySession_h

#include "vvQuerySession.h"

class vvFakeQuerySessionPrivate;

class vvFakeQuerySession : public vvQuerySession
{
  Q_OBJECT

public:
  vvFakeQuerySession(QUrl server);
  ~vvFakeQuerySession();

public slots:
  virtual bool formulateQuery(vvProcessingRequest request);
  virtual bool processQuery(vvQueryInstance query, int workingSetSize);
  virtual bool requestRefinement(int resultsToScore);
  virtual bool refineQuery(vvIqr::ScoringClassifiers feedback);

  virtual void endQuery();
  virtual void shutdown();

protected slots:
  virtual void notify();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvFakeQuerySession)

  virtual void run();

private:
  QTE_DECLARE_PRIVATE(vvFakeQuerySession)
};


#endif
