// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgVideoSource_h
#define __vgVideoSource_h

#include "vgVideoSourceRequestor.h"

class vgVideoSource
{
public:
  virtual ~vgVideoSource() {}

  /// Find the time stamp for a given frame number.
  ///
  /// This method attempts to find time stamp that best matches a particular
  /// frame number, using the requested search mode.
  ///
  /// \return The time stamp that best meets the request, or an invalid time
  ///         stamp if there is no suitable match.
  ///
  /// \note This call is synchronous, but as there is no guarantee that the
  ///       source lives in the thread as potential users, this method may
  ///       block while the request is handled by another thread.
  virtual vtkVgTimeStamp findTime(unsigned int frameNumber, vg::SeekMode) = 0;

  /// Request a video frame.
  ///
  /// This method issues a request for a video frame, which may execute
  /// asynchronously. The source will answer the request by invoking a well
  /// known method on the requestor object, passing along the request ID given
  /// in the original request. If the request cannot be satisfied, the callback
  /// method will be invoked with a null pointer for the frame data.
  ///
  /// \note The source may also choose to ignore requests if the result of the
  ///       request would be the same as the most recently answered request, or
  ///       if additional requests have been received before the source is able
  ///       to respond to earlier requests.
  ///
  /// \sa clearLastRequest
  virtual void requestFrame(vgVideoSeekRequest) = 0;

  /// Discard notion of last request answered.
  ///
  /// This method instructs a source to discard its notion of the last frame
  /// that it provided to the specified requestor. This is useful if the last
  /// received frame becomes unavailable (e.g. when using a secondary requestor
  /// to fill a look-ahead buffer), to allow the requestor to re-request the
  /// same frame.
  virtual void clearLastRequest(vgVideoSourceRequestor*) = 0;
};

#endif
