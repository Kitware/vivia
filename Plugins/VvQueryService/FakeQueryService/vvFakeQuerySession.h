// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
