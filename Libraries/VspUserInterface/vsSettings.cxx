// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsSettings.h"

#define QTSETTINGS_SUBCLASS_NAME vsSettings
#include <qtSettingsImpl.h>

#define DECLARE_KEY(keyName) \
  static const auto key ## keyName = QStringLiteral(#keyName);

DECLARE_KEY(ColorTracksBySource);
DECLARE_KEY(SelectionPenColor);
DECLARE_KEY(FilteringMaskColor);
DECLARE_KEY(DataMinColor);
DECLARE_KEY(DataMaxColor);

#define DECLARE_GRAPH_REP_WIDTHS_KEYS(suffix, prefix, keyPrefix) \
  static const auto key ## prefix ## suffix = \
    QStringLiteral( keyPrefix #suffix );

FOREACH_GRAPH_REP_WIDTHS(
  DECLARE_GRAPH_REP_WIDTHS_KEYS, Normal, "Display/")
FOREACH_GRAPH_REP_WIDTHS(
  DECLARE_GRAPH_REP_WIDTHS_KEYS, GroundTruth, "Display/GroundTruth/")

//-----------------------------------------------------------------------------
vsSettings::vsSettings()
{
  this->declareSetting(keyColorTracksBySource, QVariant{false});
  this->declareSetting(keySelectionPenColor, QColor{255, 20, 144});
  this->declareSetting(keyFilteringMaskColor, QColor{0, 0, 0, 96});
  this->declareSetting(keyDataMinColor, QColor{Qt::red});
  this->declareSetting(keyDataMaxColor, QColor{Qt::cyan});

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
