/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfDataReader_h
#define __vdfDataReader_h

#include <qtGlobal.h>

#include <vgExport.h>

#include <QObject>

class vdfDataSource;

class vdfDataReaderPrivate;

/// Synchronous data reader.
///
/// This class provides a base class for data
/// In order to function correctly, the data source must be set on the reader
/// before vdfDataSource::start() has been called on the source. To use
/// multiple readers with the same source, set the unstarted source on each
/// reader before calling exec().
///
/// \note
///   The same reader instance can be used to process multiple sources, by
///   calling setSource(vdfDataSource*) and exec() for each source. When this
///   is done, the source will return the union of data from all sources that
///   have been processed in this manner.
class VG_DATA_FRAMEWORK_EXPORT vdfDataReader : public QObject
{
  Q_OBJECT

public:
  /// Construct reader.
  explicit vdfDataReader(QObject* parent = 0);
  virtual ~vdfDataReader();

  /// Get the data source used by this reader.
  virtual vdfDataSource* source() const;

  /// Set the data source used by this reader.
  ///
  /// This attempts to set the data source used by this reader to the specified
  /// data source. The data source must provide an interface which the reader
  /// instance recognizes and can use to receive data. (The specific interface
  /// depends on the vdfDataReader subclass.)
  ///
  /// \return \c true if the source was changed, otherwise \c false.
  ///
  /// \sa connectSource(vdfDataSource*)
  virtual bool setSource(vdfDataSource*);

  /// Test if the reader read any data.
  ///
  /// \return \c true if the reader has received and is able to provide any
  ///         data, otherwise \c false.
  virtual bool hasData() const;

  /// Test if the data source encountered an error.
  ///
  /// \return \c true if the data source status is vdfDataSource::Invalid,
  ///         otherwise \c false.
  virtual bool failed() const;

  /// Read data from the source.
  ///
  /// This starts the data source and executes an event loop until the source's
  /// status changes becomes one of vdfDataSource::Stopped or
  /// vdfDataSource::Invalid. Subclasses connect to the appropriate data source
  /// interface(s) for that subclass in order to store provided data in
  /// internal buffers for later retrieval.
  ///
  /// \return \c true if the source did not encounter an error and the reader
  ///         has (ever; not necessarily from the current source) received data
  ///         (i.e. hasData() returns \c true and failed() returns \c false),
  ///         otherwise \c false.
  virtual bool exec();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vdfDataReader)

  /// Connect the specified data source to the reader.
  ///
  /// This method is called by setSource(vdfDataSource*) to connect the
  /// specified data source to the reader. Subclasses must override this method
  /// to check if the source provides the necessary interface(s) and to connect
  /// the data signals of the same.
  ///
  /// \return \c true if the source could be connected (i.e. provides one or
  ///         more interfaces which the reader subclass can use), otherwise
  ///         \c false.
  virtual bool connectSource(vdfDataSource*) = 0;

  /// Disconnect the specified data source from the reader.
  ///
  /// This method is called by setSource(vdfDataSource*) when the data source
  /// is changed, in order to allow the reader to disconnect from the old
  /// source to prevent receipt of inappropriate data.
  ///
  /// The default implementation disconnects all signals of all interfaces
  /// provided by the data source from any slots of the instance's data
  /// receiver object.
  ///
  /// \sa dataReceiver()
  virtual void disconnectSource(vdfDataSource*);

  /// Get object which receives data signals.
  ///
  /// This method is called by the default implementation of
  /// disconnectSource(vdfDataSource*) to obtain the pointer to the QObject
  /// from which to disconnect data signals. The default implementation returns
  /// \c this. Subclasses that use some other object (e.g. their implementation
  /// private class) to receive data signals should override this method and
  /// return a pointer to the appropriate QObject.
  virtual const QObject* dataReceiver() const;

protected slots:
  /// Release a data source from this reader.
  ///
  /// This releases the reader's data source if the current data source is the
  /// same as the QObject parameter. This is called when the data source is
  /// destroyed in order to ensure that the reader does not retain a reference
  /// to a source that is no longer valid.
  virtual void releaseSource(QObject*);

private:
  QTE_DECLARE_PRIVATE(vdfDataReader)
  QTE_DISABLE_COPY(vdfDataReader)
};

#endif
