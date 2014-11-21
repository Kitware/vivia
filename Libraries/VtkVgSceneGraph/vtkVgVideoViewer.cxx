/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgVideoViewer.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkVgVideoViewer);

//-----------------------------------------------------------------------------
vtkVgVideoViewer::vtkVgVideoViewer() : vtkVgViewer()
{
}

//-----------------------------------------------------------------------------
vtkVgVideoViewer::~vtkVgVideoViewer()
{
}

//-----------------------------------------------------------------------------
void vtkVgVideoViewer::Realize()
{
  // Do nothing other than setting the state to true.
  this->Realized = true;
}

//-----------------------------------------------------------------------------
void vtkVgVideoViewer::Run()
{
  if (!this->IsRealized())
    {
    this->Realize();
    }
}
