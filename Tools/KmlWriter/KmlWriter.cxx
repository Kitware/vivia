// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

// KML includes.
#include "kml/dom.h"
#include "kml/dom/kml22.h"
#include "kml/base/color32.h"
#include "kml/engine/kmz_file.h"
#include "kml/engine/kml_file.h"

using std::stringstream;
using kmldom::ColorStylePtr;
using kmldom::CoordinatesPtr;
using kmldom::DocumentPtr;
using kmldom::FolderPtr;
using kmldom::GroundOverlayPtr;
using kmldom::IconPtr;
using kmldom::InnerBoundaryIsPtr;
using kmldom::KmlPtr;
using kmldom::KmlFactory;
using kmldom::LatLonBoxPtr;
using kmldom::LinearRingPtr;
using kmldom::LineStringPtr;
using kmldom::LineStylePtr;
using kmldom::LodPtr;
using kmldom::MultiGeometryPtr;
using kmldom::OuterBoundaryIsPtr;
using kmldom::OverlayXYPtr;
using kmldom::PlacemarkPtr;
using kmldom::PointPtr;
using kmldom::PolygonPtr;
using kmldom::PolyStylePtr;
using kmldom::RegionPtr;
using kmldom::ScreenOverlayPtr;
using kmldom::ScreenXYPtr;
using kmldom::SizePtr;
using kmldom::StylePtr;
using kmldom::NetworkLinkPtr;
using kmldom::LinkPtr;
using kmldom::TimeSpanPtr;

// Meta data file needs be a CSV with four columns as described below:
// "title", "movie file", "lat", "lon"
// The title is the tag that will be shown on maps or google earth,
// movie file is the one thats going to be played inside the baloon
// at location given by lat, lon.

//-----------------------------------------------------------------------------
void PrintHelp()
{
  std::cout << "Usage: KmlWriter [metadata file] [output file]\n";
}

//-----------------------------------------------------------------------------
void Split(const std::string& line, char c, std::vector<std::string>& rowData)
{
  int i = 0;
  int j = line.find(c);

  while (j >= 0)
    {
    if (j == 0)
      {
      rowData.push_back("");
      }
    else
      {
      rowData.push_back(line.substr(i, j - i));
      }

    i = ++j;

    j = line.find(c, i);

    if (j < 0)
      {
      rowData.push_back(line.substr(i, line.length()));
      }
    }
}

//-----------------------------------------------------------------------------
int LoadCSV(std::vector<std::vector<std::string>*>& data,
            const char* metaDataFileName)
{
  std::ifstream in(metaDataFileName);

  if (!in)
    {
    return EXIT_FAILURE;
    }

  std::vector<std::string>* p = NULL;
  std::string tmp;

  while (!in.eof())
    {
    std::getline(in, tmp, '\n');

    if (tmp.empty())
      {
      continue;
      }

    p = new std::vector<std::string>();

    Split(tmp, ',', *p);

    data.push_back(p);

    tmp.clear();
    }

  return EXIT_SUCCESS;
}

// Description:
//-----------------------------------------------------------------------------
std::string CreateVideoDescription(const std::string& videoFileName)
{
  std::string returnString =
    std::string(" <div style=\"width:350px\", id=\"clock\"></div>\n\
			<EMBED id=\"vlc\" type=\"application/x-vlc-plugin\" ") +
    std::string("SRC=\"") + videoFileName +
    std::string("\" width=320 height=310 name=\"video2\" autoplay=\"yes\" loop=\"no\">\n\
			</EMBED>\n\
					<br />\n\
						<script type=\"text/javascript\">\n\
							function mute()\n\
							{\n\
								vlc.audio.toggleMute();\n\
							}\n\
							function play()\n\
							{\n\
								vlc.playlist.play();\n\
							}\n\
							function stop()\n\
							{\n\
								vlc.playlist.stop();\n\
							}\n\
							function pause()\n\
							{\n\
								vlc.playlist.togglePause();\n\
							}\n\
						</script>\n\
					<br />\n\
			</div>\n\
			<div id=\"controls\">\n\
				<input type=\"button\" onclick=\"play()\" value=\"Play\" />\n\
				<input type=\"button\" onclick=\"pause()\" value=\"Pause\" />\n\
				<input type=\"button\" onclick=\"stop()\" value=\"Stop\" />\n\
				<input type=\"button\" onclick=\"mute()\" value=\"Mute\" />\n\
			</div>\n");

  return returnString;
}

// Description:
// Takes meta data file and output file name to generate KML.
//-----------------------------------------------------------------------------
int WriteKML(const char* metaDataFileName, const char* outKMLFileName)
{
  // First create the factory.
  KmlFactory* factory = KmlFactory::GetFactory();

  // Create KML.
  KmlPtr kml = factory->CreateKml();

  // Create document.
  DocumentPtr doc = factory->CreateDocument();

  // Now set document as feature.
  kml->set_feature(doc);

  //
  std::vector<std::vector<std::string>*> Data;

  // Load CSV.
  LoadCSV(Data, metaDataFileName);

  for (size_t i = 0; i < Data.size(); ++i)
    {

    std::vector<std::string>* rowData = Data[i];

    // Crate a new place mark.
    PointPtr          point        = factory->CreatePoint();
    PlacemarkPtr      placemark    = factory->CreatePlacemark();
    CoordinatesPtr    coordinates  = factory->CreateCoordinates();

    double xy[2];
    std::istringstream iss1(rowData->at(2));
    std::istringstream iss2(rowData->at(3));

    iss1 >> xy[0];
    iss2 >> xy[1];

    coordinates->add_latlngalt(xy[0], xy[1], 0.0);
    point->set_coordinates(coordinates);

    placemark->set_geometry(point);
    placemark->set_name(rowData->at(0));
    placemark->set_description(CreateVideoDescription(rowData->at(1)));

    doc->add_feature(placemark);
    }

  std::ofstream out(outKMLFileName);
  if (out.good())
    {
    doc->set_name(outKMLFileName);
    out << kmldom::SerializePretty(kml);
    out.flush();
    out.close();

    return EXIT_SUCCESS;
    }
  else
    {
    out.close();
    std::cerr << "Error writing data to output stream." << std::endl;
    return EXIT_FAILURE;
    }
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  if (argc < 3)
    {
    PrintHelp();
    return EXIT_FAILURE;
    }

  return WriteKML(argv[1], argv[2]);
}
