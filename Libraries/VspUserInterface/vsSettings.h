// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsSettings_h
#define __vsSettings_h

#include <QColor>

#include <qtSettings.h>

class vsSettings : public qtSettings
{
public:
  vsSettings();

  qtSettings_declare(bool, colorTracksBySource, setColorTracksBySource);
  qtSettings_declare(QColor, selectionPenColor, setSelectionPenColor);
  qtSettings_declare(QColor, filteringMaskColor, setFilteringMaskColor);
  qtSettings_declare(QColor, dataMinColor, setDataMinColor);
  qtSettings_declare(QColor, dataMaxColor, setDataMaxColor);

#ifdef Q_CC_MSVC
  // Work around bug in Microsoft's typically broken compiler
  // (See http://connect.microsoft.com/VisualStudio/feedback/details/380090)
  #define WRAP_VARARGS(...) (__VA_ARGS__)
  #define FOREACH_GRAPH_REP_WIDTHS(func, ...) \
    func WRAP_VARARGS(TrackHeadWidth, __VA_ARGS__) \
    func WRAP_VARARGS(TrackTrailWidth, __VA_ARGS__) \
    func WRAP_VARARGS(EventHeadWidth, __VA_ARGS__) \
    func WRAP_VARARGS(EventTrailWidth, __VA_ARGS__)
#else
  #define FOREACH_GRAPH_REP_WIDTHS(func, ...) \
    func(TrackHeadWidth, ##__VA_ARGS__) \
    func(TrackTrailWidth, ##__VA_ARGS__) \
    func(EventHeadWidth, ##__VA_ARGS__) \
    func(EventTrailWidth, ##__VA_ARGS__)
#endif

#define DECLARE_GRAPH_REP_WIDTHS(suffix, lcPrefix, ucPrefix) \
  qtSettings_declare(float, lcPrefix ## suffix, set ## ucPrefix ## suffix);

  FOREACH_GRAPH_REP_WIDTHS(DECLARE_GRAPH_REP_WIDTHS, normal, Normal)
  FOREACH_GRAPH_REP_WIDTHS(DECLARE_GRAPH_REP_WIDTHS, groundTruth, GroundTruth)
};

#endif
