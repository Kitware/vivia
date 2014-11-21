/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgSpaceConversion.h"

// VTK includes
#include <vtkRenderer.h>

namespace
{

typedef void (*ConvertFunc)(vtkRenderer*, double*, double*);

//-----------------------------------------------------------------------------
void ConvertNormalized(vtkRenderer* renderer, double in[3], double out[3], ConvertFunc func)
{
  if (!renderer || !in || !out)
    {
    return;
    }
  double tmp[4];
  (*func)(renderer, in, tmp);
  if (fabs(tmp[3]) > 1e-10)
    {
    out[0] = tmp[0] / tmp[3];
    out[1] = tmp[1] / tmp[3];
    out[2] = tmp[2] / tmp[3];
    }
}

}

//-----------------------------------------------------------------------------
void vtkVgSpaceConversion::WorldToView(vtkRenderer* renderer, double in[4], double out[3])
{
  if (!renderer || !in || !out)
    {
    return;
    }

  renderer->SetWorldPoint(in);
  renderer->WorldToView();
  renderer->GetViewPoint(out);
}

//-----------------------------------------------------------------------------
void vtkVgSpaceConversion::WorldToDisplay(vtkRenderer* renderer, double in[4], double out[3])
{
  if (!renderer || !in || !out)
    {
    return;
    }

  renderer->SetWorldPoint(in);
  renderer->WorldToDisplay();
  renderer->GetDisplayPoint(out);
}

//-----------------------------------------------------------------------------
void vtkVgSpaceConversion::ViewToDisplay(vtkRenderer* renderer, double in[3], double out[3])
{
  if (!renderer || !in || !out)
    {
    return;
    }

  renderer->SetViewPoint(in);
  renderer->ViewToDisplay();
  renderer->GetDisplayPoint(out);
}

//-----------------------------------------------------------------------------
void vtkVgSpaceConversion::ViewToWorld(vtkRenderer* renderer, double in[3], double out[4])
{
  if (!renderer || !in || !out)
    {
    return;
    }

  renderer->SetViewPoint(in);
  renderer->ViewToWorld();
  renderer->GetWorldPoint(out);
}

//-----------------------------------------------------------------------------
void vtkVgSpaceConversion::DisplayToWorld(vtkRenderer* renderer, double in[3], double out[4])
{
  if (!renderer || !in || !out)
    {
    return;
    }

  renderer->SetDisplayPoint(in);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(out);
}

//-----------------------------------------------------------------------------
void vtkVgSpaceConversion::DisplayToView(vtkRenderer* renderer, double in[3], double out[3])
{
  if (!renderer || !in || !out)
    {
    return;
    }

  renderer->SetDisplayPoint(in);
  renderer->DisplayToView();
  renderer->GetViewPoint(out);
}

//-----------------------------------------------------------------------------
#define NORMALIZED_FLAVOR(_name_) \
  void vtkVgSpaceConversion::_name_##Normalized( \
    vtkRenderer *renderer, double in[3], double out[3]) \
  { ConvertNormalized(renderer, in, out, &vtkVgSpaceConversion::_name_); }
NORMALIZED_FLAVOR(ViewToWorld)
NORMALIZED_FLAVOR(DisplayToWorld)
