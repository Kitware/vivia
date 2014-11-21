/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgAreaPicker_h
#define __vtkVgAreaPicker_h

#include <vtkAreaPicker.h>

#include <vgExport.h>

class VTKVG_SCENEGRAPH_EXPORT vtkVgAreaPicker : public vtkAreaPicker
{
public:
  vtkTypeMacro(vtkVgAreaPicker, vtkAreaPicker);

  static vtkVgAreaPicker* New();

  virtual int PickProps(vtkRenderer* renderer);

protected:
  vtkVgAreaPicker() : vtkAreaPicker() {;}
  virtual ~vtkVgAreaPicker() {;}

private:
  vtkVgAreaPicker(const vtkVgAreaPicker&);
  void operator= (const vtkVgAreaPicker&);
};

#endif // __vtkVgAreaPicker_h
