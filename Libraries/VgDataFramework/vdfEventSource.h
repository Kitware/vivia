/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfEventSource_h
#define __vdfEventSource_h

#include "vdfDataSourceInterface.h"

#include <vvEvent.h>

#include <vgExport.h>

#include <QObject>

class vdfDataSource;

class vdfEventSource;

/// Interface for a data source providing events
///
/// This class defines a data source interface which provides event data. The
/// event data takes the form of batched events. Sources which are not normally
/// capable of batching will provide "batches" of single events.
class VG_DATA_FRAMEWORK_EXPORT vdfEventSourceInterface : public QObject
{
  Q_OBJECT

public:
  virtual ~vdfEventSourceInterface();

signals:
  /// Emitted when one or more events are available.
  ///
  /// This signal is emitted by an event source when it has one or more new
  /// events available. Events are in arbitrary order.
  void eventsAvailable(QList<vvEvent> events);

private:
  friend class vdfEventSource;

  explicit vdfEventSourceInterface(QObject* parent);
};

/// Helper class for data sources providing vdfEventSourceInterface.
///
/// This class provides a wrapper around vdfEventSourceInterface which can be
/// used by data source implementations to provide events. The wrapper class
/// exposes the interface signals with \c public access protection, allowing
/// them to be emitted by the data source implementation.
///
/// This wrapper class may only be constructed via
/// vdfDataSource::addInterface&lt;vdfDataSource&gt;().
class VG_DATA_FRAMEWORK_EXPORT vdfEventSource :
  public vdfEventSourceInterface, public vdfDataSourceInterface
{
public:
  virtual ~vdfEventSource();

  using vdfEventSourceInterface::eventsAvailable;

private:
  friend class vdfDataSource;

  explicit vdfEventSource(QObject* parent);
};

#endif
