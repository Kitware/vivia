/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsLineWidget.h"

#include <vtkCommand.h>
#include <vtkLineWidget2.h>
#include <vtkMatrix4x4.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include <vtkVgAdapt.h>
#include <vtkVgInstance.h>
#include <vtkVgLineRepresentation.h>
#include <vtkVgRendererUtils.h>

#include <vtkVgQtUtil.h>

QTE_IMPLEMENT_D_FUNC(vsLineWidget)

//-----------------------------------------------------------------------------
class vsLineWidgetPrivate
{
public:
  void init(vtkRenderWindowInteractor* rwi);

  vtkVgInstance<vtkLineWidget2> Widget;
  vtkVgInstance<vtkVgLineRepresentation> Representation;
};

//-----------------------------------------------------------------------------
void vsLineWidgetPrivate::init(vtkRenderWindowInteractor* rwi)
{
  this->Widget->SetRepresentation(this->Representation);
  this->Widget->SetInteractor(rwi);
}

//-----------------------------------------------------------------------------
vsLineWidget::vsLineWidget(vtkRenderWindowInteractor* rwi)
  : d_ptr(new vsLineWidgetPrivate())
{
  QTE_D(vsLineWidget);
  d->init(rwi);
}

//-----------------------------------------------------------------------------
vsLineWidget::~vsLineWidget()
{
}

//-----------------------------------------------------------------------------
void vsLineWidget::setMatrix(vtkMatrix4x4* matrix)
{
  QTE_D(vsLineWidget);
  d->Representation->SetTransformMatrix(matrix);
  this->updateLineLength();
}

//-----------------------------------------------------------------------------
void vsLineWidget::setMatrix(const vgMatrix4d& matrix)
{
  vtkVgInstance<vtkMatrix4x4> vtkMatrix;
  vtkVgAdapt(matrix, vtkMatrix);
  this->setMatrix(vtkMatrix);
}

//-----------------------------------------------------------------------------
double vsLineWidget::lineLength()
{
  QTE_D(vsLineWidget);

  return d->Representation->GetDistance();
}

//-----------------------------------------------------------------------------
void vsLineWidget::setVisible(bool visible)
{
  QTE_D(vsLineWidget);
  d->Representation->SetVisibility(visible);
}

//-----------------------------------------------------------------------------
void vsLineWidget::begin()
{
  QTE_D(vsLineWidget);

  d->Widget->On();

  double bounds[6];
  vtkVgRendererUtils::GetBounds(d->Representation->GetRenderer(), bounds);
  bounds[4] = 0.5;
  bounds[5] = 0.5;

  double pos1[3] = { 0.0, 0.0, 0.0 };
  double pos2[3] = { 1.0, 0.0, 0.0 };
  d->Representation->SetPoint1WorldPosition(pos1);
  d->Representation->SetPoint2WorldPosition(pos2);

  double padding = 0.4 * (bounds[1] - bounds[0]);
  bounds[0] += padding;
  bounds[1] -= padding;

  // Initially place the widget in the center of the view
  d->Representation->PlaceWidget(bounds);

  d->Widget->Render();

  vtkConnect(d->Widget, vtkCommand::InteractionEvent,
             this, SLOT(updateLineLength()));
}

//-----------------------------------------------------------------------------
void vsLineWidget::end()
{
  QTE_D(vsLineWidget);

  d->Widget->ProcessEventsOff();
}

//-----------------------------------------------------------------------------
void vsLineWidget::updateLineLength()
{
  QTE_D(vsLineWidget);

  emit this->lineLengthChanged(d->Representation->GetDistance());
}
