// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfTemporalSelector_h
#define __vdfTemporalSelector_h

#include "vdfSelector.h"

#include <vgNamespace.h>

class vgTimeStamp;

class vdfTemporalSelectorPrivate;

/// Selector which filters based on time.
///
/// This selector requests the data which most closely matches a specific time,
/// according to the seek direction. Depending on the type of data, this may
/// match a single element (e.g. video data), or many elements (e.g. in a
/// collection of track data, the best track state match for each track).
class VG_DATA_FRAMEWORK_EXPORT vdfTemporalSelector : public vdfSelector
{
  Q_OBJECT

public:
  /// Additional options to control data selection.
  enum TemporalSelectionFlag
    {
    /// Request data node to provide interpolated data where possible in order
    /// to achieve a better match.
    TS_Interpolate = 0x1
    };
  Q_DECLARE_FLAGS(TemporalSelectionFlags, TemporalSelectionFlag)

  /// Create selector.
  ///
  /// This creates a selector with the specified time stamp, seek mode, and
  /// selection flags.
  vdfTemporalSelector(const vgTimeStamp&, vg::SeekMode = vg::SeekNearest,
                      TemporalSelectionFlags = 0);
  /// Create selector.
  /// \overload
  vdfTemporalSelector(const vgTimeStamp&, TemporalSelectionFlags);

  virtual ~vdfTemporalSelector();

  /// \copybrief vdfSelector::clone
  virtual vdfSelector* clone() const;

  /// Get time stamp to use for selecting data.
  vgTimeStamp timeStamp() const;

  /// Get seek mode to use when selecting data.
  vg::SeekMode seekMode() const;

  /// Get selection option flag.
  TemporalSelectionFlags options() const;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vdfTemporalSelector)

private:
  QTE_DECLARE_PRIVATE(vdfTemporalSelector)
  QTE_DISABLE_COPY(vdfTemporalSelector)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(vdfTemporalSelector::TemporalSelectionFlags)

#endif
