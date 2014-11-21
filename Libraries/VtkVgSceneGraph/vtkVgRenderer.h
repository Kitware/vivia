/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
