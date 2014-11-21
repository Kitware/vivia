/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsSettings_h
#define __vsSettings_h

#include <QColor>

#include <qtSettings.h>

class vsSettings : public qtSettings
{
public:
  vsSettings();

  qtSettings_declare(bool, colorTracksBySource, setColorTracksBySource);
  qtSettings_declare(QColor, eventSelectedColor, setEventSelectedColor);
  qtSettings_declare(QColor, filteringMaskColor, setFilteringMaskColor);
  qtSettings_declare(QColor, dataMinColor, setDataMinColor);
  qtSettings_declare(QColor, dataMaxColor, setDataMaxColor);
};

#endif
