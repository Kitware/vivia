/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpQtViewer3dWidget_h
#define __vpQtViewer3dWidget_h

// Qt includes
#include <QWidget>

#include "vpQtViewer3d.h"

// Forward declarations.
class QVTKWidget;

class vtkVgTimeStamp;

class vpQtViewer3dWidget : public QWidget
{
  Q_OBJECT

public:
  vpQtViewer3dWidget(QWidget* parent, Qt::WindowFlags flag = 0);

  virtual ~vpQtViewer3dWidget();

  vpQtViewer3d* getViewer3d() const;

  virtual void update(const vtkVgTimeStamp& timestamp);

  virtual void reset();


private:

  void setupViewer(QVTKWidget* qvtkWidget);

  class vtkInternal;
  vtkInternal* Internal;

  vpQtViewer3d* Viewer3d;
};

#endif // __vpQtViewer3dWidget_h
