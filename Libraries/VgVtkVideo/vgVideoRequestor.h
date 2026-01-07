// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
