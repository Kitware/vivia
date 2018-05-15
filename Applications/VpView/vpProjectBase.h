/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpProjectBase_h
#define __vpProjectBase_h

#include <vgColor.h>

#include <vgGeoTypes.h>

#include <QDir>
#include <QPointF>
#include <QSizeF>
#include <QString>

struct vpProjectBase
{
  QDir ConfigFileStem;

  QString OverviewFile;
  QString DataSetSpecifier;
  QString TracksFile;
  QString TrackTraitsFile;
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
};

#endif
