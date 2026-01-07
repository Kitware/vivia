// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpProjectBase_h
#define __vpProjectBase_h

#include <vgColor.h>

#include <vgGeoTypes.h>

#include <QPointF>
#include <QSizeF>
#include <QString>

struct vpProjectBase
{
  QString OverviewFile;
  QString DataSetSpecifier;
  QString TracksFile;
  QString TrackTraitsFile;
  QString TrackClassifiersFile;
  QString EventsFile;
  QString EventLinksFile;
  QString IconsFile;
  QString ActivitiesFile;
  QString InformaticsIconFile;
  QString NormalcyMapsFile;
  QString ImageTimeMapFile;
  QString HomographyIndexFile;
  QString FiltersFile;
  QString SceneElementsFile;
  QString CameraDirectory;
  QString DepthConfigFile;
  QString BundleAdjustmentConfigFile;

  int PrecomputeActivity = 0;

  double OverviewSpacing = 1.0;
  QPointF OverviewOrigin = {0.0, 0.0};

  vgGeocodedTile AOI;
  QSizeF AnalysisDimensions;

  double ColorWindow = 255.0;
  double ColorLevel = 127.5;

  double ColorMultiplier = 1.0;

  vgColor TrackColorOverride;

  int FrameNumberOffset = 0;
  int HomographyReferenceFrame = 0;
};

#endif
