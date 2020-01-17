/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfTrackReader_h
#define __vdfTrackReader_h

#include "vdfDataReader.h"
#include "vdfTrackData.h"
#include "vdfTrackId.h"

#include <vgTimeMap.h>
#include <vvTrack.h>

#include <QHash>
#include <QSet>

class vdfTrackReaderPrivate;

/// Synchronous track reader.
///
/// This helper class allows a user to synchronously read tracks from a data
/// source. See vdfDataReader description for more details and usage caveats.
class VG_DATA_FRAMEWORK_EXPORT vdfTrackReader : public vdfDataReader
{
  Q_OBJECT

public:
  struct Track
    {
    QString Name;
    vvTrackObjectClassification Classification;
    vgTimeMap<vvTrackState> Trajectory;
    vgTimeMap<vdfTrackAttributes> Attributes;
    QHash<QString, vdfTrackScalarData> ScalarData;
    };

public:
  /// Construct reader, optionally specifying the associated data source.
  ///
  /// \warning Do not pass a source from the constructor of a subclass, as
  ///          doing so may call the wrong implementation of relevant virtual
  ///          methods.
  explicit vdfTrackReader(vdfDataSource* source = 0, QObject* parent = 0);
  virtual ~vdfTrackReader();

  /// \copydoc vdfDataReader::hasData
  virtual bool hasData() const QTE_OVERRIDE;

  /// Get tracks that have been read.
  ///
  /// This returns a collection of tracks that have been read by this reader.
  QHash<vdfTrackId, Track> tracks() const;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vdfTrackReader)

  /// \copybrief vdfDataReader::connectSource
  ///
  /// \return \c true if the specified source provides vdfTrackSourceInterface
  ///         and was successfully connected, otherwise \c false.
  virtual bool connectSource(vdfDataSource*) QTE_OVERRIDE;

  /// \copybrief vdfDataReader::dataReceiver
  virtual const QObject* dataReceiver() const QTE_OVERRIDE;

private:
  QTE_DECLARE_PRIVATE(vdfTrackReader)
  QTE_DISABLE_COPY(vdfTrackReader)
};

#endif
