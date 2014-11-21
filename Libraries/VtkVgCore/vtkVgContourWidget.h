/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
