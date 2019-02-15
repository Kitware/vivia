/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfTrackSource_h
#define __vdfTrackSource_h

#include "vdfDataSourceInterface.h"
#include "vdfTrackData.h"
#include "vdfTrackId.h"

#include <vvTrack.h>

#include <vgTimeMap.h>

#include <vgExport.h>

#include <QHash>
#include <QObject>
#include <QSet>

class vdfDataSource;

class vdfTrackSource;

/// Interface for a data source providing tracks
///
/// This class defines a data source interface which provides track data. The
/// track data takes the form of individual or batched track states, closures,
/// and other associated track meta data.
class VG_DATA_FRAMEWORK_EXPORT vdfTrackSourceInterface : public QObject
{
  Q_OBJECT

public:
  virtual ~vdfTrackSourceInterface();

signals:
  /// Emitted when a track name is available.
  ///
  /// This signal is emitted by a track source when the source is able to
  /// provide a name for the track. The name is usually a synthesis of
  /// identifying attributes known about the track and is not expected to
  /// change. The name may be available before any track states have been
  /// emitted.
  ///
  /// The name is intended to be used as a(n initial) display name for the
  /// track.
  void trackNameAvailable(vdfTrackId trackId, QString name);

  /// Emitted when an object classification is available for a track.
  ///
  /// This signal is emitted by a source when it is able to provide an object
  /// classification for a track. Note that the object classification may come
  /// some time after the track has started, after it has been closed, or not
  /// at all. (Additionally, object classifications for the tracks provided by
  /// a particular source may be provided by a different source altogether.)
  void trackClassificationAvailable(vdfTrackId trackId,
                                    vvTrackObjectClassification toc);

  /// Emitted when a track has a new or updated state available.
  ///
  /// This signal is emitted by a track source when it has a new or updated
  /// track state for the specified track. States may arrive in arbitrary
  /// order, and states for multiple tracks may be interleaved. The attributes
  /// and scalar data are for the same time stamp as the track state.
  void trackUpdated(vdfTrackId trackId, vvTrackState state,
                    vdfTrackAttributes attributes,
                    vdfTrackStateScalars scalarData);

  /// Emitted when a track has a new or updated states available.
  ///
  /// This signal is emitted by a track source when it has a new or updated
  /// track states, attributes, and/or scalar data for the specified track. It
  /// is a convenience batching mechanism that allows a source to provide a
  /// group of state updates with less overhead than emitting a signal for each
  /// state. The states all belong to the same track, but may be in any order.
  /// Additional updates may contain states that supersede and/or are
  /// interleaved with states from this batch, and batches for multiple tracks
  /// may be interleaved. Similarly, the attributes and scalar data (which are
  /// delivered as ordered maps) may or may not match the time stamps of the
  /// track states.
  void trackUpdated(vdfTrackId trackId, QList<vvTrackState> states,
                    vgTimeMap<vdfTrackAttributes> attributes,
                    vdfTrackScalarDataCollection scalarData);

  /// Emitted when a track is closed (terminated).
  ///
  /// This signal is emitted by a track source when a track is "closed",
  /// indicating that the provider has determined that the track has ended.
  ///
  /// The difference between an "open" and "closed" track is mainly used for
  /// display purposes when selecting tracks that overlap a given time
  /// interval; the end time of an "open" track has not been determined and is
  /// usually taken to be 'infinity'.
  void trackClosed(vdfTrackId trackId);

private:
  friend class vdfTrackSource;

  explicit vdfTrackSourceInterface(QObject* parent);
};

/// Helper class for data sources providing vdfTrackSourceInterface.
///
/// This class provides a wrapper around vdfTrackSourceInterface which can be
/// used by data source implementations to provide track data. The wrapper
/// class exposes the interface signals with \c public access protection,
/// allowing them to be emitted by the data source implementation.
///
/// This wrapper class may only be constructed via
/// vdfDataSource::addInterface&lt;vdfDataSource&gt;().
class VG_DATA_FRAMEWORK_EXPORT vdfTrackSource :
  public vdfTrackSourceInterface, public vdfDataSourceInterface
{
public:
  virtual ~vdfTrackSource();

  using vdfTrackSourceInterface::trackNameAvailable;
  using vdfTrackSourceInterface::trackUpdated;
  using vdfTrackSourceInterface::trackClosed;

private:
  friend class vdfDataSource;

  explicit vdfTrackSource(QObject* parent);
};

#endif
