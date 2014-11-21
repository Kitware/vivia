/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfAbstractQueryResultSetNode_h
#define __vdfAbstractQueryResultSetNode_h

#include <vvIqr.h>

#include <QObject>

class QAbstractItemModel;

class vdfNodeBase;

struct vvQueryResult;

/// Interface for a data node providing a collection of query results.
///
/// This interface manages access to a collection of query results. It provides
/// users with a way to look up specific results or their associated data nodes
/// by ID, and to retrieve the ID's of result subsets. It also provides a Qt
/// item data model that may be used to represent results in a Qt item view.
class vdfAbstractQueryResultSetNode
{
public:
  /// Enumeration of sort orders for result subset retrieval.
  enum SortOrder
    {
    /// The results are sorted by ID.
    SortById,
    /// The results are sorted by relevancy score, then by rank, and lastly by
    /// ID.
    SortByRelevancy,
    /// The results are sorted by preference score, then by rank, and lastly by
    /// ID.
    SortByFeedbackPreference,
    /// The results are sorted by rank, then by relevancy score, and lastly by
    /// ID.
    SortByRank
    };

  typedef long long ResultId;

  virtual ~vdfAbstractQueryResultSetNode() {}

  /// Look up result by ID.
  ///
  /// \return Pointer to the requested result, or \c nullptr if no such result
  ///         exists.
  virtual const vvQueryResult* result(ResultId iid) const = 0;

  /// Get the total number of results available.
  virtual int resultCount() const = 0;

  /// Get ID's of a subset of the result set.
  ///
  /// This method allows users to request a list of result ID's of either the
  /// entire set, or a subset.
  ///
  /// \param sortOrder Order in which the results will be sorted.
  /// \param start First index of the sorted set to be included in the subset.
  /// \param maxCount Maximum number of results to return. If negative, no
  ///                 limit is imposed.
  ///
  /// \note Sorting is done prior to extracting the result subset, and so
  ///       influences which results are returned by specifying a \p start
  ///       and/or \p maxCount.
  virtual QList<ResultId> results(SortOrder sortOrder, int start = 0,
                                  int maxCount = -1) = 0;

  /// Look up result data node by ID.
  ///
  /// \return Pointer to the requested result node, or \c nullptr if no such
  ///         result exists.
  virtual vdfNodeBase* resultNode(ResultId) const = 0;

  /// Return an item model for this result set.
  ///
  /// This returns a Qt item model for the result set which is suitable for
  /// displaying the results in a Qt item view (e.g. QTreeView). Most users
  /// will want to use this model indirectly via a sort/filter proxy in order
  /// to allow the user to sort the displayed results.
  virtual QAbstractItemModel* resultItemModel() const = 0;

  /// Set result feedback (user score).
  ///
  /// This changes the feedback on the result set by setting the user score on
  /// the specified result. The corresponding result node is also updated.
  ///
  /// \return \c true if the operation was accepted (whether or not the node's
  ///         user score actually changes), otherwise \c false (e.g. if the
  ///         operation is not permitted, or if the requested result was not
  ///         found).
  virtual bool setResultFeedback(ResultId, vvIqr::Classification) = 0;

protected:
  vdfAbstractQueryResultSetNode() {}
};

Q_DECLARE_INTERFACE(vdfAbstractQueryResultSetNode,
                    "org.visgui.framework.data.queryResultSet")

#endif
