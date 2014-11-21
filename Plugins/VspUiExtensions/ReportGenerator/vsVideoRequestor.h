/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsVideoRequestor_h
#define __vsVideoRequestor_h

#include <qtGlobal.h>

#include <vgNamespace.h>

#include <vgVideoSourceRequestor.h>

class vgVideoSource;

class vsVideoRequestorPrivate;

class vsVideoRequestor
{
public:
  explicit vsVideoRequestor(vgVideoSource* source);
  ~vsVideoRequestor();

  bool requestFrame(vtkVgVideoFrame& out,
                    vtkVgTimeStamp time, vg::SeekMode direction);

  static bool requestFrame(vtkVgVideoFrame& out, vgVideoSource* source,
                           vtkVgTimeStamp time, vg::SeekMode direction);

protected:
  QTE_DECLARE_PRIVATE_PTR(vsVideoRequestor)

private:
  Q_DISABLE_COPY(vsVideoRequestor)
};


#endif
