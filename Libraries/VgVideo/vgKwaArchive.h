/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgKwaArchive_h
#define __vgKwaArchive_h

#include <QScopedPointer>
#include <QString>

#include <qtGlobal.h>

#include <vgExport.h>

class QUrl;

class vgKwaVideoClip;

class vgKwaArchivePrivate;

struct vgKwaArchiveRequest
{
  vgKwaArchiveRequest() : StartTime(-1), EndTime(-1), Padding(0) {}

  QString MissionId;
  QString StreamId;
  double StartTime;
  double EndTime;
  double Padding;
};

class VG_VIDEO_EXPORT vgKwaArchive
{
public:
  typedef vgKwaArchiveRequest Request;

  vgKwaArchive();
  virtual ~vgKwaArchive();

  /// Add source to archive.
  ///
  /// Add an additional video source to this archive. A source is usually a
  /// clip, or a file containing a list of clips. Once a source is added, it
  /// can be used to resolve ::findUri and ::findClip requests.
  void addSource(const QUrl& uri);

  /// Retrieve (create) URI of single clip from the archive.
  ///
  /// Attempt to determine the URI of a clip from the archive, based on the
  /// given request. On success, the resulting URI can be used to construct
  /// a new clip, or passed to another user. Note that clips constructed from
  /// a URI are <i>not</i> shared with the archive; they will use additional
  /// resources, but can also be safely owned and accessed by other threads.
  QUrl getUri(const Request&) const;

  /// Retrieve (create) a single clip from the archive.
  ///
  /// Attempt to create a clip from the archive, based on the given request. On
  /// success, the new clip will have a duration that is a subset of (or equal
  /// to) the duration specified in the request. The caller is responsible for
  /// deleting the returned clip. If \p uri is specified, the URI of the clip
  /// will be placed in \p uri.
  ///
  /// \par Note:
  /// To reduce resource allocation, the returned clip's data is <i>shared</i>
  /// with this archive instance. If you need to use the clip in a different
  /// thread, you must take any precautions required by the clip to ensure safe
  /// usage in a multi-threaded environment.
  ///
  /// \sa vgKwaVideoClip::subClip
  vgKwaVideoClip* getClip(const Request&, QUrl* uri = 0) const;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgKwaArchive)

private:
  QTE_DECLARE_PRIVATE(vgKwaArchive)
  Q_DISABLE_COPY(vgKwaArchive)
};

#endif
