// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfNodeProxy_h
#define __vdfNodeProxy_h

#include "vdfNamespace.h"

#include <vgNamespace.h>
#include <vgTimeStamp.h>

#include <qtGlobal.h>

#include <vgExport.h>

#include <QObject>

class vdfNodeBase;
class vdfSelectorSet;

class vdfNodeProxyPrivate;

/// Interface proxy for data nodes.
///
/// This class provides a targeted interface for users of a data node. The
/// purpose of this interface is to marshal per-user requests on the data node,
/// without requiring users to worry about filtering replies by the intended
/// recipient.
///
/// The node proxy itself is generally immutable and stateless, operating on
/// the (mutable) associated data node.
///
/// \sa \ref vdfUpdates
class VG_DATA_FRAMEWORK_EXPORT vdfNodeProxy : public QObject
{
  Q_OBJECT

public:
  /// Status of update request.
  ///
  /// This enumeration describes the current status of
  enum ProgressType
    {
    /// The update() is not yet complete, but new data is available
    UpdateDataAvailable,
    /// The update() is complete, and the requested data is available
    UpdateCompleted,
    /// The update() was discarded
    UpdateDiscarded,
    /// The update() failed for some other reason
    UpdateFailed
    };

  virtual ~vdfNodeProxy();

  /// Return the data node associated with this proxy.
  vdfNodeBase* node() const;

  /// Get current update option flags.
  vdf::UpdateFlags updateMode() const;

  /// Set update option flags.
  void setUpdateMode(vdf::UpdateFlags);

signals:
  /// Notification when the status of a update() request changes.
  ///
  /// \param id Identifier of the most recent request that is satisfied by this
  ///           ready update.
  ///
  /// \note The response identifier corresponds to most recent satisfied
  ///       request which had a valid (non-negative) identifier associated with
  ///       it, and \em not necessarily the most recent request actually
  ///       satisfied. The update may satisfy a later request that did not
  ///       provide a valid identifier.
  void progress(ProgressType, qint64 id) const;

public slots:
  /// Select active data set of associated data node.
  ///
  /// This makes this proxy the active (current) consumer of the data node,
  /// such that the data provided by the node corresponds to the most recent
  /// data retrieved for this consumer. Consumers should always call this
  /// method before attempting to access the data of the associated data node.
  void makeCurrent() const;

  /// Request data from the associated data node.
  ///
  /// This makes a request to the data node using the provided selectors.
  ///
  /// \param selectors Set of selectors controlling what data will be
  ///                  requested.
  /// \param requestId Identifier for the seek request.
  ///
  /// Negative request identifiers are considered 'invalid'; these are still
  /// allowed requests, but the reply will not identify what request was
  /// satisfied. It is an error which shall result in undefined behavior to
  /// issue a request with a valid identifier less than that of another
  /// outstanding request against the same proxy. (Identifiers of requests made
  /// against different proxies have no relation to each other.)
  void update(const vdfSelectorSet& selectors, qint64 requestId = -1) const;

  ////-------------------------------------------------------------------------
  /// \cond internal

protected:
  friend class vdfNodeBase;

  vdfNodeProxy(vdfNodeBase*);

  QTE_DECLARE_PRIVATE_RPTR(vdfNodeProxy)

private:
  QTE_DECLARE_PRIVATE(vdfNodeProxy)
  QTE_DISABLE_COPY(vdfNodeProxy)

  /// \endcond
};

#endif
