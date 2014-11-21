/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfNodeBase_h
#define __vdfNodeBase_h

#include "vdfNamespace.h"

#include <vgNamespace.h>

#include <qtGlobal.h>

#include <vgExport.h>

#include <QObject>

template <typename T> class QSet;

struct vgTimeStamp;

class vdfNodeProxy;
class vdfSelector;
class vdfSelectorSet;
class vdfSelectorType;

class vdfNodeBasePrivate;

/// Base class for data tree nodes.
///
/// This class defines the minimum interface implemented by a node in a VisGUI
/// data tree. Concrete data nodes implement interfaces to provide data for
/// visualization, or to enable interaction with the node. Typically, a scene
/// tree is constructed by walking a data tree, querying each node for
/// supported interfaces, and building representations based on what interfaces
/// each node implements.
///
/// Not every node will implement usable interfaces. The root node in
/// particular is often actually a vdfNodeBase (i.e. not a subclass thereof).
///
/// Management of the node tree is handled by QObject. Consumers should be
/// careful to allow for their associated data nodes to be destroyed at any
/// time.
class VG_DATA_FRAMEWORK_EXPORT vdfNodeBase : public QObject
{
  Q_OBJECT

  /// Node display name.
  ///
  /// This gives a descriptive name of this specific data node, which can
  /// normally be changed by the user. The name describes the content of the
  /// data node, and may for example refer to a specific data set. As suggested
  /// by the name, this property is most typically used when presenting the
  /// node to the user, e.g. when showing information about the node, or a tree
  /// view of data nodes.
  ///
  /// \sa setName()
  Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName
                                 NOTIFY displayNameChanged)

  /// Node type.
  ///
  /// This gives a description of the type of data node, which cannot be
  /// changed. It is intended for use in a user interface.
  Q_PROPERTY(QString type READ type CONSTANT)

  /// Node user visibility state.
  ///
  /// The user visibility state provides a mechanism for the user to hide a
  /// set of data from all views. Representations of a node should be visible
  /// iff the representation's user visibility state is \c true, the data
  /// node's user visibility state is \c true, and the data node is not hidden.
  ///
  /// \sa isVisible(), setVisible()
  Q_PROPERTY(bool visible READ isVisible WRITE setVisible
                          NOTIFY visibilityChanged)

  /// Node 'hidden' state.
  ///
  /// Node hiding is used to avoid presenting the user with unnecessary
  /// information, and may be done for a variety of reasons. A hidden node
  /// should not be shown to the user, even to make it visible or change its
  /// display name.
  ///
  /// One reason to hide nodes is used by dynamic data nodes which must be
  /// created to handle types of data which may be received by sources,
  /// allowing them to 'lurk' until and unless such a source is actually
  /// created.
  ///
  /// Another reason is to hide internal child nodes, especially the individual
  /// source types of source nodes, for which only the primary source node
  /// should be visible to the user.
  ///
  /// \sa isHidden()
  Q_PROPERTY(bool hidden READ isHidden)

public:
  vdfNodeBase(QObject* parent = 0);
  virtual ~vdfNodeBase();

  /// Get parent data node.
  ///
  /// \return Pointer to the parent vdfNodeBase, or \c nullptr if the data node
  ///         has no parent.
  ///
  /// As the parent data node is by definition also the parent QObject, this
  /// method is shorthand for qobject_cast&lt;vdfNodeBase*&gt;(parent()).
  inline vdfNodeBase* parentNode() const
    { return qobject_cast<vdfNodeBase*>(this->parent()); }

  /// Get node display name.
  /// \sa displayName
  QString const displayName() const;

  /// Get node type.
  /// \sa type
  virtual QString type() const;

  /// Get user visibility state.
  /// \sa visible, setVisible()
  virtual bool isVisible() const;

  /// Get if node is hidden.
  ///
  /// \return \c true if node is hidden, otherwise \c false.
  ///
  /// The default implementation returns \c false. Usually only dynamic nodes
  /// will need to override this.
  ///
  /// \sa hidden
  virtual bool isHidden() const;

  /// Get if the user can change the node's visibility.
  ///
  /// The default implementation returns \c true. Usually only data source
  /// nodes that feed aggregator nodes and do not directly provide data for
  /// visualization should return \c false.
  virtual bool canChangeVisibility() const;

  /// Get if the user can delete the node.
  ///
  /// \return \c true if the user should be allowed to request deletion of the
  ///         node, otherwise \c false.
  ///
  /// The default implementation returns \c true. Note that the node must still
  /// support being deleted due to its parent being deleted.
  virtual bool canDelete() const;

  /// Get node's supported selectors.
  ///
  /// This gives a collection of pointers to the meta-objects of all selector
  /// types which are supported by this data node.
  ///
  /// \sa \ref vdfNodeTypes
  virtual QSet<vdfSelectorType> supportedSelectors() const;

  /// Query if a node supports a particular selector type.
  ///
  /// \return \c true if the node supports selectors of the specified type,
  ///         otherwise \c false.
  ///
  /// \sa supportedSelectors()
  bool isSelectorSupported(const vdfSelectorType&) const;

  /// Query if a node supports a particular selector type.
  ///
  /// \return \c true if the node supports selectors of the same type as
  ///         \p selector, otherwise \c false.
  ///
  /// \sa supportedSelectors()
  bool isSelectorSupported(const vdfSelector* selector) const;

  /// Establish a connection to this data node.
  ///
  /// This creates a connection from an arbitrary \p consumer to the data node,
  /// in the form of a vdfNodeProxy, which can be used to request data from the
  /// node. The connection is automatically destroyed, along with any
  /// associated per-consumer data, when either this data node or the specified
  /// \p consumer is destroyed.
  ///
  /// The \p consumer must not be null. If a valid \p consumer is not
  /// specified, the connection attempt will fail and return \c nullptr.
  ///
  /// Establishing a connection also calls enter().
  virtual vdfNodeProxy* connect(QObject* consumer);

  // Don't hide Qt signal/slot methods
  using QObject::connect;
  using QObject::disconnect;

signals:
  /// Notification when the node's display name changes.
  /// \sa displayName
  void displayNameChanged(const QString&);

  /// Notification when the node's user visibility state changes.
  /// \sa visible
  void visibilityChanged(bool);

  /// Notification when a child node is added.
  ///
  /// This signal is emitted when a data node is newly added as a child to this
  /// data node. The signal parameter is a pointer to the new node.
  ///
  /// \note Internally, new children are collected and - if they have not been
  ///       deleted in the mean time - processed at the next iteration of the
  ///       application event loop. This ensures that the new node is fully
  ///       constructed when this signal is emitted, although it is possible
  ///       for a child to be added and immediately destroyed without
  ///       triggering this notification.
  void childAdded(vdfNodeBase*);

  /// Notification when a node's data has changed.
  ///
  /// This signal is emitted whenever a data node's data has changed. This
  /// could be a change to the underlying raw data, or to the presented data
  /// due to a change in a data filter. Such changes generally recommend that
  /// associated scene nodes should be updated.
  ///
  /// \note Emission of this signal does not mean that the new data is ready
  ///       for immediate consumption. Consumers should request that the node
  ///       update for their current time stamp in order to obtain the updated
  ///       data. (The node may still discard such a request if the new data
  ///       that caused this signal to be emitted is not relevant to that
  ///       consumer.)
  void modified();

public slots:
  /// Destroy connection to this data node.
  ///
  /// This removes the specified connection to this data node, along with any
  /// per-connection data belonging to the data node. Once called, the
  /// specified connection is invalid and must not be used.
  ///
  /// Releasing a connection also calls leave().
  virtual void disconnect(vdfNodeProxy*);

  /// Begin using this node for a new consumer.
  ///
  /// This slot increments a data node's count of consumers that are using the
  /// node. This may cause the node to construct additional structures (e.g.
  /// data models) that may be required to visualize the node.
  ///
  /// Normally, this is called when a consumer connects to the node using
  /// connect(). Consumers that use the node without a connection \b must call
  /// this slot before attaching any representations to the node.
  void enter();

  /// Release this node from a consumer.
  ///
  /// This slot decrements a data node's count of consumers that are using the
  /// node. When this count reaches zero, the node may choose to release
  /// resources that are not needed when the node is not being visualized in
  /// order to conserve memory or otherwise reduce load on the system.
  ///
  /// This is also called when a connection is removed from the node. Consumers
  /// should only need to call this directly if calling enter() directly.
  void leave();

  /// Set the display name of this data node.
  /// \sa name
  void setDisplayName(const QString&);

  /// Set the user visibility state of this data node.
  ///
  /// The user visibility state provides a mechanism for the user to hide a
  /// data tree in all views.
  ///
  /// \return \c true if the request to change state was accepted (whether or
  ///         not the visibility state actually changed), otherwise \c false.
  ///
  /// Normally, this method will return \c false iff canChangeVisibility()
  /// would return \c false. Note that the default implementation enforces
  /// this, so that nodes that do not permit changing their visibility do not
  /// normally need to override this method to prevent visibility changes.
  virtual bool setVisible(bool);

protected:
  /// Begin using this node for visualization.
  ///
  /// This method is called when a consumer requests use of a node that is not
  /// currently being used by any consumers. The default implementation does
  /// nothing. Nodes that lazy-construct models or other resources needed to
  /// visualize the node should construct and/or initialize such resources in
  /// this method.
  ///
  /// \sa enter()
  virtual void acquire();

  /// End use of this node for visualization.
  ///
  /// This method is called when the last consumer detaches from a node. The
  /// default implementation does nothing. Nodes that release visualization
  /// resources when they are unneeded may do so in this method.
  ///
  /// \sa leave()
  virtual void release();

  /// Handle addition of a new child node.
  ///
  /// This method is called after a new child node is added to this data node,
  /// to allow the parent node to perform any necessary special action in
  /// reaction to the addition. The default implementation is responsible for
  /// emitting the childAdded() signal, so classes that reimplement this must
  /// be careful to call the base implementation.
  ///
  /// Processing of new children is deferred until the next iteration of the
  /// event loop, so that they are fully constructed when this method is
  /// called. Children that are deleted before this will not be processed.
  ///
  /// \sa childAdded()
  virtual void addChild(vdfNodeBase*);

  /// Request that the data node update to the specified time stamp.
  ///
  /// This dispatches an update request for the specified consumer (as
  /// identified by its connection proxy).
  ///
  /// \sa vdfNodeProxy::update()
  virtual void update(const vdfNodeProxy*, qint64 requestId,
                      const vdfSelectorSet&, vdf::UpdateFlags);

  /// Select active data set.
  ///
  /// This selects the active data set for this node to be the data set
  /// for the specified consumer (as identified by its connection proxy).
  ///
  /// \sa vdfNodeProxy::select()
  virtual void select(const vdfNodeProxy*);

  ////-------------------------------------------------------------------------
  /// \cond internal

protected slots:
  void updateChildren();

protected:
  friend class vdfNodeProxy;

  virtual void childEvent(QChildEvent*) QTE_OVERRIDE;

  QTE_DECLARE_PRIVATE_RPTR(vdfNodeBase)

private:
  QTE_DECLARE_PRIVATE(vdfNodeBase)
  QTE_DISABLE_COPY(vdfNodeBase)

  /// \endcond
};

#endif
