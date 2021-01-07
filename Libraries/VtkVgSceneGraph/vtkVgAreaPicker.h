// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
