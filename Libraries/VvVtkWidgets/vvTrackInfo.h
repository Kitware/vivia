// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvTrackInfo_h
#define __vvTrackInfo_h

#include <QString>
#include <QList>

#include <vgExport.h>

#include <vtkVgTrack.h>

#include <vgColor.h>

struct VV_VTKWIDGETS_EXPORT vvTrackInfo
{
  vvTrackInfo();
  explicit vvTrackInfo(vtkVgTrack::enumTrackPVOType);

  /// Save track information to settings.
  ///
  /// This saves the track information held by this structure to the user's
  /// configuration.
  bool write() const;

  int Source;
  vtkVgTrack::enumTrackPVOType Type;
  QString Name;
  vgColor PenColor;
  vgColor ForegroundColor;
  vgColor BackgroundColor;

public:
  static bool getTrackType(vvTrackInfo& ti, vtkVgTrack::enumTrackPVOType type);
  static bool getTrackSource(vvTrackInfo& ti, int source);

  /// Read per-type track information from settings.
  ///
  /// This returns a list of vvTrackInfo structures describing possible track
  /// classifications. The information is read from the user's configuration,
  /// using a set of 'canned' default values.
  static QList<vvTrackInfo> trackTypes();

  /// Read per-source track information from settings.
  ///
  /// This returns a list of vvTrackInfo structures describing possible track
  /// sources. The information is read from the user's configuration. There are
  /// no default track sources; if the user has not configured any track
  /// sources, this list will be empty.
  static QList<vvTrackInfo> trackSources();

  /// Remove a track source from settings.
  ///
  /// This erases the user configuration keys for the specified track source,
  /// so that it is no longer returned by trackSources().
  static void eraseTrackSource(int source);
};

#endif
