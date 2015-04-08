/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfTrackId_h
#define __vdfTrackId_h

#include <vgExport.h>

#include <QUuid>

class vdfDataSource;

/// Track logical identifier.
///
/// This structure defines a complete logical identifier for a track. Logical
/// identifiers are used to reference tracks where a well known and well
/// defined identifier is needed (such as when independently providing track
/// related data from multiple sources), as opposed to the unique but arbitrary
/// internal model identifier.
struct VG_DATA_FRAMEWORK_EXPORT vdfTrackId
{
  /// Construct a default (invalid) track identifier.
  vdfTrackId();

  /// Construct an identifier with the specified values.
  vdfTrackId(vdfDataSource* source, long long serialNumber,
             const QUuid& uniqueId = QUuid());

  /// Construct an identifier with the specified values.
  vdfTrackId(vdfDataSource* source, int provider,
             long long serialNumber, const QUuid& uniqueId = QUuid());

  /// Compare two vdfTrackId instances.
  ///
  /// \return \c true if the specified track ID is the same as this instance
  ///         (operator== returns \c true for all data members), otherwise
  ///         \c false.
  bool operator==(const vdfTrackId&) const;

  /// Pointer to the data source which provided the track.
  ///
  /// This identifies the specific data source instance which provided the
  /// track. For track sources, this is a pointer to the data source node which
  /// owns the source interface.
  vdfDataSource* Source;

  /// Unique identifier of the track.
  ///
  /// This universally unique identifier, if available, can be used to identify
  /// a specific track across different application instances or even different
  /// machines.
  ///
  /// A unique identifier may not be available, in which case the QUuid will be
  /// invalid (QUuid::isValid() returns \c false).
  QUuid UniqueId;

  /// Numerical identifier of the process which produced this track.
  ///
  /// This is used to differentiate tracks from different producers which may
  /// be using independent pools to assign serial numbers (which, as a result,
  /// are not unique). It is intended to uniquely identify a provider within a
  /// tracking session.
  ///
  /// If a provider instance is not available, the value will be \c -1.
  ///
  /// The exact meaning of this value is dependent on the track source. Some
  /// sources will not give provider identifiers, while others may use bit
  /// packing to combine multiple pieces of information into this field.
  int Provider;

  /// Provider-unique track identifier.
  ///
  /// This gives a serial number for the track, which uniquely identifies a
  /// track from a specific provider. The value is always non-negative for a
  /// valid track identifier; a negative value indicates an invalid identifier.
  long long SerialNumber;
};

extern VG_DATA_FRAMEWORK_EXPORT uint qHash(const vdfTrackId&);

#endif
