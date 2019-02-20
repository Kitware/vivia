/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqQueryParser.h"

#include <QDir>
#include <QFileInfo>
#include <QUrlQuery>

#include <qtKstReader.h>
#include <qtStlUtil.h>

#include <vvKstReader.h>
#include <vvKstWriter.h>

#include <vgCheckArg.h>

#include <vvQueryService.h>

#include "vqDebug.h"
#include "vqScopedOverrideCursor.h"
#include "vqSettings.h"

namespace // anonymous
{

//-----------------------------------------------------------------------------
bool checkCacheDir(const QDir& cacheDir)
{
  // Try to create cache directory if it does not exist
  if (!cacheDir.exists())
    {
    if (!cacheDir.mkpath(cacheDir.absolutePath()))
      {
      qtDebug(vqdQueryParserCache)
          << "unable to update cache; cache directory" << cacheDir.path()
          << "does not exist, and we were unable to create it";
      return false;
      }
    }

  return true;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
struct vqQueryParser::Internal
{
  Internal();
  ~Internal();

  QUrl server_;
  QUrl cacheLocation_;
  QUrl DescriptorCacheFile;
  QUrl TrackCacheFile;
  QScopedPointer<vvQuerySession> session_;
};

//-----------------------------------------------------------------------------
vqQueryParser::Internal::Internal()
  : session_(0)
{
  vqSettings settings;
  this->cacheLocation_ = settings.queryCacheUri();
}

//-----------------------------------------------------------------------------
vqQueryParser::Internal::~Internal()
{
  if (this->session_)
    {
    this->session_->shutdown();
    }
}

//-----------------------------------------------------------------------------
vqQueryParser::vqQueryParser(const QUrl& queryServer, const QUrl& cacheUri)
  : internal_(new vqQueryParser::Internal)
{
  this->internal_->server_ = queryServer;
  if (!cacheUri.isEmpty())
    {
    this->internal_->cacheLocation_ = cacheUri;
    }
}

//-----------------------------------------------------------------------------
vqQueryParser::~vqQueryParser()
{
}

//-----------------------------------------------------------------------------
bool vqQueryParser::loadQuery(const QUrl& queryUri)
{
  if (queryUri.scheme().toLower() == "file")
    {
    // Open saved query file
    vvReader reader;
    vvHeader header;

    if (!(reader.open(queryUri) && reader.readHeader(header)))
      {
      return this->syncAbort("Unable to read file " + queryUri.toString());
      }

    if (header.type == vvHeader::QueryResults)
      {
      // Read results from file
      int count = 0;
      while (!reader.atEnd())
        {
        vvQueryResult qre;

        // vvReader (by design) won't set fields that don't exist (i.e. due to
        // the file being an old format version), so set them up front... for
        // now, instance ID is the only one that needs a value other than what
        // is set by the vvQueryResult default ctor
        qre.InstanceId = count;

        // Read result
        if (!reader.readQueryResult(qre))
          {
          return this->asyncAbort(reader.error());
          }

        emit this->resultAvailable(qre);
        ++count;
        }
      this->postStatus("Successfully read " + QString::number(count) +
                       " saved results");
      emit this->resultSetComplete();
      emit this->finished();
      return true;
      }
    else if (header.type == vvHeader::QueryPlan)
      {
      // Read query plan
      vvQueryInstance query;
      if (!reader.readQueryPlan(query))
        {
        return this->syncAbort(reader.error());
        }

      emit this->planAvailable(query);
      emit this->finished();
      return true;
      }
    return this->syncAbort("Unsupported file format");
    }
  else
    {
    return this->syncAbort("Unsupported scheme for URI "
                           + queryUri.toString());
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vqQueryParser::formulateQuery(
  vvProcessingRequest request, bool bypassCache)
{
  if (!this->internal_->cacheLocation_.isEmpty())
    {
    // Limit override cursor only for this scope (using RAII)
    vqScopedOverrideCursor overrideCursor;

    // Get the path where we expect to find the cache; even if bypassCache is
    // enabled, we will use it later to write out the new results
    // \TODO something more robust against duplicate names, version changes,
    //       etc.
    QUrl videoUri = qtUrl(request.VideoUri);
    qtDebug(vqdQueryParserCache)
        << "determining cache file URI for cache"
        << this->internal_->cacheLocation_
        << "video" << videoUri;

    QFileInfo vfi(videoUri.path());
    QUrl cf;
    QString query;
    const auto& vqi = QUrlQuery{videoUri}.queryItems();
    if (vqi.count())
      {
      QStringList queryItems;
      foreach (const auto& qi, vqi)
        {
        queryItems.append(qi.first + '=' + qi.second);
        }
      QChar sep = '?';
      do
        {
        query += sep + queryItems.takeFirst();
        sep = '&';
        }
      while (queryItems.count());
      }
    cf.setPath('/' + vfi.fileName() + query + ".vsd");
    this->internal_->DescriptorCacheFile.setPath(
      this->internal_->cacheLocation_.path(QUrl::FullyEncoded) +
      cf.path(QUrl::FullyEncoded), QUrl::StrictMode);
    qtDebug(vqdQueryParserCache)
        << "descriptor cache file base name" << cf
        << "final URI" << this->internal_->DescriptorCacheFile;
    cf.setPath('/' + vfi.fileName() + query + ".vst");
    this->internal_->TrackCacheFile.setPath(
      this->internal_->cacheLocation_.path(QUrl::FullyEncoded) +
      cf.path(QUrl::FullyEncoded), QUrl::StrictMode);
    qtDebug(vqdQueryParserCache)
        << "track cache file base name" << cf
        << "final URI" << this->internal_->TrackCacheFile;

    while (!bypassCache)
      {
      qtDebug(vqdQueryParserCache) << "attempting cache load";

      qtKstReader kst(this->internal_->DescriptorCacheFile);
      vvKstReader reader;
      vvHeader header;

      // Open file with video descriptors
      if (!(kst.isValid() && reader.readHeader(kst, header)
            && header.type == vvHeader::Descriptors))
        {
        this->postStatus("Unable to read file "
                         + this->internal_->DescriptorCacheFile.toString()
                         + "; cache will not be used");
        break;
        }

      // Read descriptors
      QList<vvDescriptor> descriptors;
      if (!reader.readDescriptors(kst, descriptors,
                                  header.version))
        {
        this->postStatus("Error reading descriptors from file "
                         + this->internal_->DescriptorCacheFile.toString()
                         + "; cache will not be used");
        break;
        }

      // Success; next see if we have a track cache
      qtDebug(vqdQueryParserCache) << "descriptor cache load successful";

      qtKstReader trackKst(this->internal_->TrackCacheFile);
      bool trackCacheSucceeded = true;

      // Open file with tracks
      if (!(trackKst.isValid() && reader.readHeader(trackKst, header)
            && header.type == vvHeader::Tracks))
        {
        this->postStatus("Unable to read file "
                         + this->internal_->TrackCacheFile.toString()
                         + "; track cache will not be used");
        trackCacheSucceeded = false;
        }

      // Read tracks
      QList<vvTrack> tracks;
      if (trackCacheSucceeded && !reader.readTracks(trackKst, tracks,
                                                    header.version))
        {
        this->postStatus("Error reading tracks from file "
                         + this->internal_->TrackCacheFile.toString()
                         + "; track cache will not be used");
        trackCacheSucceeded = false;
        }

      if (trackCacheSucceeded)
        {
        this->postStatus("Cache retrieval successful", true);
        emit this->formulationComplete(descriptors, tracks);
        }
      else
        {
        this->postStatus("Cache retrieval partially successful"
                         " (unable to read tracks)", true);
        emit this->formulationComplete(descriptors);
        }
      return true;
      }
    }

  if (!this->beginSession())
    {
    return false;
    }

  return this->internal_->session_->formulateQuery(request);
}

//-----------------------------------------------------------------------------
bool vqQueryParser::processQuery(vvQueryInstance query)
{
  if (!this->beginSession())
    {
    return false;
    }

  query.abstractQuery()->StreamIdLimit.clear();
  const int wss = vqSettings().iqrWorkingSetSize();
  return this->internal_->session_->processQuery(query, wss);
}

//-----------------------------------------------------------------------------
bool vqQueryParser::requestScoring(int resultsToScore)
{
  if (!this->internal_->session_)
    {
    return false;
    }
  return this->internal_->session_->requestRefinement(resultsToScore);
}

//-----------------------------------------------------------------------------
bool vqQueryParser::refineQuery(vvIqr::ScoringClassifiers feedback)
{
  if (!this->internal_->session_)
    {
    return false;
    }
  return this->internal_->session_->refineQuery(feedback);
}

//-----------------------------------------------------------------------------
bool vqQueryParser::updateIqrModel(vvQueryInstance& query)
{
  // must be a SimilarityQuery to have IQR model to update
  if (!this->internal_->session_ || !query.isSimilarityQuery())
    {
    return false;
    }

  return this->internal_->session_->updateIqrModel(query);
}

//-----------------------------------------------------------------------------
void vqQueryParser::forwardSignal(const char* signal)
{
  connect(this->internal_->session_.data(), signal, this, signal);
}

//-----------------------------------------------------------------------------
void vqQueryParser::forwardStatusMessage(qtStatusSource, QString msg)
{
  emit this->statusMessageAvailable(this->statusSource(), msg);
}

//-----------------------------------------------------------------------------
void vqQueryParser::forwardProgress(qtStatusSource, bool available,
                                    qreal value)
{
  emit this->progressAvailable(this->statusSource(), available, value);
}

//-----------------------------------------------------------------------------
void vqQueryParser::forwardProgress(qtStatusSource, bool available,
                                    int value, int steps)
{
  emit this->progressAvailable(this->statusSource(), available, value, steps);
}

//-----------------------------------------------------------------------------
void vqQueryParser::forwardError(qtStatusSource, QString msg)
{
  emit this->error(this->statusSource(), msg);
}

//-----------------------------------------------------------------------------
bool vqQueryParser::beginSession()
{
  if (this->internal_->session_)
    {
    return true;
    }

  this->internal_->session_.reset(
    vvQueryService::createSession(this->internal_->server_));
  if (!this->internal_->session_)
    {
    return this->syncAbort("No handler available for query server URI scheme "
                           + this->internal_->server_.scheme());
    }

  this->statusSource().setName(this->internal_->session_.data());

  this->forwardSignal(SIGNAL(formulationComplete(QList<vvDescriptor>,
                                                 QList<vvTrack>)));
  this->forwardSignal(SIGNAL(resultAvailable(vvQueryResult, bool)));
  this->forwardSignal(SIGNAL(resultSetComplete(bool)));

  connect(this->internal_->session_.data(),
          SIGNAL(statusMessageAvailable(qtStatusSource, QString)),
          this, SLOT(forwardStatusMessage(qtStatusSource, QString)));
  connect(this->internal_->session_.data(),
          SIGNAL(progressAvailable(qtStatusSource, bool, qreal)),
          this, SLOT(forwardProgress(qtStatusSource, bool, qreal)));
  connect(this->internal_->session_.data(),
          SIGNAL(progressAvailable(qtStatusSource, bool, int, int)),
          this, SLOT(forwardProgress(qtStatusSource, bool, int, int)));
  connect(this->internal_->session_.data(),
          SIGNAL(error(qtStatusSource, QString)),
          this, SLOT(forwardError(qtStatusSource, QString)));

  connect(this->internal_->session_.data(),
          SIGNAL(formulationComplete(QList<vvDescriptor>,
                                     QList<vvTrack>)),
          this,
          SLOT(updateTrackAndDescriptorCache(QList<vvDescriptor>,
                                             QList<vvTrack>)));
  connect(this->internal_->session_.data(), SIGNAL(finished()),
          this, SLOT(queryFinished()));

  return true;
}

//-----------------------------------------------------------------------------
bool vqQueryParser::syncAbort(const QString& message)
{
  this->postError(message);
  return false;
}

//-----------------------------------------------------------------------------
bool vqQueryParser::asyncAbort(const QString& message)
{
  this->postError(message);
  emit this->finished();
  return true;
}

//-----------------------------------------------------------------------------
bool vqQueryParser::updateDescriptorCache(QList<vvDescriptor> descriptors)
{
  // Anything to do?
  CHECK_ARG(!this->internal_->DescriptorCacheFile.isEmpty(), true);

  // Ensure cache directory exists
  const QString cacheFileName =
    this->internal_->DescriptorCacheFile.toLocalFile();
  CHECK_ARG(checkCacheDir(QFileInfo(cacheFileName).dir()), false);

  // Try to open cache file for writing
  QFile file(cacheFileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
    qtDebug(vqdQueryParserCache)
        << "unable to open descriptor cache file"
        << this->internal_->DescriptorCacheFile
        << "-" << file.errorString();
    return false;
    }

  // Write descriptors to file
  vvKstWriter writer(file);
  writer << vvHeader::Descriptors << descriptors;

  qtDebug(vqdQueryParserCache)
      << "updated descriptor cache written to"
      << this->internal_->DescriptorCacheFile;
  return true;
}

//-----------------------------------------------------------------------------
bool vqQueryParser::updateTrackAndDescriptorCache(
  QList<vvDescriptor> descriptors, QList<vvTrack> tracks)
{
  // First write the descriptors; if that fails, don't bother with the tracks
  CHECK_ARG(this->updateDescriptorCache(descriptors), false);

  // Anything to do?
  CHECK_ARG(!this->internal_->TrackCacheFile.isEmpty(), true);

  // Ensure cache directory exists
  const QString cacheFileName =
    this->internal_->TrackCacheFile.toLocalFile();
  CHECK_ARG(checkCacheDir(QFileInfo(cacheFileName).dir()), false);

  // Try to open cache file for writing
  QFile file(cacheFileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
    qtDebug(vqdQueryParserCache)
        << "unable to open track cache file" << this->internal_->TrackCacheFile
        << "-" << file.errorString();
    return false;
    }

  // Write tracks to file
  vvKstWriter writer(file);
  writer << vvHeader::Tracks << tracks;

  qtDebug(vqdQueryParserCache)
      << "updated track cache written to" << this->internal_->TrackCacheFile;
  return true;
}

//-----------------------------------------------------------------------------
void vqQueryParser::queryFinished()
{
  if (this->internal_->session_)
    {
    this->internal_->session_->shutdown();
    this->internal_->session_.reset();
    this->statusSource().setName(this);
    }
  emit this->finished();
}
