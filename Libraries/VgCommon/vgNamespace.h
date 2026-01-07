// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgNamespace_h
#define __vgNamespace_h

namespace vg
{
  /// Requested seek mode.
  ///
  /// The value relates to the caller's expected result based on the input. The
  /// meaning of lower/upper bound is similar to, but not exactly the same as,
  /// the like-named Qt/STL container search methods.
  enum SeekMode
    {
    /// Direction is not specified; the result is implementation defined.
    ///
    /// Most implementations will interpret this as "choose a sensible default"
    /// (most often ::SeekNearest).
    SeekUnspecified = -1,
    /// Request the closest possible value to the input.
    SeekNearest = 0,
    /// Request nearest value that is greater than or equal to the request.
    ///
    /// The request is treated as a lower bound for permissible result values.
    /// The result value will be the same as ::SeekExact, if such a value
    /// exists. Otherwise, the result will be the same as ::SeekNext.
    SeekLowerBound,
    /// Request nearest value that is less than or equal to the request.
    ///
    /// The request is treated as an upper bound for permissible result values.
    /// The result value will be the same as ::SeekExact, if such a value
    /// exists. Otherwise, the result will be the same as ::SeekPrevious.
    SeekUpperBound,
    /// Request an exact match only.
    ///
    /// The result value will be "exactly" equal to the request. If no such
    /// value exists, no value will be returned. An implementation is permitted
    /// to interpret "exact" as "within a reasonable amount of fuzz to
    /// accommodate for floating point rounding error".
    SeekExact,
    /// Request nearest value that is strictly greater than the request.
    ///
    /// \sa ::SeekLowerBound
    SeekNext,
    /// Request nearest value that is strictly less than the request.
    ///
    /// \sa ::SeekUpperBound
    SeekPrevious
    };
} // namespace vg

#endif
