/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsDisplayInfo_h
#define __vsDisplayInfo_h

#include <vgColor.h>

struct vsDisplayInfo
{
  /// Display state of the entity.
  ///
  /// This provides the display state of the entity, i.e. \c false if the
  /// entity has been hidden by the user, otherwise \c true. The display state
  /// is not affected by filters.
  bool DisplayState;

  /// Visibility of the entity.
  ///
  /// This indicates if the entity is visible. It is \c true iff the entity is
  /// not filtered or hidden.
  bool Visible;

  /// Best classification (track or event type) of this entity.
  ///
  /// This provides the identifier (a track classification type or registered
  /// event type, depending on the type of entity) that best matches the entity
  /// given current filter settings. If the entity is filtered out, the value
  /// is \c -1.
  int BestClassification;

  /// Confidence (probability) of classification.
  ///
  /// This provides the computed estimate that the classification given by
  /// BestClassification is accurate. The normal range is from \c 0.0 to
  /// \c 1.0. If the confidence is unavailable, the value will be negative
  /// (e.g. if it cannot be computed, or there is no classification).
  double Confidence;

  /// Current color of the entity.
  ///
  /// This provides the current (pen) color of the entity, taking into account
  /// any filters and/or special display modes which may be in effect, but not
  /// selection.
  vgColor Color;
};

#endif
