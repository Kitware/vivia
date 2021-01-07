// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgContourWidget_h
#define __vtkVgContourWidget_h

#include <vtkContourWidget.h>

#include <vgExport.h>

class VTKVG_CORE_EXPORT vtkVgContourWidget : public vtkContourWidget
{
public:
  vtkTypeMacro(vtkVgContourWidget, vtkContourWidget);

  static vtkVgContourWidget* New();

protected:
  vtkVgContourWidget();

  static void DeleteAction(vtkAbstractWidget*);
};

#endif
