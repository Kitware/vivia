// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsContourWidget.h"

#include <vtkCellArray.h>
#include <vtkCommand.h>
#include <vtkContourWidget.h>
#include <vtkIdList.h>
#include <vtkLinearContourLineInterpolator.h>
#include <vtkMatrix4x4.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkRenderWindowInteractor.h>

#include <vtkVgContourRepresentation.h>
#include <vtkVgInstance.h>

#include <vtkVgQtUtil.h>

QTE_IMPLEMENT_D_FUNC(vsContourWidget)

//-----------------------------------------------------------------------------
class vsContourWidgetPrivate
{
public:
  vsContourWidgetPrivate(int id) : Id(id) {}

  void init(vtkRenderWindowInteractor* rwi);

  int State;

  const int Id;
  QString Name;
  vsContour::Type Type;

  vtkVgInstance<vtkContourWidget> Widget;
  vtkVgInstance<vtkVgContourRepresentation> Representation;
};

//-----------------------------------------------------------------------------
void vsContourWidgetPrivate::init(vtkRenderWindowInteractor* rwi)
{
  this->Type = vsContour::NoType;

  this->Widget->SetRepresentation(this->Representation);
  this->Widget->SetInteractor(rwi);
  this->Widget->SetContinuousDraw(true);

  vtkLinearContourLineInterpolator* interpolator =
    vtkLinearContourLineInterpolator::New();

  this->Representation->SetLineInterpolator(interpolator);
  interpolator->FastDelete();

  this->Representation->AlwaysOnTopOn();
  this->Representation->SetLineColor(1.0, 0.0, 0.0);
}

//-----------------------------------------------------------------------------
vsContourWidget::vsContourWidget(int id, vtkRenderWindowInteractor* rwi)
  : d_ptr(new vsContourWidgetPrivate(id))
{
  QTE_D(vsContourWidget);
  d->init(rwi);

  d->Name = "Unnamed Region " + QString::number(id);
  d->State = vsContourWidget::Begin;
}

//-----------------------------------------------------------------------------
vsContourWidget::vsContourWidget(const vsContour& contour,
                                 vtkRenderWindowInteractor* rwi)
  : d_ptr(new vsContourWidgetPrivate(contour.id()))
{
  QTE_D(vsContourWidget);
  d->init(rwi);

  d->Name = contour.name();
  d->State = vsContourWidget::Complete;

  d->Widget->On();
  d->Widget->ProcessEventsOff();

  this->setType(contour.type());
  this->setPoints(contour.points());

  d->Representation->Finalize();
}

//-----------------------------------------------------------------------------
vsContourWidget::~vsContourWidget()
{
}

//-----------------------------------------------------------------------------
int vsContourWidget::state() const
{
  QTE_D_CONST(vsContourWidget);
  return d->State;
}

//-----------------------------------------------------------------------------
bool vsContourWidget::isClosed()
{
  QTE_D(vsContourWidget);
  return !!d->Representation->GetClosedLoop();
}

//-----------------------------------------------------------------------------
int vsContourWidget::id() const
{
  QTE_D_CONST(vsContourWidget);
  return d->Id;
}

//-----------------------------------------------------------------------------
QString vsContourWidget::name() const
{
  QTE_D_CONST(vsContourWidget);
  return d->Name;
}

//-----------------------------------------------------------------------------
vsContour::Type vsContourWidget::type() const
{
  QTE_D_CONST(vsContourWidget);
  return d->Type;
}

//-----------------------------------------------------------------------------
vtkContourWidget* vsContourWidget::widget()
{
  QTE_D(vsContourWidget);
  return d->Widget;
}

//-----------------------------------------------------------------------------
vtkPoints* vsContourWidget::points()
{
  QTE_D(vsContourWidget);
  return d->Representation->GetPoints();
}

//-----------------------------------------------------------------------------
vsContour vsContourWidget::toContour()
{
  QTE_D_CONST(vsContourWidget);

  // Convert points to QPolygonF
  QPolygonF polygon;
  vtkPoints* vp = this->points();
  for (vtkIdType n = 0; n < vp->GetNumberOfPoints(); ++n)
    {
    double p[3];
    vp->GetPoint(n, p);
    polygon.append(QPointF(p[0], p[1]));
    }

  // QPolygonF is closed if first point == last point; vsContour does not
  // duplicate to indicate closure, so need to repeat the first point
  if (this->isClosed())
    {
    polygon.append(polygon.front());
    }

  // Create vsContour
  return vsContour(d->Id, d->Type, polygon, d->Name);
}

//-----------------------------------------------------------------------------
void vsContourWidget::setMatrix(vtkMatrix4x4* matrix)
{
  QTE_D(vsContourWidget);
  d->Representation->SetTransformMatrix(matrix);
}

//-----------------------------------------------------------------------------
void vsContourWidget::setPoints(const QPolygonF& points)
{
  QTE_D(vsContourWidget);

  vtkIdList* ids = vtkIdList::New();
  vtkPoints* pts = vtkPoints::New();
  vtkPolyData* pd = vtkPolyData::New();
  vtkCellArray* ca = vtkCellArray::New();

  int numIds = points.count();
  int numPoints = points.isClosed() ? numIds - 1 : numIds;

  ids->SetNumberOfIds(numIds);
  pts->SetNumberOfPoints(numPoints);

  for (int i = 0; i < numPoints; ++i)
    {
    const QPointF& point = points[i];
    pts->SetPoint(i, point.x(), point.y(), 0.0);
    ids->SetId(i, i);
    }

  if (points.isClosed())
    {
    ids->SetId(numIds - 1, 0);
    }

  ca->InsertNextCell(ids);
  pd->SetPoints(pts);
  pd->SetLines(ca);

  d->Widget->Initialize(pd);

  ids->FastDelete();
  pts->FastDelete();
  pd->FastDelete();
  ca->FastDelete();
}

//-----------------------------------------------------------------------------
void vsContourWidget::setName(QString name)
{
  QTE_D(vsContourWidget);
  d->Name = name;
}

//-----------------------------------------------------------------------------
void vsContourWidget::setType(vsContour::Type type)
{
  QTE_D(vsContourWidget);

  d->Type = type;
  switch (type)
    {
    case vsContour::Annotation:
      d->Representation->SetLineColor(1.0, 0.0, 0.0);
      break;
    case vsContour::Tripwire:
      d->Representation->SetLineColor(1.0, 1.0, 0.0);
      break;
    case vsContour::Selector:
      d->Representation->SetLineColor(1.0, 1.0, 1.0);
      break;
    case vsContour::Filter:
      d->Representation->SetLineColor(1.0, 0.0, 1.0);
      break;
    default:
      d->Representation->SetLineColor(0.0, 0.0, 0.0);
      break;
    }
}

//-----------------------------------------------------------------------------
void vsContourWidget::setVisible(bool visible)
{
  QTE_D(vsContourWidget);
  d->Representation->SetVisibility(visible);
}

//-----------------------------------------------------------------------------
void vsContourWidget::begin()
{
  QTE_D(vsContourWidget);

  d->Widget->On();
  d->Widget->Initialize();
  d->Widget->FollowCursorOn();
  d->Widget->Render();

  // widget fires an EndInteraction event when it changes to manipulate mode
  // (by closing the loop or the user right clicking)
  vtkConnect(d->Widget, vtkCommand::EndInteractionEvent,
             this, SLOT(onInteractionComplete()));
}

//-----------------------------------------------------------------------------
void vsContourWidget::close()
{
  QTE_D(vsContourWidget);

  // don't automatically add the node following the mouse
  if (!d->Representation->GetClosedLoop() &&
      d->State == vsContourWidget::Begin)
    {
    d->Representation->DeleteLastNode();
    }
  d->Widget->CloseLoop();
}

//-----------------------------------------------------------------------------
void vsContourWidget::finalize()
{
  QTE_D(vsContourWidget);

  d->Widget->ProcessEventsOff();
  d->Representation->Finalize();
}

//-----------------------------------------------------------------------------
void vsContourWidget::onRightClick()
{
  QTE_D(vsContourWidget);

  if (d->State == vsContourWidget::Manipulate)
    {
    d->State = vsContourWidget::Complete;
    vtkDisconnect(d->Widget->GetInteractor()->GetInteractorStyle(),
                  vtkCommand::RightButtonPressEvent);
    emit this->manipulationComplete();
    }
}

//-----------------------------------------------------------------------------
void vsContourWidget::onInteractionComplete()
{
  QTE_D(vsContourWidget);

  if (d->State == vsContourWidget::Begin)
    {
    d->State = vsContourWidget::Manipulate;

    // don't automatically add the node following the mouse
    if (!d->Representation->GetClosedLoop())
      {
      d->Representation->DeleteLastNode();
      }

    // close loop if required by type
    if (vsContour::isLoopType(d->Type))
      {
      this->close();
      }

    // listen for the final right click to indicate completion
    vtkConnect(d->Widget->GetInteractor()->GetInteractorStyle(),
               vtkCommand::RightButtonPressEvent,
               this, SLOT(onRightClick()));

    emit this->interactionComplete();
    }
}
