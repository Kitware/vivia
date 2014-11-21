/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsSettings.h"

#define QTSETTINGS_SUBCLASS_NAME vsSettings
#include <qtSettingsImpl.h>

static const char* const keyColorTracksBySource    = "ColorTracksBySource";
static const char* const keyEventSelectedColor     = "EventSelectedColor";
static const char* const keyFilteringMaskColor     = "FilteringMaskColor";
static const char* const keyDataMinColor           = "DataMinColor";
static const char* const keyDataMaxColor           = "DataMaxColor";

//-----------------------------------------------------------------------------
vsSettings::vsSettings()
{
  this->declareSetting(keyColorTracksBySource, QVariant(false));
  this->declareSetting(keyEventSelectedColor, QColor(255, 20, 144));
  this->declareSetting(keyFilteringMaskColor, QColor(0, 0, 0, 96));
  this->declareSetting(keyDataMinColor, Qt::red);
  this->declareSetting(keyDataMaxColor, Qt::cyan);
}

//-----------------------------------------------------------------------------
qtSettings_implement(colorTracksBySource, ColorTracksBySource, bool)
qtSettings_implement(eventSelectedColor, EventSelectedColor, QColor)
qtSettings_implement(filteringMaskColor, FilteringMaskColor, QColor)
qtSettings_implement(dataMinColor, DataMinColor, QColor)
qtSettings_implement(dataMaxColor, DataMaxColor, QColor)
