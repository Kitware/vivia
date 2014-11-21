/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// Kitware license?

#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkActor.h"
#include "vtkTexture.h"
#include "vtkPolyDataMapper.h"
#include "vtkPlaneSource.h"
#include "vtkSmartPointer.h"
#include "vtkImageFlip.h"

#include "vtkVgImageSource.h"
#include "vtkVgInteractorStyleRubberBand2D.h"

// VXL code from vxl/core/vil/tests/test_file_format_read.cxx


int main(int argc,
         char* argv[])
{
  if (argc  < 2)
    {
    cerr << "Requires image filename as argument." << endl;
    return 0;
    }

  // East to use vars.
  typedef vtkSmartPointer<vtkRenderWindowInteractor>
    vtkRenderWindowInteractorRef;
  typedef vtkSmartPointer<vtkRenderWindow>    vtkRenderWindowRef;
  typedef vtkSmartPointer<vtkPolyDataMapper>  vtkPolyDataMapperRef;
  typedef vtkSmartPointer<vtkPlaneSource>     vtkPlaneSourceRef;
  typedef vtkSmartPointer<vtkVgInteractorStyleRubberBand2D>
    vtkVgInteractorStyleRubberBand2DRef;

  vtkRenderWindowInteractorRef iren(vtkRenderWindowInteractorRef::New());
  vtkRenderWindowRef           win(vtkRenderWindowRef::New());
  vtkSmartPointer<vtkRenderer> r(vtkSmartPointer<vtkRenderer>::New());
  vtkSmartPointer<vtkActor>    a(vtkSmartPointer<vtkActor>::New());
  vtkPolyDataMapperRef         m(vtkPolyDataMapperRef::New());
  vtkPlaneSourceRef            p(vtkPlaneSourceRef::New());
  vtkSmartPointer<vtkTexture>  t(vtkSmartPointer<vtkTexture>::New());
  vtkVgInteractorStyleRubberBand2DRef
    is(vtkVgInteractorStyleRubberBand2DRef::New());

  is->SetEnabled(1);
  iren->SetInteractorStyle(is);
  win->SetInteractor(iren);
  win->AddRenderer(r);

  m->SetInputConnection(p->GetOutputPort());
  a->SetMapper(m);
  a->SetTexture(t);
  r->AddViewProp(a);

  // VXL to VTK pipeline
  vtkStdString fileName(argv[1]);

  vtkSmartPointer<vtkVgImageSource> vgImgSrc(vtkSmartPointer<vtkVgImageSource>::New());
  vgImgSrc->SetFileName(fileName);
  vgImgSrc->SetReadExtents(320, 639, 240, 479);
  vgImgSrc->SetOutputResolution(320 * 4, 240 * 4);
  vgImgSrc->SetUseOutputResolution(true);
  vgImgSrc->Update();
  t->SetInputConnection(vgImgSrc->GetOutputPort());
  t->SetInterpolate(true);

  iren->Initialize();
  iren->Start();

  return 0;
}
