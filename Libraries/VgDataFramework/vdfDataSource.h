/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfDataSource_h
#define __vdfDataSource_h

#include "vdfNodeBase.h"

template <typename T> class QSet;

class vdfDataSourceInterface;

class vdfDataSourcePrivate;

/// Base class for data source nodes.
///
/// This class serves as a base class for data source nodes. Unlike most data
/// nodes, data source nodes often do not directly provide data which can be
/// visualized in a scene. Rather, data source nodes provide low level "raw"
/// data that is meant to be received by suitable aggregator nodes, which
/// collect and organize this data for presentation. As such, data notes
/// usually cannot be updated.
///
/// This raw data is provided via one or more source interfaces, which define
/// signals through which the data is supplied. Applications typically create
/// aggregator nodes for those data types supported by the application, which
/// are connected to compatible interfaces provided by a data source node when
/// a new data source node becomes available.
class VG_DATA_FRAMEWORK_EXPORT vdfDataSource : public vdfNodeBase
{
  Q_OBJECT

public:
  /// Generic description of the means by which a source delivers data.
  enum Mechanism
    {
    /// The source is invalid or otherwise does not provide data.
    None,
    /// Data comes from an archive (e.g. file or database).
    Archive,
    /// Data is being produced 'live' from an external source.
    Stream,
    /// Data is produced 'live' via an internal process.
    Process
    };

  /// Source status indicator.
  enum Status
    {
    /// The source has not been started yet.
    ///
    /// All sources start out in an unstarted state and remain that way until
    /// start() has been called.
    Unstarted,
    /// The source is waiting on an external event.
    ///
    /// A pending source has been started but is not yet ready to deliver data.
    /// This frequently happens with streaming sources, e.g. because a
    /// connection to the external provided has not yet been established.
    Pending,
    /// The source is idle.
    ///
    /// No data is currently being delivered by the source, but the source may
    /// deliver data at any time.
    Idle,
    /// The source is active.
    ///
    /// Data is currently being retrieved, manipulated, or delivered by the
    /// source.
    Active,
    /// The source has been suspended.
    ///
    /// Delivery of data has been interrupted. Some sources may allow the user
    /// to temporarily suspend delivery of data.
    Suspended,
    /// The source is permanently stopped.
    ///
    /// The source is no longer able to deliver data. This may happen because a
    /// streaming source has lost its connection to the external provider, or
    /// because an archive source has delivered all available data.
    Stopped,
    /// The source is in an invalid state.
    ///
    /// The source has provided no data and is unable to do so due to an
    /// unrecoverable internal error. Sources in this state should be
    /// considered candidates for automatic culling by the application.
    Invalid
    };

  virtual ~vdfDataSource();

  /// \copydoc vdfNodeBase::type()
  virtual QString type() const QTE_OVERRIDE;

  /// \copybrief vdfNodeBase::canUpdate
  ///
  /// The default implementation returns \c false.
  virtual bool canUpdate() const;

  /// Get the data source's generic type.
  ///
  /// This returns an enumeration value describing, in very broad terms, the
  /// means via which the data source's data originates. The default
  /// implementation returns None, indicating that the source does not provide
  /// data. Subclasses should override this and return an appropriate value.
  virtual vdfDataSource::Mechanism mechanism() const;

  /// Get the data source's status.
  ///
  /// This returns the current status of the data source. The default
  /// implementation returns a value which is initially Unstarted, and can be
  /// changed by the subclass calling setStatus().
  virtual vdfDataSource::Status status() const;

  /// Return data source interfaces provided by this data source.
  QObjectList interfaces() const;

  /// Return specific data source interface.
  ///
  /// This is equivalent to <code>qobject_cast&lt;\p InterfaceType *&gt;(<!--
  /// -->\link vdfDataSource::interface(const QMetaObject*)const interface<!--
  /// -->\endlink(\p InterfaceType<!-- -->::staticMetaObject))</code>.
  template <typename InterfaceType>
  InterfaceType* interface() const;

  /// \copybrief interface()
  ///
  /// \return Pointer to an interface provided by the data source which
  ///         implements the specified interface, or \c nullptr if no such
  ///         interface is available.
  QObject* interface(const QMetaObject*) const;

  /// Return interface types provided by this data source.
  ///
  /// This method returns a collection of QMetaObject's which are the static
  /// metatype objects of the interfaces provided by this data source, as well
  /// as the QMetaObject's for the base classes of the same, excluding the
  /// QMetaObject for QObject.
  QSet<const QMetaObject*> interfaceTypes() const;

  /// Test if data source provides the specified interface.
  ///
  /// This is equivalent to
  /// <code>\link vdfDataSource::hasInterface(const QMetaObject*)const <!--
  /// -->hasInterface<!-- -->\endlink(\p InterfaceType<!--
  /// -->::staticMetaObject)</code>.
  template <typename InterfaceType>
  bool hasInterface() const;

  /// \copybrief hasInterface()
  ///
  /// This method tests if the data source provides an interface whose type
  /// matches the specified QMetaObject, or is a subclass of the type matching
  /// the specified QMetaObject.
  bool hasInterface(const QMetaObject*) const;

  /// Initiate data delivery.
  ///
  /// This method signals the data node that it should begin delivering data.
  /// Data nodes should refrain from delivering data before this method is
  /// called in order to give the application time to establish any necessary
  /// connections to new sources (especially connecting of data source
  /// interfaces to compatible aggregator nodes), in order to avoid data loss.
  virtual void start() = 0;

signals:
  /// Emitted when the source's status changes.
  void statusChanged(vdfDataSource::Status);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vdfDataSource)

  explicit vdfDataSource(QObject* parent);

  /// Add an interface to this data source.
  ///
  /// This creates a new interface of the specified (template parameter)
  /// interface type, and returns the new interface to the caller. The
  /// interface will be included in the list returned by interfaces().
  ///
  /// \p InterfaceType must be a subclass of vdfDataSourceInterface.
  template <typename InterfaceType> InterfaceType* addInterface();

protected slots:
  /// Set the data source's status.
  ///
  /// This slot sets the status returned by the default implementation of
  /// status() and emits statusChanged() if appropriate. It is intended to be
  /// called only by subclasses and should not be called on an instance other
  /// than \c this, or the public class pointer of a private class.
  void setStatus(vdfDataSource::Status);

private:
  QTE_DECLARE_PRIVATE(vdfDataSource)
  QTE_DISABLE_COPY(vdfDataSource)

  void addInterface(QObject* interface, vdfDataSourceInterface* checkType);
};

//-----------------------------------------------------------------------------
template <typename InterfaceType>
InterfaceType* vdfDataSource::interface() const
{
  QObject* const i = this->interface(&InterfaceType::staticMetaObject);
  return qobject_cast<InterfaceType*>(i);
}

//-----------------------------------------------------------------------------
template <typename InterfaceType>
bool vdfDataSource::hasInterface() const
{
  return this->hasInterface(InterfaceType::staticMetaObject);
}

//-----------------------------------------------------------------------------
template <typename InterfaceType>
InterfaceType* vdfDataSource::addInterface()
{
  InterfaceType* const interface = new InterfaceType(this);
  this->addInterface(interface, interface);

  return interface;
}

#endif
