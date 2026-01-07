// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgKwaVideoClip_h
#define __vgKwaVideoClip_h

#include <vgExport.h>

#include "vgVideo.h"

class QUrl;

class vgKwaFrameMetadata;

class vgKwaVideoClipPrivate;

class VG_VIDEO_EXPORT vgKwaVideoClip : public vgVideo
{
public:
  typedef vgTimeMap<vgKwaFrameMetadata> MetadataMap;

  explicit vgKwaVideoClip(const QUrl& indexUri);
  virtual ~vgKwaVideoClip();

  QString missionId() const;
  QString streamId() const;

  // Metadata accessors
  vgKwaFrameMetadata currentMetadata() const;
  vgKwaFrameMetadata metadataAt(
    vgTimeStamp pos, vg::SeekMode direction = vg::SeekNearest) const;

  MetadataMap metadata() const;

  /// Extract a sub-clip from this clip.
  ///
  /// Create a new clip that is a subset of this clip. The returned clip will
  /// contain no frames before \p startTime or after \p endTime. If \p startTime
  /// and \p endTime are both \c -1, the returned clip will be a clone of this
  /// clip. The caller is responsible for deleting the returned clip.
  ///
  /// \par Note:
  /// The new clip shares internal resources with this clip. To prevent errors,
  /// users must ensure that all attempts to access image data from either clip
  /// are serialized.
  vgKwaVideoClip* subClip(double startTime, double endTime,
                          double padding) const;

  /// Resolve times with padding applied.
  ///
  /// Calculate actual start and end times from base times plus padding. After
  /// resolution, the start and end times will lie within the times actually
  /// contained in the clip, and the limits (if within the clip) will be grown
  /// by either \p padding, or until the edge of a "shot", whichever is smaller.
  /// (A "shot" is defined as the contiguous region between homography breaks.)
  ///
  /// Negative padding will be treated as zero. If the input range is invalid
  /// (both limits equal to \c -1), the limits will be set to the complete
  /// extent of the clip.
  ///
  /// \return \c true if resolution succeeds, \c false otherwise (e.g. if the
  /// input range does not overlap the clip's actual range).
  ///
  /// \par Note:
  /// For performance reasons, if this clip does not have external metadata,
  /// no padding will be applied.
  bool resolvePadding(double& startTime, double& endTime,
                      double padding) const;

protected:
  friend class vgKwaArchive;

  explicit vgKwaVideoClip(const vgKwaVideoClip& other,
                          double startTime, double endTime);

private:
  QTE_DECLARE_PRIVATE(vgKwaVideoClip)
  Q_DISABLE_COPY(vgKwaVideoClip)
};

#endif
