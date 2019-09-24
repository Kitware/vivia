/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfEventReader_h
#define __vdfEventReader_h

#include "vdfDataReader.h"

#include <vvEvent.h>

template <class T> class QList;

class vdfEventReaderPrivate;

/// Synchronous event reader.
///
/// This helper class allows a user to synchronously read events from a data
/// source. See vdfDataReader description for more details and usage caveats.
class VG_DATA_FRAMEWORK_EXPORT vdfEventReader : public vdfDataReader
{
  Q_OBJECT

public:
  /// Construct reader, optionally specifying the associated data source.
  ///
  /// \warning Do not pass a source from the constructor of a subclass, as
  ///          doing so may call the wrong implementation of relevant virtual
  ///          methods.
  explicit vdfEventReader(vdfDataSource* source = 0, QObject* parent = 0);
  virtual ~vdfEventReader();

  /// \copydoc vdfDataReader::hasData
  virtual bool hasData() const override;

  /// Get events that have been read.
  ///
  /// This returns a collection of events that have been read by this reader.
  QList<vvEvent> events() const;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vdfEventReader)

  /// \copybrief vdfDataReader::connectSource
  ///
  /// \return \c true if the specified source provides vdfEventSourceInterface
  ///         and was successfully connected, otherwise \c false.
  virtual bool connectSource(vdfDataSource*) override;

private:
  QTE_DECLARE_PRIVATE(vdfEventReader)
  QTE_DISABLE_COPY(vdfEventReader)
};

#endif
