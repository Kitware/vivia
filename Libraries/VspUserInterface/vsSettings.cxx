/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsSettings.h"

#define QTSETTINGS_SUBCLASS_NAME vsSettings
#include <qtSettingsImpl.h>

static const char* const keyColorTracksBySource    = "ColorTracksBySource";
static const char* const keySelectionPenColor      = "SelectionPenColor";
static const char* const keyFilteringMaskColor     = "FilteringMaskColor";
static const char* const keyDataMinColor           = "DataMinColor";
static const char* const keyDataMaxColor           = "DataMaxColor";

#define DECLARE_GRAPH_REP_WIDTHS_KEYS(suffix, prefix, keyPrefix) \
  static const char* const key ## prefix ## suffix = \
  keyPrefix #suffix;

FOREACH_GRAPH_REP_WIDTHS(
  DECLARE_GRAPH_REP_WIDTHS_KEYS, Normal, "Display/")
FOREACH_GRAPH_REP_WIDTHS(
  DECLARE_GRAPH_REP_WIDTHS_KEYS, GroundTruth, "Display/GroundTruth/")

//-----------------------------------------------------------------------------
vsSettings::vsSettings()
{
  this->declareSetting(keyColorTracksBySource, QVariant(false));
  this->declareSetting(keySelectionPenColor, QColor(255, 20, 144));
  this->declareSetting(keyFilteringMaskColor, QColor(0, 0, 0, 96));
  this->declareSetting(keyDataMinColor, Qt::red);
  this->declareSetting(keyDataMaxColor, Qt::cyan);

  this->declareSetting(keyNormalTrackHeadWidth, 1.0f);
  this->declareSetting(keyNormalTrackTrailWidth, 1.7f);
  this->declareSetting(keyNormalEventHeadWidth, 1.0f);
  this->declareSetting(keyNormalEventTrailWidth, 4.0f);

  this->declareSetting(keyGroundTruthTrackHeadWidth, 1.0f * 1.5f);
  this->declareSetting(keyGroundTruthTrackTrailWidth, 1.7f * 1.5f);
  this->declareSetting(keyGroundTruthEventHeadWidth, 1.0f * 1.5f);
  this->declareSetting(keyGroundTruthEventTrailWidth, 4.0f * 1.5f);
}

//-----------------------------------------------------------------------------
qtSettings_implement(bool, colorTracksBySource, ColorTracksBySource)
qtSettings_implement(QColor, selectionPenColor, SelectionPenColor)
qtSettings_implement(QColor, filteringMaskColor, FilteringMaskColor)
qtSettings_implement(QColor, dataMinColor, DataMinColor)
qtSettings_implement(QColor, dataMaxColor, DataMaxColor)

#define IMPLEMENT_GRAPH_REP_WIDTHS(suffix, lcPrefix, ucPrefix) \
  qtSettings_implement(float, lcPrefix ## suffix, ucPrefix ## suffix);

FOREACH_GRAPH_REP_WIDTHS(IMPLEMENT_GRAPH_REP_WIDTHS, normal, Normal)
FOREACH_GRAPH_REP_WIDTHS(IMPLEMENT_GRAPH_REP_WIDTHS, groundTruth, GroundTruth)
