// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfNamespace_h
#define __vdfNamespace_h

#include <QtGlobal>

#include <vgExport.h>

/// \namespace vdf Global definitions for the VisGUI Data Framework
namespace vdf
{
  /// Option flags controlling how node updates are performed.
  /// \sa \ref vdfUpdateModesAndResponses
  enum UpdateFlag
    {
    /// Allow multiple updates per request
    UpdateIncremental = 0x1,
    /// Obtain data as fast as possible at possible cost of quality
    UpdateFastest = 0x2,
    };
  Q_DECLARE_FLAGS(UpdateFlags, UpdateFlag)

  /// Register Qt meta types used by the VisGUI Data Framework
  ///
  /// This function performs Qt meta type registration for those data types
  /// used by the VisGUI Data Framework that may need to be passed in queued
  /// signal connections. This function uses qtOnce internally, and so is
  /// fairly inexpensive to call once registration has been performed.
  extern VG_DATA_FRAMEWORK_EXPORT void registerMetaTypes();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(vdf::UpdateFlags)

#endif
