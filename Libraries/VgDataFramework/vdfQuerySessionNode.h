// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfQuerySessionNode_h
#define __vdfQuerySessionNode_h

#include "vdfAbstractQuerySessionNode.h"

#include <vvQueryResult.h>

class QUrl;

class vvQuerySession;

class vdfQuerySessionNodePrivate;

/// Concrete vvIO-based query instance node.
///
/// This class implements an interactive query session node
/// (vdfAbstractQuerySessionNode) using vvQuerySession as a back-end.
class VG_DATA_FRAMEWORK_EXPORT vdfQuerySessionNode :
  public vdfAbstractQuerySessionNode
{
  Q_OBJECT

public:
  /// Create query session node.
  ///
  /// This constructs a query session node using the specified query service.
  /// A vvQuerySession will be automatically created using vvQueryService.
  vdfQuerySessionNode(const QUrl& service, QObject* parent = 0);
  virtual ~vdfQuerySessionNode();

  // vdfNodeBase interface

  /// \copybrief vdfNodeBase::release
  virtual void release() QTE_OVERRIDE;

  // vdfAbstractQueryResultSetNode interface

  /// \copydoc vdfAbstractQueryResultSetNode::result
  virtual const vvQueryResult* result(ResultId iid) const QTE_OVERRIDE;

  /// \copydoc vdfAbstractQueryResultSetNode::resultCount
  virtual int resultCount() const QTE_OVERRIDE;

  /// \copydoc vdfAbstractQueryResultSetNode::results
  virtual QList<ResultId> results(SortOrder sortOrder, int start = 0,
                                  int maxCount = -1) QTE_OVERRIDE;

  /// \copydoc vdfAbstractQueryResultSetNode::resultNode
  virtual vdfNodeBase* resultNode(ResultId) const QTE_OVERRIDE;

  /// \copydoc vdfAbstractQueryResultSetNode::setResultFeedback
  virtual bool setResultFeedback(ResultId, vvIqr::Classification) QTE_OVERRIDE;

  /// \copydoc vdfAbstractQueryResultSetNode::resultItemModel
  ///
  /// vdfQuerySessionNode constructs the model on demand.
  virtual QAbstractItemModel* resultItemModel() const QTE_OVERRIDE;

  // vdfAbstractQuerySessionNode interface

  /// \copydoc vdfAbstractQuerySessionNode::execute
  virtual bool execute(const vvQueryInstance& queryPlan,
                       int workingSetSize) QTE_OVERRIDE;

  /// \copydoc vdfAbstractQuerySessionNode::initialQueryPlan
  virtual vvQueryInstance initialQueryPlan() const QTE_OVERRIDE;

  /// \copydoc vdfAbstractQuerySessionNode::queryPlan
  virtual vvQueryInstance queryPlan() const QTE_OVERRIDE;

  /// \copydoc vdfAbstractQuerySessionNode::statusNotifier
  virtual const qtStatusNotifier* statusNotifier() const QTE_OVERRIDE;

  /// \copydoc vdfAbstractQuerySessionNode::isBusy
  virtual bool isBusy() const QTE_OVERRIDE;

  /// \copydoc vdfAbstractQuerySessionNode::canExecute
  virtual bool canExecute() const QTE_OVERRIDE;

  /// \copydoc canGetFeedbackRequests::canExecute
  virtual bool canGetFeedbackRequests() const QTE_OVERRIDE;

  /// \copydoc vdfAbstractQuerySessionNode::canRefine
  virtual bool canRefine() const QTE_OVERRIDE;

  /// \copydoc vdfAbstractQuerySessionNode::isFeedbackImplicit
  virtual bool isFeedbackImplicit() const QTE_OVERRIDE;

  /// \copydoc vdfAbstractQuerySessionNode::setFeedbackImplicit
  virtual void setFeedbackImplicit(bool) QTE_OVERRIDE;

  /// \copydoc vdfAbstractQuerySessionNode::desiredFeedbackCount
  virtual int desiredFeedbackCount() const QTE_OVERRIDE;

  /// \copydoc vdfAbstractQuerySessionNode::setDesiredFeedbackCount
  virtual void setDesiredFeedbackCount(int) QTE_OVERRIDE;

  /// \copydoc vdfAbstractQuerySessionNode::feedbackRequests
  virtual QSet<ResultId> feedbackRequests() const QTE_OVERRIDE;

  /// \copydoc vdfAbstractQuerySessionNode::isFeedbackRequested
  virtual bool isFeedbackRequested(ResultId) const QTE_OVERRIDE;

public slots:
  /// \copydoc vdfAbstractQuerySessionNode::getFeedbackRequests
  virtual bool getFeedbackRequests() QTE_OVERRIDE;

  /// \copydoc vdfAbstractQuerySessionNode::refine
  virtual bool refine() QTE_OVERRIDE;

protected slots:
  void addResult(vvQueryResult, bool isFeedbackRequest);
  void prepareResults(bool feedbackRequests);

  void fail(qtStatusSource, QString);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vdfQuerySessionNode)

private:
  QTE_DECLARE_PRIVATE(vdfQuerySessionNode)
  QTE_DISABLE_COPY(vdfQuerySessionNode)
};

#endif
