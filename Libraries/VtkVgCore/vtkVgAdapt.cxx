/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgAdapt.h"

//-----------------------------------------------------------------------------
void vtkVgAdapt(const vnl_matrix_fixed<double, 3, 3>& in, vtkMatrix4x4* out)
{
  double k = (in[2][2] < 0.0 ? -1.0 : 1.0);

  out->Identity();

  out->SetElement(0, 0, in[0][0] * k);
  out->SetElement(0, 1, in[0][1] * k);
  out->SetElement(0, 3, in[0][2] * k);

  out->SetElement(1, 0, in[1][0] * k);
  out->SetElement(1, 1, in[1][1] * k);
  out->SetElement(1, 3, in[1][2] * k);

  out->SetElement(3, 0, in[2][0] * k);
  out->SetElement(3, 1, in[2][1] * k);
  out->SetElement(3, 3, in[2][2] * k);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMatrix4x4>
vtkVgAdapt(const vnl_matrix_fixed<double, 3, 3>& vnlMatrix)
{
  vtkSmartPointer<vtkMatrix4x4> vtkMatrix =
    vtkSmartPointer<vtkMatrix4x4>::New();

  vtkVgAdapt(vnlMatrix, vtkMatrix);

  return vtkMatrix;
}
