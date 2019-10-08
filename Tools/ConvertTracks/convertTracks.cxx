/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <vdfDataSource.h>
#include <vdfSourceService.h>
#include <vdfTrackReader.h>

#include <vvTrack.h>
#include <vvHeader.h>
#include <vvWriter.h>

#include <qtCliArgs.h>
#include <qtKstReader.h>
#include <qtStlUtil.h>

#include <QCoreApplication>
#include <QDebug>
#include <QFile>

typedef QHash<vdfTrackId, vdfTrackReader::Track> TrackMap;

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  // Set application information
  QCoreApplication::setApplicationName("VisGUI track conversion utility");
  QCoreApplication::setOrganizationName("Kitware");
  QCoreApplication::setOrganizationDomain("kitware.com");

  // Set up command line options
  qtCliArgs args(argc, argv);

  qtCliOptions options;
  options.add("output <file>", "Path to output file", qtCliOption::Required)
         .add("o", qtCliOption::Short);
  options.add("tracks <file>", "Path to input track file",
              qtCliOption::Required)
         .add("t", qtCliOption::Short);
  options.add("class <file>",
              "Path to input track classification file")
         .add("c", qtCliOption::Short);
  options.add("source <num>",
              "Track source ID (if not provided by input track file)", "1")
         .add("s", qtCliOption::Short);
  args.addOptions(options);

  // Parse arguments
  args.parseOrDie();

  bool ok;
  const int defaultSource = args.value("source").toInt(&ok);
  if (!ok)
    {
    qCritical() << "ERROR: Failed to parse numeric value"
                << args.value("source");
    qCritical() << "ERROR: Source option requires an integer";
    return EXIT_FAILURE;
    }

  // Create application (needed for vdfTrackReader event loop)
  QCoreApplication app(args.qtArgc(), args.qtArgv());

  // Set up reader
  vdfTrackReader reader;

  // Read input tracks
  const QUrl trackUri = QUrl::fromLocalFile(args.value("tracks"));
  QScopedPointer<vdfDataSource> trackSource(
    vdfSourceService::createArchiveSource(trackUri, reader.desiredSources()));

  if (!trackSource)
    {
    qCritical() << "ERROR: Failed to create track source";
    return EXIT_FAILURE;
    }

  // Read tracks
  qDebug() << "Reading tracks from" << trackUri;
  reader.setSource(trackSource.data());
  if (!reader.exec() || !reader.hasData())
    {
    qCritical() << "ERROR: Failed to read tracks (or no tracks present)";
    return EXIT_FAILURE;
    }

  // Add tracks to internal map, setting default ID if needed
  const TrackMap& rawTracks = reader.tracks();
  TrackMap tracks;
  QMultiHash<long long, vdfTrackId> trackIds;
  qDebug() << "Read" << rawTracks.count() << "track(s)";
  foreach_iter (TrackMap::const_iterator, ti, rawTracks)
    {
    vdfTrackId id = ti.key();
    id.Provider = (id.Provider == -1 ? defaultSource : id.Provider);
    tracks.insert(id, ti.value());
    trackIds.insert(id.SerialNumber, id);
    }
  qDebug() << "Read" << tracks.count() << "track(s)";

  // Open P/V/O input file (if given) and read TOC's
  // TODO: Use a framework source to do this, once we have one...
  if (args.isSet("class"))
    {
    const QUrl tocUri = QUrl::fromLocalFile(args.value("class"));
    qDebug() << "Reading TOC's from" << tocUri;

    qtKstReader tocReader(tocUri, QRegExp("\\s+"), QRegExp("\n"));
    if (!tocReader.isValid())
      {
      qCritical() << "ERROR: Parse error reading track object classifications";
      return EXIT_FAILURE;
      }
    qDebug() << "Read" << tocReader.recordCount() << "TOC(s)";

    while (!tocReader.isEndOfFile())
      {
      long long trackSerialNumber;
      double pPerson, pVehicle, pOther;
      if (tocReader.readLong(trackSerialNumber, 0) &&
          tocReader.readReal(pPerson, 1) &&
          tocReader.readReal(pVehicle, 2) &&
          tocReader.readReal(pOther, 3))
        {
        QList<vdfTrackId> matchingIds = trackIds.values(trackSerialNumber);
        switch (matchingIds.count())
          {
          case 1:
            break;
          case 0:
            qWarning() << "WARNING: No match for track serial number"
                       << trackSerialNumber << "while mapping TOC's to tracks";
            break;
          default:
            qWarning() << "WARNING: Track serial number" << trackSerialNumber
                       << "matches" << matchingIds.count() << "tracks";
            qWarning() << "WARNING: TOC will be applied to all of them";
            break;
          }
        foreach (const vdfTrackId& tid, matchingIds)
          {
          vdfTrackReader::Track& track = tracks[tid];
          track.Classification.insert(
            std::make_pair(std::string("Person"), pPerson));
          track.Classification.insert(
            std::make_pair(std::string("Vehicle"), pVehicle));
          track.Classification.insert(
            std::make_pair(std::string("Other"), pOther));
          }
        }
      tocReader.nextRecord();
      }
    }

  // Open output file
  qDebug() << "Writing tracks to" << args.value("output");
  QFile of(args.value("output"));
  if (!of.open(QIODevice::WriteOnly | QIODevice::Text))
    {
    qCritical() << "ERROR: Failed to open output file:" << of.errorString();
    return EXIT_FAILURE;
    }

  // Write output file
  vvWriter writer(of);
  writer << vvHeader::Tracks;

  int oc = 0;
  foreach_iter (TrackMap::const_iterator, ti, tracks)
    {
    // Convert track from vgDataFramework format to vvIO format
    vvTrack ot;
    ot.Id = vvTrackId(ti.key().Provider, ti.key().SerialNumber);
    ot.Classification = ti.value().Classification;
    foreach (const vvTrackState& s, ti.value().Trajectory)
      {
      ot.Trajectory.insert(s);
      }
    writer << ot;
    ++oc;
    }

  qDebug() << "Wrote" << oc << "track(s)";
  return EXIT_SUCCESS;
}
