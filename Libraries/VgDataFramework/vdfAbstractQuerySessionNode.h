/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfAbstractQuerySessionNode_h
#define __vdfAbstractQuerySessionNode_h

#include "vdfAbstractQueryResultSetNode.h"
#include "vdfNodeBase.h"

#include <qtStatusSource.h>

template <typename T> class QSet;

class qtStatusNotifier;

class vvQueryInstance;
struct vvQueryResult;

/// Interface for a data node providing interactive access to a query session.
class VG_DATA_FRAMEWORK_EXPORT vdfAbstractQuerySessionNode :
  public vdfNodeBase,
  virtual public vdfAbstractQueryResultSetNode
{
  Q_OBJECT
  Q_INTERFACES(vdfAbstractQueryResultSetNode)

  /// Flag indicating if the query session is busy.
  ///
  /// Users should not be allowed to interact with a busy query session.
  ///
  /// \sa busyChanged()
  Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)

  /// Flag indicating if execution of a query is possible.
  ///
  /// Normally it is only possible to execute a query immediately after
  /// creating a node (i.e. before a query has been executed). However, if
  /// there is a problem with the node, it may never be possible to execute a
  /// query.
  Q_PROPERTY(bool canExecute READ canExecute)

  /// Flag indicating if feedback requests may be retrieved.
  ///
  /// A query session can normally only ask for feedback requests just after
  /// the 'normal' result set has been received. If this is done implicitly
  /// (see feedbackImplicit), this is likely to never be \c true. If managing
  /// feedback requests manually, this flag indicates when the node may be
  /// asked to get feedback requests.
  Q_PROPERTY(bool canGetFeedbackRequests READ canGetFeedbackRequests)

  /// Flag indicating if refinement of the query is possible.
  ///
  /// Refinement may not be possible for some query sessions, or until the user
  /// provides feedback. The application should disable refinement controls
  /// when refinement is not possible.
  Q_PROPERTY(bool canRefine READ canRefine NOTIFY canRefineChanged)

  /// Flag indicating if the node should automatically get feedback requests.
  ///
  /// Executing a query returns a 'normal' result set. In order to improve
  /// <abbr title="Iterative Query Refinement">IQR</abbr>, the query service
  /// may be asked to provide a set of feedback requests. This flag controls
  /// whether this request is made implicitly once the 'normal' results are
  /// available. When \c true, getFeedbackRequests() is called automatically
  /// at the appropriate time.
  ///
  /// By default, feedbackImplicit is \c true.
  Q_PROPERTY(bool feedbackImplicit READ isFeedbackImplicit
                                   WRITE setFeedbackImplicit)

  /// Desired number of results on which to request feedback.
  Q_PROPERTY(int desiredFeedbackCount READ desiredFeedbackCount
                                      WRITE setDesiredFeedbackCount)

public:
  virtual ~vdfAbstractQuerySessionNode() {}

  /// Execute query plan.
  ///
  /// This executes the specified query plan and sets the working set size. The
  /// working set is the pool of results that will be used for query
  /// refinement; depending on the implementation of the refinement engine,
  /// results after refinement may or may not be limited to the initial working
  /// set. Similarly, the size is a suggestion which is passed to the back end,
  /// which the back end may choose to ignore.
  ///
  /// As vdfAbstractQuerySessionNode is not intended for managing multiple
  /// queries per instance, most implementations will only allow one query to
  /// be executed per instance. Providing this operation separate from
  /// construction is intended mainly to give the application the opportunity
  /// to connect to the status notifier before any status changes occur.
  ///
  /// \return \c true if the query was successfully dispatched, otherwise
  ///         \c false.
  ///
  /// Users should check canExecute before calling this method.
  ///
  /// \note Because the query may (and probably will) execute asynchronously,
  ///       the return value ("dispatched" versus "executed") will generally
  ///       only catch errors that happen in the early, synchronous stages of
  ///       query execution (e.g. if canExecute() is \c false).
  virtual bool execute(const vvQueryInstance& queryPlan,
                       int workingSetSize) = 0;

  /// Get the query plan used to initiate this query.
  ///
  /// The query plan is empty until a query is executed.
  virtual vvQueryInstance initialQueryPlan() const = 0;

  /// Get the query plan associated with this query session.
  ///
  /// This returns the query plan associated with this query. Initially this is
  /// the query plan used to initiate the query, but the query plan may be
  /// updated as the query is refined.
  ///
  /// \sa initialQueryPlan()
  virtual vvQueryInstance queryPlan() const = 0;

  /// Get the status notifier associated with this query session.
  ///
  /// This returns the qtStatusNotifier associated with this query session. The
  /// status notifier provides notification of query progress and status that
  /// the application may wish to present to the user, by connecting to the
  /// status notifier's signals.
  virtual const qtStatusNotifier* statusNotifier() const = 0;

  /// Get query session 'busy' state.
  /// \sa busy
  virtual bool isBusy() const = 0;

  /// Get query session 'canExecute' state.
  /// \sa canExecute
  virtual bool canExecute() const = 0;

  /// Get query session 'canGetFeedbackRequests' state.
  /// \sa canGetFeedbackRequests
  virtual bool canGetFeedbackRequests() const = 0;

  /// Get query session 'canRefine' state.
  /// \sa canRefine
  virtual bool canRefine() const = 0;

  /// Get if asking for feedback requests is implicit.
  /// \sa feedbackImplicit
  virtual bool isFeedbackImplicit() const = 0;

  /// Set if asking for feedback requests is implicit.
  /// \sa feedbackImplicit
  virtual void setFeedbackImplicit(bool) = 0;

  /// Get desired number of results on which to request feedback.
  /// \sa desiredFeedbackCount
  virtual int desiredFeedbackCount() const = 0;

  /// Set desired number of results on which to request feedback.
  /// \sa desiredFeedbackCount
  virtual void setDesiredFeedbackCount(int) = 0;

  /// Get results for which feedback is requested.
  ///
  /// This returns a list of result ID's for which feedback has been requested
  /// by the query engine being used for this session.
  virtual QSet<ResultId> feedbackRequests() const = 0;

  /// Get if feedback has been requested for specific result.
  ///
  /// \return \c true if feedback is requested for the specified result,
  ///         otherwise \c false.
  virtual bool isFeedbackRequested(ResultId) const = 0;

signals:
  /// Notification when the query session's 'busy' state changes.
  /// \param newValue New value of the state.
  /// \sa busy
  void busyChanged(bool newValue);

  /// Notification when the query session's 'canRefine' state changes.
  /// \param newValue New value of the state.
  /// \sa canRefine
  void canRefineChanged(bool newValue);

  /// Notification when all results have been received.
  ///
  /// This signal is emitted when we have received all results for a query
  /// iteration of the type designated by \p scoringRequests. Note that this is
  /// usually emitted twice per query iteration; once when all 'regular'
  /// results are ready (with \p scoringRequests = \c false), and once when all
  /// results for which IQR has requested feedback have been received (with
  /// \p scoringRequests = \c true).
  void resultSetComplete(bool scoringRequests);

  /// Notification when an error occurs while processing a query action.
  void error(qtStatusSource, QString) const;

public slots:
  /// Ask session for feedback requests
  ///
  /// This method requests that the session to provide a set of results on
  /// which it would like feedback in order to optimally perform refinement.
  /// (If feedbackImplicit is \c true, this is done automatically.)
  ///
  /// \return \c true if refinement is proceeding, \c false if refinement is
  ///         not possible at this time.
  ///
  /// Users should check canGetFeedbackRequests before calling this method.
  virtual bool getFeedbackRequests() = 0;

  /// Request refinement of the query.
  ///
  /// This method requests that the session refine the query, using whatever
  /// refinement data has been provided. Refinement is typically done
  /// asynchronously.
  ///
  /// \return \c true if refinement is proceeding, \c false if refinement is
  ///         not possible at this time.
  ///
  /// Users should check canRefine before calling this method.
  virtual bool refine() = 0;

protected:
  vdfAbstractQuerySessionNode(QObject* parent = 0) : vdfNodeBase(parent) {}
};

#endif
