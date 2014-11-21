/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgVideoSourceRequestor_h
#define __vgVideoSourceRequestor_h

#include <QObject>
#include <QSharedPointer>

#include <vgExport.h>

#include <vgNamespace.h>

#include <vtkVgTimeStamp.h>

#include "vgVtkVideoFrame.h"

class vgVideoSourceRequestor;

//-----------------------------------------------------------------------------
typedef QSharedPointer<vgVideoSourceRequestor> vgVideoSourceRequestorPtr;

struct VG_VTKVIDEO_EXPORT vgVideoSeekRequest
{
  vgVideoSeekRequest();
  ~vgVideoSeekRequest();

  /// Copy another request, except for the requestor.
  ///
  /// This method is more efficient than a normal copy when the requestor is
  /// not needed in the new request (e.g. when issuing the reply), since it
  /// avoids the overhead of atomic reference counting operations.
  void copyWithoutRequestor(const vgVideoSeekRequest& other);

  /// Copy another request, destructively.
  ///
  /// This method copies another request in a manner that is less expensive
  /// than a normal copy, but causes the other request to become invalid. This
  /// should be used when the other request can or will be discarded after the
  /// copy is made.
  ///
  /// This method works by calling ::copyWithoutRequestor, then swapping the
  /// requestor pointer with \p other, thereby reducing the number of atomic
  /// reference counting operations that must be performed.
  void destructiveCopy(vgVideoSeekRequest& other);

  /// Send reply to the request.
  ///
  /// This method invokes vgVideoSourceRequestor::update on the requestor,
  /// giving back the \p frame and a copy of this request. If the requestor's
  /// event loop is running in a thread other than the current thread, the
  /// update will be delivered asynchronously.
  void sendReply(vgVtkVideoFramePtr frame) const;

  vgVideoSourceRequestorPtr Requestor;
  qint64 RequestId;
  vtkVgTimeStamp TimeStamp;
  vg::SeekMode Direction;
};

//-----------------------------------------------------------------------------
class VG_VTKVIDEO_EXPORT vgVideoSourceRequestor : public QObject
{
  Q_OBJECT

public:
  virtual ~vgVideoSourceRequestor() {}

protected slots:
  virtual void update(vgVideoSeekRequest, vgVtkVideoFramePtr) = 0;

protected:
  vgVideoSourceRequestor() {}

private:
  Q_DISABLE_COPY(vgVideoSourceRequestor)
};

#endif
