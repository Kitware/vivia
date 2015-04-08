/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgVideoRequestor_h
#define __vgVideoRequestor_h

#include "vgVtkVideoFrame.h"

#include <vgNamespace.h>

#include <qtGlobal.h>

class vgVideoSource;

class vgVideoRequestorPrivate;

class VG_VTKVIDEO_EXPORT vgVideoRequestor
{
public:
  explicit vgVideoRequestor(vgVideoSource* source);
  ~vgVideoRequestor();

  bool requestFrame(vtkVgVideoFrame& out,
                    vtkVgTimeStamp time, vg::SeekMode direction);

  static bool requestFrame(vtkVgVideoFrame& out, vgVideoSource* source,
                           vtkVgTimeStamp time, vg::SeekMode direction);

protected:
  QTE_DECLARE_PRIVATE_PTR(vgVideoRequestor)

private:
  Q_DISABLE_COPY(vgVideoRequestor)
};


#endif
