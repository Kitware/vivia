// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgRenderer_h
#define __vtkVgRenderer_h

#include "vtkOpenGLRenderer.h"

#include <vgExport.h>

class VTKVG_SCENEGRAPH_EXPORT vtkVgRenderer : public vtkOpenGLRenderer
{
public:
  vtkTypeMacro(vtkVgRenderer, vtkOpenGLRenderer);

  static vtkVgRenderer* New();

protected:
  vtkVgRenderer() : vtkOpenGLRenderer() {}
  virtual ~vtkVgRenderer() {}

  virtual void PickRender(vtkPropCollection* props);

private:
  vtkVgRenderer(const vtkVgRenderer&);
  void operator= (const vtkVgRenderer&);
};

#endif // VTKVGRENDERER_H
