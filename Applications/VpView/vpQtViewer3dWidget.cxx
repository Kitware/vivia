// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpQtViewer3dWidget.h"

#include "ui_vpQtViewer3dWidget.h"

// VisGUI includes
#include "vpQtViewer3d.h"
#include "vtkVgTimeStamp.h"

// VTK includes
#include <vtkInteractorStyle.h>
#include <QVTKWidget.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// Qt includes
#include <QDebug>

class vpQtViewer3dWidget::vtkInternal
{
public:

  vtkInternal()
    {
    this->UI = new Ui::Viewer3dWidget();
    }

  ~vtkInternal()
    {
    delete this->UI;
    }

  Ui::Viewer3dWidget* UI;
};

//-----------------------------------------------------------------------------
vpQtViewer3dWidget::vpQtViewer3dWidget(QWidget* parent, Qt::WindowFlags flag) :
  QWidget(parent, flag)
{
  this->Internal = new vtkInternal();
  this->Viewer3d = new vpQtViewer3d(this);

  this->Internal->UI->setupUi(this);

  this->setupViewer(this->Internal->UI->qvtkWidget);
}

//-----------------------------------------------------------------------------
vpQtViewer3dWidget::~vpQtViewer3dWidget()
{
  delete this->Internal;
  this->Internal = 0;
}

//-----------------------------------------------------------------------------
vpQtViewer3d* vpQtViewer3dWidget::getViewer3d() const
{
  return this->Viewer3d;
}

//-----------------------------------------------------------------------------
void vpQtViewer3dWidget::update(const vtkVgTimeStamp& timestamp)
{
  this->Viewer3d->update(timestamp);
}

//-----------------------------------------------------------------------------
void vpQtViewer3dWidget::reset()
{
  this->Viewer3d->reset();
}

//-----------------------------------------------------------------------------
void vpQtViewer3dWidget::setupViewer(QVTKWidget* qvtkWidget)
{
  if (qvtkWidget)
    {
    this->Viewer3d->setSceneRenderWindow(qvtkWidget->GetRenderWindow());
    }
  else
    {
    qDebug() << "ERROR: Invalid (null) widget";
    }
}
