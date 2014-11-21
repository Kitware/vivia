/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <vcl_string.h>

#include <vul/vul_arg.h>

#include <tracking_data/io/kw18_reader.h>

#include <qtKstReader.h>
#include <qtStlUtil.h>

#include <vvAdaptVidtk.h>

#include <vvTrack.h>
#include <vvHeader.h>
#include <vvWriter.h>

#include <QFile>
#include <QMap>

//-----------------------------------------------------------------------------
int
main(int argc, char** argv)
{
  vul_arg<vcl_string> argOutputFileName(
    "-o", "(REQUIRED) Path to output file");
  vul_arg<vcl_string> argInputTracksFileName(
    "-t", "(REQUIRED) Path to input .kw18 file");
  vul_arg<vcl_string> argInputTocFileName(
    "-p", "Path to input .pvo.txt file");
  vul_arg<int> argTrackSource(
    "--source", "Track source ID (default = 1)");

  vul_arg_parse(argc, argv);

  if (argOutputFileName() == "")
    {
    vcl_cerr << "Output file name is required\n";
    argOutputFileName.display_usage_and_exit();
    }
  if (argInputTracksFileName() == "")
    {
    vcl_cerr << "Input file name is required\n";
    argOutputFileName.display_usage_and_exit();
    }

  QMap<vvTrackId, vvTrack> tracks;

  // Open track input file and read tracks
  vcl_cout << "Reading tracks from " << argInputTracksFileName() << "... ";
  vcl_cout.flush();
  vidtk::kw18_reader trackReader(argInputTracksFileName().c_str());
  vcl_vector<vidtk::track_sptr> vidtkTracks;
  if (!trackReader.read(vidtkTracks))
    {
    vcl_cerr << "FAILED\n";
    return EXIT_FAILURE;
    }
  vcl_cout << "OK (read " << vidtkTracks.size() << " tracks)\n";
  if (vidtkTracks.size() == 0)
    {
    vcl_cerr << "Exiting without writing to output file"
                " because no tracks were read!\n";
    return EXIT_SUCCESS;
    }

  // Convert tracks to VisGUI format (i.e. vvTrack)
  for (size_t i = 0; i < vidtkTracks.size(); ++i)
    {
    vidtk::track& vt = *vidtkTracks[i];

    vvTrack track;
    track.Id = vvTrackId(argTrackSource(), vt.id());

    const vcl_vector<vidtk::track_state_sptr>& h = vt.history();
    for (size_t j = 0; j < h.size(); ++j)
      {
      // vvTrack can't store multiple states per time, so just take the first
      // state (we only expect one under normal circumstances, anyway)
      track.Trajectory.insert(vvAdapt(*h[j]).first());
      }

    tracks.insert(track.Id, track);
    }

  // Open P/V/O input file (if given) and read TOC's
  if (argInputTocFileName.set())
    {
    vcl_cout << "Reading TOC's from " << argInputTocFileName() << "... ";
    vcl_cout.flush();
    QUrl uri = QUrl::fromLocalFile(qtString(argInputTocFileName()));
    qtKstReader tocReader(uri, QRegExp("\\s+"), QRegExp("\n"));
    if (!tocReader.isValid())
      {
      vcl_cerr << "FAILED\n";
      return EXIT_FAILURE;
      }
    vcl_cout << "OK (read " << tocReader.recordCount() << " TOC's)\n";

    while (!tocReader.isEndOfFile())
      {
      long long trackSerialNumber;
      double pPerson, pVehicle, pOther;
      if (tocReader.readLong(trackSerialNumber, 0) &&
          tocReader.readReal(pPerson, 1) &&
          tocReader.readReal(pVehicle, 2) &&
          tocReader.readReal(pOther, 3))
        {
        vvTrackId tid(argTrackSource(), trackSerialNumber);
        if (tracks.contains(tid))
          {
          vvTrack& track = tracks[tid];
          track.Classification.insert(
            std::make_pair(std::string("Person"), pPerson));
          track.Classification.insert(
            std::make_pair(std::string("Vehicle"), pVehicle));
          track.Classification.insert(
            std::make_pair(std::string("Other"), pOther));
          }
        else
          {
          vcl_cerr << "WARNING: no match for track " << trackSerialNumber
                   << " while mapping TOC's to tracks\n";
          }
        }
      tocReader.nextRecord();
      }
    }

  // Open output file
  vcl_cout << "Writing tracks to " << argOutputFileName() << "... ";
  vcl_cout.flush();
  QFile of(argOutputFileName().c_str());
  if (!of.open(QIODevice::WriteOnly | QIODevice::Text))
    {
    vcl_cerr << "FAILED\n";
    return EXIT_FAILURE;
    }
  vvWriter writer(of);

  // Write output file
  writer << vvHeader::Descriptors << tracks.values();
  vcl_cout << "OK\n";

  return EXIT_SUCCESS;
}
