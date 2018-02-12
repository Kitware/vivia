/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgQtAdapt.h"

#include <QMatrix4x4>

#include <vtkMatrix4x4.h>

//-----------------------------------------------------------------------------
QMatrix4x4 qtAdapt(const vtkMatrix4x4* in)
{
  QMatrix4x4 out;
  if (sizeof(qreal) == sizeof(double))
    {
    memcpy(out.data(), in->Element, 16 * sizeof(double));
    return out.transposed();
    }
  else
    {
    for (int i = 0; i < 4; ++i)
      {
      for (int j = 0; j < 4; ++j)
        {
        out(i, j) = in->GetElement(i, j);
        }
      }
    return out;
    }
}

//-----------------------------------------------------------------------------
void qtAdapt(const QMatrix4x4& in, vtkMatrix4x4* out)
{
  if (sizeof(qreal) == sizeof(double))
    {
    memcpy(out->Element, in.transposed().constData(), 16 * sizeof(double));
    }
  else
    {
    for (int i = 0; i < 4; ++i)
      {
      for (int j = 0; j < 4; ++j)
        {
        out->SetElement(i, j, in(i, j));
        }
      }
    }
}
