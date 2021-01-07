// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vdfQuerySessionNode.h"

#include <vvQueryService.h>
#include <vvQuerySession.h>

#include <qtMap.h>
#include <qtStatusForwarder.h>

#include <QSet>

namespace // anonymous
{

// Alias types so we can use it unqualified
typedef vdfAbstractQueryResultSetNode::ResultId ResultId;
typedef vdfAbstractQuerySessionNode::SortOrder SortOrder;

// Convenience types
typedef QList<ResultId> ResultIdList;
typedef QPair<vvIqr::Classification, vvIqr::Classification> FeedbackChange;
typedef QHash<ResultId, vvQueryResult>::const_iterator ResultIterator;

typedef bool (*CompareResults)(const vvQueryResult*, const vvQueryResult*);

//-----------------------------------------------------------------------------
bool CompareResultsById(const vvQueryResult* a, const vvQueryResult* b)
{
  return a->InstanceId < b->InstanceId;
}

//-----------------------------------------------------------------------------
bool CompareResultsByRank(const vvQueryResult* a, const vvQueryResult* b)
{
  if (a->Rank == b->Rank)
    {
    const double ra = a->RelevancyScore;
    const double rb = b->RelevancyScore;

    return (!qFuzzyCompare(ra, rb) ? ra < rb : a->InstanceId < b->InstanceId);
    }

  return a->Rank < b->Rank;
}

//-----------------------------------------------------------------------------
bool CompareResultsByRelevancy(const vvQueryResult* a, const vvQueryResult* b)
{
  const double ra = a->RelevancyScore;
  const double rb = b->RelevancyScore;

  return (!qFuzzyCompare(ra, rb) ? ra < rb :
          a->Rank != b->Rank ? a->Rank < b->Rank :
          a->InstanceId < b->InstanceId);
}

//-----------------------------------------------------------------------------
bool CompareResultsByPreference(const vvQueryResult* a, const vvQueryResult* b)
{
  const double pa = a->PreferenceScore;
  const double pb = b->PreferenceScore;

  return (!qFuzzyCompare(pa, pb) ? pa < pb :
          a->Rank != b->Rank ? a->Rank < b->Rank :
          a->InstanceId < b->InstanceId);
}

} // namespace <anonymous>

///////////////////////////////////////////////////////////////////////////////

//BEGIN vdfQuerySessionNodePrivate

//-----------------------------------------------------------------------------
class vdfQuerySessionNodePrivate : public qtStatusForwarder
{
public:
  vdfQuerySessionNodePrivate(vdfQuerySessionNode* q) : q_ptr(q) {}

  void setBusy(bool);
  void setCanRefine(bool);

  bool getFeedbackRequests();

  void addResult(ResultId iid, const vvQueryResult& result);
  void removeResult(ResultId iid);

  void sortResults(SortOrder sortOrder, ResultIdList& out);

  bool Busy;
  bool CanExecute;
  bool CanGetFeedbackRequests;
  bool CanRefine;

  bool FeedbackImplicit;
  int DesiredFeedbackCount;

  QScopedPointer<vvQuerySession> Session;

  vvQueryInstance QueryPlan;
  vvQueryInstance InitialQueryPlan;

  QHash<ResultId, vvQueryResult> Results;
  QSet<ResultId> FeedbackRequests;
  QSet<ResultId> RemovedResults;

  QHash<ResultId, FeedbackChange> FeedbackChanges;

  QHash<SortOrder, ResultIdList> SortedResults;

private:
  QTE_DECLARE_PUBLIC_PTR(vdfQuerySessionNode)
  QTE_DECLARE_PUBLIC(vdfQuerySessionNode)
};

QTE_IMPLEMENT_D_FUNC(vdfQuerySessionNode)

//-----------------------------------------------------------------------------
void vdfQuerySessionNodePrivate::setBusy(bool state)
{
  if (this->Busy != state)
    {
    QTE_Q(vdfQuerySessionNode);
    this->Busy = state;
    emit q->busyChanged(state);
    }
}

//-----------------------------------------------------------------------------
void vdfQuerySessionNodePrivate::setCanRefine(bool state)
{
  if (this->CanRefine != state)
    {
    QTE_Q(vdfQuerySessionNode);
    this->CanRefine = state;
    emit q->canRefineChanged(state);
    }
}

//-----------------------------------------------------------------------------
bool vdfQuerySessionNodePrivate::getFeedbackRequests()
{
  this->setBusy(true);
  return this->Session->requestRefinement(this->DesiredFeedbackCount);
}

//-----------------------------------------------------------------------------
void vdfQuerySessionNodePrivate::addResult(
  ResultId iid, const vvQueryResult& result)
{
  // TODO create or update result node
  this->Results.insert(iid, result);
  this->SortedResults.clear();
}

//-----------------------------------------------------------------------------
void vdfQuerySessionNodePrivate::removeResult(ResultId iid)
{
  // TODO remove result node
  this->Results.remove(iid);
  this->SortedResults.clear();
}

//-----------------------------------------------------------------------------
void vdfQuerySessionNodePrivate::sortResults(
  SortOrder sortOrder, ResultIdList& out)
{
  // Get compare function
  CompareResults compare;
  switch (sortOrder)
    {
    case vdfAbstractQueryResultSetNode::SortByRank:
      compare = &CompareResultsByRank;
      break;
    case vdfAbstractQueryResultSetNode::SortByRelevancy:
      compare = &CompareResultsByRelevancy;
      break;
    case vdfAbstractQueryResultSetNode::SortByFeedbackPreference:
      compare = &CompareResultsByPreference;
      break;
    default:
      compare = &CompareResultsById;
      break;
    }

  // Get list of results (as pointers, to avoid copying)
  QList<const vvQueryResult*> sortedResults;
  sortedResults.reserve(this->Results.count());

  const ResultIterator end = this->Results.constEnd();
  for (ResultIterator iter = this->Results.constBegin(); iter != end; ++iter)
    {
    sortedResults.append(&iter.value());
    }

  // Sort results
  qSort(sortedResults.begin(), sortedResults.end(), compare);

  // Convert list of result pointers to list of result ID's
  out.clear();
  out.reserve(sortedResults.count());
  foreach (const vvQueryResult* r, sortedResults)
    {
    out.append(r->InstanceId);
    }
}

//END vdfQuerySessionNodePrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vdfQuerySessionNode ctor/dtor and state-query methods

//-----------------------------------------------------------------------------
vdfQuerySessionNode::vdfQuerySessionNode(
  const QUrl& service, QObject* parent) :
  vdfAbstractQuerySessionNode(parent),
  d_ptr(new vdfQuerySessionNodePrivate(this))
{
  QTE_D(vdfQuerySessionNode);

  d->Busy = false;
  d->CanRefine = false;
  d->CanGetFeedbackRequests = false;
  d->CanExecute = false;

  d->FeedbackImplicit = true;
  d->DesiredFeedbackCount = 10;

  vvQuerySession* const session = vvQueryService::createSession(service);
  if (session)
    {
    d->Session.reset(session);
    d->CanExecute = true;

    // Connect to query session
    connect(session, SIGNAL(resultAvailable(vvQueryResult, bool)),
            this, SLOT(addResult(vvQueryResult, bool)));
    connect(session, SIGNAL(resultSetComplete(bool)),
            this, SLOT(prepareResults(bool)));
    connect(session, SIGNAL(error(qtStatusSource, QString)),
            this, SLOT(fail(qtStatusSource, QString)));

    // Forward status notifications
    d->connect(d->Session.data());
    }
}

//-----------------------------------------------------------------------------
vdfQuerySessionNode::~vdfQuerySessionNode()
{
}

//-----------------------------------------------------------------------------
bool vdfQuerySessionNode::isBusy() const
{
  QTE_D_CONST(vdfQuerySessionNode);
  return d->Busy;
}

//-----------------------------------------------------------------------------
bool vdfQuerySessionNode::canExecute() const
{
  QTE_D_CONST(vdfQuerySessionNode);
  return d->CanExecute && !d->Busy;
}

//-----------------------------------------------------------------------------
bool vdfQuerySessionNode::canGetFeedbackRequests() const
{
  QTE_D_CONST(vdfQuerySessionNode);
  return d->CanGetFeedbackRequests && !d->Busy;
}

//-----------------------------------------------------------------------------
bool vdfQuerySessionNode::canRefine() const
{
  QTE_D_CONST(vdfQuerySessionNode);
  return d->CanRefine && !d->Busy;
}

//-----------------------------------------------------------------------------
bool vdfQuerySessionNode::isFeedbackImplicit() const
{
  QTE_D_CONST(vdfQuerySessionNode);
  return d->FeedbackImplicit;
}

//-----------------------------------------------------------------------------
void vdfQuerySessionNode::setFeedbackImplicit(bool state)
{
  QTE_D(vdfQuerySessionNode);
  d->DesiredFeedbackCount = state;
}

//-----------------------------------------------------------------------------
int vdfQuerySessionNode::desiredFeedbackCount() const
{
  QTE_D_CONST(vdfQuerySessionNode);
  return d->DesiredFeedbackCount;
}

//-----------------------------------------------------------------------------
void vdfQuerySessionNode::setDesiredFeedbackCount(int value)
{
  QTE_D(vdfQuerySessionNode);
  d->DesiredFeedbackCount = value;
}

//-----------------------------------------------------------------------------
const qtStatusNotifier* vdfQuerySessionNode::statusNotifier() const
{
  QTE_D_CONST(vdfQuerySessionNode);
  return d;
}

//END vdfQuerySessionNode ctor/dtor and state-query methods

///////////////////////////////////////////////////////////////////////////////

//BEGIN vdfQuerySessionNode data access

//-----------------------------------------------------------------------------
void vdfQuerySessionNode::release()
{
  QTE_D(vdfQuerySessionNode);
  // TODO release Qt item model
  d->SortedResults.clear();

  vdfNodeBase::release();
}

//-----------------------------------------------------------------------------
vvQueryInstance vdfQuerySessionNode::initialQueryPlan() const
{
  QTE_D_CONST(vdfQuerySessionNode);
  return d->InitialQueryPlan;
}

//-----------------------------------------------------------------------------
vvQueryInstance vdfQuerySessionNode::queryPlan() const
{
  QTE_D_CONST(vdfQuerySessionNode);
  return d->QueryPlan;
}

//-----------------------------------------------------------------------------
int vdfQuerySessionNode::resultCount() const
{
  QTE_D_CONST(vdfQuerySessionNode);
  return d->Results.count();
}

//-----------------------------------------------------------------------------
QList<ResultId> vdfQuerySessionNode::results(
  SortOrder sortOrder, int start, int maxCount)
{
  QTE_D(vdfQuerySessionNode);

  if (start < 0 || start > d->Results.count())
    {
    // Return empty in case of range errors
    return ResultIdList();
    }

  ResultIdList& fullList = d->SortedResults[sortOrder];
  if (fullList.isEmpty() && !d->Results.isEmpty())
    {
    // Full list was implicitly created by QHash::operator[], which means we
    // need to create the actual sorted list now and store it back into the
    // sorting cache
    d->sortResults(sortOrder, fullList);
    }

  // Return requested slice of the full sorted list
  return fullList.mid(start, maxCount);
}

//-----------------------------------------------------------------------------
const vvQueryResult* vdfQuerySessionNode::result(ResultId iid) const
{
  QTE_D_CONST(vdfQuerySessionNode);
  QHash<ResultId, vvQueryResult>::const_iterator iter = d->Results.find(iid);
  return (iter != d->Results.constEnd() ? &iter.value() : 0);
}

//-----------------------------------------------------------------------------
vdfNodeBase* vdfQuerySessionNode::resultNode(ResultId iid) const
{
  // TODO
  return 0;
}

//-----------------------------------------------------------------------------
QAbstractItemModel* vdfQuerySessionNode::resultItemModel() const
{
  // TODO
  return 0;
}

//-----------------------------------------------------------------------------
QSet<ResultId> vdfQuerySessionNode::feedbackRequests() const
{
  QTE_D_CONST(vdfQuerySessionNode);
  return d->FeedbackRequests;
}

//-----------------------------------------------------------------------------
bool vdfQuerySessionNode::isFeedbackRequested(ResultId iid) const
{
  return this->feedbackRequests().contains(iid);
}

//END vdfQuerySessionNode data access

///////////////////////////////////////////////////////////////////////////////

//BEGIN vdfQuerySessionNode data manipulation

//-----------------------------------------------------------------------------
bool vdfQuerySessionNode::setResultFeedback(
  ResultId iid, vvIqr::Classification score)
{
  QTE_D(vdfQuerySessionNode);

  // Does the requested result exist?
  if (d->Results.contains(iid))
    {
    // Get the result and check if we need to do anything
    vvQueryResult& r = d->Results[iid];
    if (r.UserScore != score)
      {
      // Was feedback for this result already changed?
      if (d->FeedbackChanges.contains(iid))
        {
        FeedbackChange& c = d->FeedbackChanges[iid];
        if (c.first == score)
          {
          // Undoing previous change; result feedback is no longer changed
          d->FeedbackChanges.remove(iid);
          }
        else
          {
          // Feedback is still changed, but new score is different from
          // previous change
          c.second = score;
          }
        }
      else
        {
        // Result feedback not changed since last IQR round; create new change
        d->FeedbackChanges.insert(iid, FeedbackChange(r.UserScore, score));
        }

      // Update the result...
      r.UserScore = score;
      // TODO update result node also
      if (d->InitialQueryPlan.isSimilarityQuery())
        {
        // ...and whether or not refinement is now possible (which it only is
        // for similarity queries)...
        d->setCanRefine(!d->FeedbackChanges.empty());
        }

      // ...and signal that we are modified
      emit this->modified();
      }

    return true;
    }

  return false;
}

//END vdfQuerySessionNode data manipulation

///////////////////////////////////////////////////////////////////////////////

//BEGIN vdfQuerySessionNode query execution and response handling

//-----------------------------------------------------------------------------
bool vdfQuerySessionNode::execute(
  const vvQueryInstance& queryPlan, int workingSetSize)
{
  if (queryPlan.isValid() && this->canExecute())
    {
    QTE_D(vdfQuerySessionNode);
    d->CanExecute = false;
    d->setBusy(true);
    d->QueryPlan = d->InitialQueryPlan = queryPlan;
    return d->Session->processQuery(queryPlan, workingSetSize);
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vdfQuerySessionNode::getFeedbackRequests()
{
  if (this->canGetFeedbackRequests())
    {
    QTE_D(vdfQuerySessionNode);
    return d->getFeedbackRequests();
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vdfQuerySessionNode::refine()
{
  if (this->canRefine())
    {
    QTE_D(vdfQuerySessionNode);

    vvIqr::ScoringClassifiers feedback;
    int feedbackCount = 0;

    // Include scores for all feedback requests, regardless of score...
    foreach (ResultId iid, d->FeedbackRequests)
      {
      vvIqr::Classification score = d->Results[iid].UserScore;
      feedback.insert(iid, score);
      if (score != vvIqr::UnclassifiedExample)
        {
        // ...but only count as usable feedback if it isn't Unclassified
        ++feedbackCount;
        }
      }

    // Include positive/negative scores for all nodes
    const ResultIterator end = d->Results.constEnd();
    for (ResultIterator iter = d->Results.constBegin(); iter != end; ++iter)
      {
      vvIqr::Classification score = iter->UserScore;
      if (score != vvIqr::UnclassifiedExample)
        {
        feedback.insert(iter.key(), score);
        ++feedbackCount;
        }
      }

    // Check that there is usable feedback to provide
    if (!feedbackCount)
      {
      emit this->error(d->statusSource(),
                       "Unable to refine query: no feedback provided");
      return false;
      }

    // In case we don't get back all the old results, mark all of the current
    // results as removed by the refinement round; we'll remove them again from
    // the set as they arrive in the new set
    d->RemovedResults = d->Results.keys().toSet();

    // Initiate refinement
    d->setCanRefine(false);
    d->FeedbackRequests.clear();
    d->FeedbackChanges.clear();
    return d->Session->refineQuery(feedback);
    }

  return false;
}

//-----------------------------------------------------------------------------
void vdfQuerySessionNode::addResult(vvQueryResult r, bool isFeedbackRequest)
{
  QTE_D(vdfQuerySessionNode);

  const ResultId iid = r.InstanceId;

  if (isFeedbackRequest)
    {
    d->FeedbackRequests.insert(iid);
    if (!d->Results.contains(iid))
      {
      // Only add result if we don't already have it
      d->addResult(iid, r);
      }
    else
      {
      // Otherwise, update the preference score
      d->Results[iid].PreferenceScore = r.PreferenceScore;
      d->SortedResults.remove(SortByFeedbackPreference);
      }
    }
  else
    {
    // If not a feedback request, always replace the result (e.g. updating
    // results after an IQR round)...
    const ResultIterator iter = d->Results.constFind(iid);
    if (iter != d->Results.constEnd())
      {
      // ...but first copy user mutable information that may not be preserved
      // by (or even ever known to) the query back-end
      const vvQueryResult& o = iter.value();
      r.UserData = o.UserData;
      r.UserScore = o.UserScore;
      }
    d->addResult(iid, r);
    }

  // Result was returned again; remove from set of results that weren't
  d->RemovedResults.remove(iid);

  emit this->modified();
}

//-----------------------------------------------------------------------------
void vdfQuerySessionNode::prepareResults(bool feedbackRequests)
{
  QTE_D(vdfQuerySessionNode);

  d->CanGetFeedbackRequests = !feedbackRequests;

  if (!feedbackRequests && d->FeedbackImplicit)
    {
    // If we implicitly want to ask for feedback requests, do so now...
    d->getFeedbackRequests();
    }
  else
    {
    // ...otherwise clean up removed results, and then we are no longer busy
    if (!d->RemovedResults.isEmpty())
      {
      qtUtil::mapBound(d->RemovedResults, d,
                       &vdfQuerySessionNodePrivate::removeResult);
      emit this->modified();
      }
    d->setBusy(false);
    }

  // Re-emit signal; done here rather than forwarding to ensure we have cleaned
  // up any results removed by refinement before consumers are alerted that the
  // new set is ready
  emit this->resultSetComplete(feedbackRequests);
}

//-----------------------------------------------------------------------------
void vdfQuerySessionNode::fail(qtStatusSource ss, QString errorMessage)
{
  QTE_D(vdfQuerySessionNode);

  Q_UNUSED(ss)

  d->setBusy(false);
  emit this->error(d->statusSource(), errorMessage);
}

//END vdfQuerySessionNode query execution and response handling
