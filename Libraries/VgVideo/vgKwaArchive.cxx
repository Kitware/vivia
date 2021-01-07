// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgKwaArchive.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QMap>
#include <QRegExp>
#include <QSet>
#include <QUrl>
#include <QUrlQuery>

#include <vgCheckArg.h>

#include "vgKwaVideoClip.h"
#include "vgKwaUtil.h"

static const char* RE_LooksLikeUri = "^[a-zA-Z0-9]{2,}:";

static const char* ERROR_PREFIX = "vgKwaArchive:";

QTE_IMPLEMENT_D_FUNC(vgKwaArchive)

//BEGIN vgKwaArchivePrivate

//-----------------------------------------------------------------------------
class vgKwaArchivePrivate
{
public:
  void addSource(const QUrl& uri, const QString& referencePath);
  void addArchive(QFile& file, const QString& versionString,
                  const QString& path);
  void addClip(const QUrl& uri);

  vgKwaVideoClip* findClip(const vgKwaArchive::Request& request) const;

  QUrl generateUri(vgKwaVideoClip* clip,
                   double resolvedStartTime,
                   double resolvedEndTime) const;

  QDebug debug() const { return qDebug().nospace() << ERROR_PREFIX; }

  QSet<vgKwaVideoClip*> clips;
  QHash<vgKwaVideoClip*, QUrl> clipUris;
  QHash<QString, QList<vgKwaVideoClip*> > clipsByMissionId;
};

//-----------------------------------------------------------------------------
void vgKwaArchivePrivate::addSource(
  const QUrl& uri, const QString& referencePath)
{
  // Only local files supported for now
  if (uri.scheme() != "file")
    {
    this->debug() << " unable to open requested source " << uri
                  << ": the scheme is not supported";
    }

  // Resolve path, if given a reference path
  QString path = uri.toLocalFile();
  if (!referencePath.isEmpty())
    {
    if (!vgKwaUtil::resolvePath(path, referencePath, "source"))
      {
      // resolvePath() will have displayed a suitable error
      return;
      }
    }

  // Open the source file
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    this->debug() << " unable to open requested source " << uri
                  << ": " << qPrintable(file.errorString());
    return;
    }

  // Read the header and parse the file
  const QString header = QString::fromLocal8Bit(file.readLine()).trimmed();
  if (header.toLower().startsWith("archive"))
    {
    this->addArchive(file, header.mid(7), path);
    }
  else
    {
    QFileInfo fi(path);
    this->addClip(QUrl::fromLocalFile(fi.canonicalFilePath()));
    }
}

//-----------------------------------------------------------------------------
void vgKwaArchivePrivate::addArchive(
  QFile& file, const QString& versionString, const QString& path)
{
  // Get version number
  bool okay;
  int version = (versionString + '0').toInt(&okay) / 10;
  if (!okay)
    {
    this->debug() << " error loading source archive " << path
                  << ": could not parse archive version number "
                  << versionString;
    return;
    }

  if (version < 0 || version > 1)
    {
    this->debug() << " error loading source archive " << path
                  << ": version " << versionString << " is not supported";
    return;
    }

  // Load referenced sources
  while (!file.atEnd())
    {
    // Read line and remove trailing newline
    QString source = QString::fromLocal8Bit(file.readLine());
    if (source.endsWith('\n'))
      {
      source = source.left(source.length() - 1);
      }

    // Convert to URI
    const bool looksLikeUri = QRegExp(RE_LooksLikeUri).exactMatch(source);
    const QUrl uri = (looksLikeUri ? QUrl::fromUserInput(source) :
                                     QUrl::fromLocalFile(source));

    // Hand back to addSource (recursive archives okay!)
    this->addSource(uri, path);
    }
}

//-----------------------------------------------------------------------------
void vgKwaArchivePrivate::addClip(const QUrl& uri)
{
  // Attempt to load clip
  QScopedPointer<vgKwaVideoClip> clip(new vgKwaVideoClip(uri));
  if (!clip->firstTime().IsValid() || !clip->lastTime().IsValid())
    {
    // Something went wrong; clip should have displayed a suitable error
    return;
    }

  // Add clip to clip list and maps
  this->clipsByMissionId[clip->missionId()].append(clip.data());
  this->clipUris.insert(clip.data(), uri);
  this->clips.insert(clip.take());
}

//-----------------------------------------------------------------------------
vgKwaVideoClip* vgKwaArchivePrivate::findClip(
  const vgKwaArchive::Request& request) const
{
  CHECK_ARG(request.EndTime >= request.StartTime, 0);
  CHECK_ARG(this->clipsByMissionId.contains(request.MissionId), 0);

  const double requestStart = request.StartTime;
  const double requestEnd = request.EndTime;

  vgKwaVideoClip* result = 0;
  double bestMatch = 0.0;
  const QList<vgKwaVideoClip*>& candidates =
    this->clipsByMissionId[request.MissionId];

  // Search for "best" match, defined as the match that will give us the longest
  // actual clip (in case the temporal limits do not fit entirely within any
  // single clip, but partly overlap more than one), not counting padding
  foreach (vgKwaVideoClip* clip, candidates)
    {
    // Check for stream ID match
    if (!(request.StreamId.isEmpty() || clip->streamId().isEmpty()))
      {
      if (request.StreamId != clip->streamId())
        {
        // Skip clip if stream ID was requested and does not match clip
        continue;
        }
      }

    const double clipStart = clip->firstTime().Time;
    const double clipEnd = clip->lastTime().Time;

    // Check for overlap
    if (clipStart <= requestEnd || clipEnd >= requestStart)
      {
      if (clipStart <= requestStart && clipEnd >= requestEnd)
        {
        // Clip wholly contains the request; looks like a winner
        return clip;
        }
      const double matchLength =
        qMin(clipEnd, requestEnd) - qMax(clipStart, requestStart);
      if (matchLength > bestMatch)
        {
        // Better match than we've found so far...
        result = clip;
        bestMatch = matchLength;
        }
      }
    }

  return result;
}

//-----------------------------------------------------------------------------
QUrl vgKwaArchivePrivate::generateUri(
  vgKwaVideoClip* clip, double resolvedStartTime, double resolvedEndTime) const
{
  // Generate URI for the request
  auto result = this->clipUris.value(clip);
  auto query = QUrlQuery{result};

  const qint64 uriStartTime = qRound64(resolvedStartTime);
  const qint64 uriEndTime = qRound64(resolvedEndTime);
  query.addQueryItem("StartTime", QString::number(uriStartTime));
  query.addQueryItem("EndTime", QString::number(uriEndTime));

  result.setQuery(query);
  return result;
}

//END vgKwaArchivePrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgKwaArchive

//-----------------------------------------------------------------------------
vgKwaArchive::vgKwaArchive() : d_ptr(new vgKwaArchivePrivate)
{
}

//-----------------------------------------------------------------------------
vgKwaArchive::~vgKwaArchive()
{
  QTE_D(vgKwaArchive);
  qDeleteAll(d->clips);
}

//-----------------------------------------------------------------------------
void vgKwaArchive::addSource(const QUrl& uri)
{
  QTE_D(vgKwaArchive);
  d->addSource(uri, QString());
}

//-----------------------------------------------------------------------------
QUrl vgKwaArchive::getUri(const Request& request) const
{
  QTE_D_CONST(vgKwaArchive);
  vgKwaVideoClip* clip = d->findClip(request);

  if (clip)
    {
    // Resolve padding
    double resolvedStartTime = request.StartTime;
    double resolvedEndTime = request.EndTime;
    if (clip->resolvePadding(resolvedStartTime, resolvedEndTime,
                             request.Padding))
      {
      return d->generateUri(clip, resolvedStartTime, resolvedEndTime);
      }
    }

  return QUrl();
}

//-----------------------------------------------------------------------------
vgKwaVideoClip* vgKwaArchive::getClip(const Request& request, QUrl* uri) const
{
  QTE_D_CONST(vgKwaArchive);
  vgKwaVideoClip* clip = d->findClip(request);

  if (clip)
    {
    if (uri)
      {
      // Resolve padding
      double resolvedStartTime = request.StartTime;
      double resolvedEndTime = request.EndTime;
      if (clip->resolvePadding(resolvedStartTime, resolvedEndTime,
                               request.Padding))
        {
        *uri = d->generateUri(clip, resolvedStartTime, resolvedEndTime);
        return new vgKwaVideoClip(*clip, resolvedStartTime, resolvedEndTime);
        }
      }
    else
      {
      return clip->subClip(request.StartTime, request.EndTime,
                           request.Padding);
      }
    }

  return 0;
}

//END vgKwaArchive
