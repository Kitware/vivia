// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgRegionWidget.h"

#include <QDebug>
#include <QMouseEvent>
#include <QTimer>

#include <qtStatusSource.h>

#include <vtkBorderWidget.h>
#include <vtkCommand.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

#include <vtkVgBorderRepresentation.h>
#include <vtkVgInstance.h>
#include <vtkVgSpaceConversion.h>

#include <vgColor.h>

#include <vtkVgQtUtil.h>

QTE_IMPLEMENT_D_FUNC(vtkVgRegionWidget)

///////////////////////////////////////////////////////////////////////////////

//BEGIN vtkVgRegionWidgetPrivate

class vtkVgRegionWidgetPrivate
{
public:
  vtkVgRegionWidgetPrivate(vtkVgRegionWidget* q);

  enum InteractionState
    {
    Off,
    Drop,
    DragDrop,
    IgnoreDrop,
    PickFirst,
    DragDraw,
    PickSecond,
    Manipulate
    };

  void setState(InteractionState);
  InteractionState state() { return this->State; }

  void updateDropPosition();
  void updateSecondCorner();

  qtStatusSource StatusSource;

  vtkRenderWindowInteractor* Interactor;

  vtkVgInstance<vtkBorderWidget> BorderWidget;
  vtkVgInstance<vtkVgBorderRepresentation> BorderRepresentation;

  QPoint FirstPoint, SecondPoint;
  QSize DropSize;

protected:
  QTE_DECLARE_PUBLIC_PTR(vtkVgRegionWidget)

  void postStatusMessage(const QString&);

  InteractionState State;
  bool QuickMode;
  bool DropDrawCentered;
  bool IsWithinView;

private:
  QTE_DECLARE_PUBLIC(vtkVgRegionWidget)
};

//-----------------------------------------------------------------------------
vtkVgRegionWidgetPrivate::vtkVgRegionWidgetPrivate(vtkVgRegionWidget* q) :
  StatusSource(q),
  q_ptr(q),
  State(Off),
  DropDrawCentered(false),
  IsWithinView(true)
{
}

//-----------------------------------------------------------------------------
void vtkVgRegionWidgetPrivate::setState(InteractionState s)
{
  if (this->State == s)
    return;

  QTE_Q(vtkVgRegionWidget);

  switch (this->State = s)
    {
    case Drop:
      this->BorderWidget->Render();
      this->postStatusMessage("Click to drop box; right-click to cancel");
      break;
    case PickFirst:
      this->BorderRepresentation->SetShowBorderToOn();
      this->BorderWidget->Render();
      this->postStatusMessage("Select first corner; right-click to cancel");
      break;
    case PickSecond:
      this->BorderRepresentation->SetShowBorderToOn();
      this->BorderWidget->Render();
      this->postStatusMessage("Select second corner; right-click to cancel");
      break;
    case Manipulate:
      this->BorderWidget->ProcessEventsOn();
      this->BorderRepresentation->SetShowBorderToOn();
      this->postStatusMessage("Editing region; right-click when done");
      vtkConnect(
        this->Interactor, vtkCommand::RightButtonPressEvent, q,
        SLOT(viewMouseEvent(vtkObject*, unsigned long)));
      emit q->beginningManipulation();
      break;
    case Off:
      this->BorderWidget->Off();
      break;
    case DragDraw:
    case DragDrop:
    case IgnoreDrop:
      break;
    }
  this->BorderWidget->Render();
}

//-----------------------------------------------------------------------------
void vtkVgRegionWidgetPrivate::updateDropPosition()
{
  int pos1[2];
  this->Interactor->GetEventPosition(pos1);
  this->DropDrawCentered = this->Interactor->GetControlKey();

  if (this->DropDrawCentered)
    {
    pos1[0] -= this->DropSize.width() / 2;
    pos1[1] += this->DropSize.height() / 2;
    }

  int pos2[2] =
    {
    pos1[0] + this->DropSize.width(),
    pos1[1] - this->DropSize.height()
    };

  this->BorderRepresentation->SetDisplayPoint1(pos1);
  this->BorderRepresentation->SetDisplayPoint2(pos2);
  this->BorderWidget->Render();
}

//-----------------------------------------------------------------------------
void vtkVgRegionWidgetPrivate::updateSecondCorner()
{
  int pos1[2] = { this->FirstPoint.x(), this->FirstPoint.y() };
  int pos2[2];
  this->Interactor->GetEventPosition(pos2);

  if (this->Interactor->GetControlKey())
    {
    pos1[0] -= (pos2[0] - pos1[0]);
    pos1[1] -= (pos2[1] - pos1[1]);
    }
  this->BorderRepresentation->SetDisplayPoint1(pos1);
  this->BorderRepresentation->SetDisplayPoint2(pos2);
  this->BorderWidget->Render();
}

//-----------------------------------------------------------------------------
void vtkVgRegionWidgetPrivate::postStatusMessage(const QString& message)
{
  QTE_Q(vtkVgRegionWidget);
  emit q->statusMessageAvailable(message);
  emit q->statusMessageAvailable(this->StatusSource, message);
}

//END vtkVgRegionWidgetPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vtkVgRegionWidget

//-----------------------------------------------------------------------------
vtkVgRegionWidget::vtkVgRegionWidget(
  vtkRenderWindowInteractor* interactor, QObject* parent)
  : QObject(parent), d_ptr(new vtkVgRegionWidgetPrivate(this))
{
  QTE_D(vtkVgRegionWidget);

  d->StatusSource.setName(this);

  d->Interactor = interactor;
  d->BorderWidget->SetRepresentation(d->BorderRepresentation);
  d->BorderWidget->SetInteractor(interactor);
  d->BorderWidget->SetSelectable(false);
  d->BorderWidget->Off();
  d->BorderWidget->SetKeyPressActivation(false);

  d->BorderRepresentation->GetBorderProperty()->SetColor(0.0, 1.0, 1.0);
  d->BorderRepresentation->GetBorderProperty()->SetLineWidth(3);
}

//-----------------------------------------------------------------------------
vtkVgRegionWidget::~vtkVgRegionWidget()
{
}

//-----------------------------------------------------------------------------
QRect vtkVgRegionWidget::rect()
{
  QTE_D(vtkVgRegionWidget);

  QRect r;
  r.setLeft(qRound(d->BorderRepresentation->GetPosition()[0]));
  r.setTop(qRound(d->BorderRepresentation->GetPosition()[1]));
  r.setRight(qRound(d->BorderRepresentation->GetPosition2()[0]));
  r.setBottom(qRound(d->BorderRepresentation->GetPosition2()[1]));

  return r.normalized();
}

//-----------------------------------------------------------------------------
QSize vtkVgRegionWidget::displaySize()
{
  QTE_D(vtkVgRegionWidget);

  vtkRenderer* renderer = d->BorderRepresentation->GetRenderer();

  double point1[3], point2[3];
  double worldPt[4] = { d->BorderRepresentation->GetPosition()[0],
                        d->BorderRepresentation->GetPosition()[1], 0.0, 1.0};
  vtkVgSpaceConversion::WorldToDisplay(renderer, worldPt, point1);

  worldPt[0] = d->BorderRepresentation->GetPosition2()[0];
  worldPt[1] = d->BorderRepresentation->GetPosition2()[1];
  vtkVgSpaceConversion::WorldToDisplay(renderer, worldPt, point2);

  return QSize(abs(qRound(point2[0] - point1[0])),
               abs(qRound(point2[1] - point1[1])));
}

//-----------------------------------------------------------------------------
void vtkVgRegionWidget::setRect(const QRect& r)
{
  QTE_D(vtkVgRegionWidget);

  d->BorderRepresentation->SetPosition(r.left(), r.top());
  d->BorderRepresentation->SetPosition2(r.right(), r.bottom());

  d->BorderWidget->On();
  d->setState(vtkVgRegionWidgetPrivate::Manipulate);
}

//-----------------------------------------------------------------------------
void vtkVgRegionWidget::setColor(QColor c)
{
  QTE_D(vtkVgRegionWidget);

  vgColor vgc(c);
  d->BorderRepresentation->GetBorderProperty()->SetColor(vgc.data().array);
}

//-----------------------------------------------------------------------------
void vtkVgRegionWidget::setLineWidth(qreal w)
{
  QTE_D(vtkVgRegionWidget);

  d->BorderRepresentation->GetBorderProperty()->SetLineWidth(w);
}

//-----------------------------------------------------------------------------
void vtkVgRegionWidget::setStyle(QColor color, qreal lineWidth)
{
  this->setColor(color);
  this->setLineWidth(lineWidth);
}

//-----------------------------------------------------------------------------
void vtkVgRegionWidget::begin()
{
  QTE_D(vtkVgRegionWidget);

  const char* const kslot = SLOT(viewKeyEvent(vtkObject*, unsigned long));
  const char* const mslot = SLOT(viewMouseEvent(vtkObject*, unsigned long));

  // Connect this to the interactor style to prevent it from handling drags
  // (e.g. rubber band zoom)
  vtkConnect(d->Interactor->GetInteractorStyle(),
             vtkCommand::LeftButtonPressEvent,
             this, mslot);

  vtkConnect(d->Interactor, vtkCommand::LeftButtonReleaseEvent, this, mslot);
  vtkConnect(d->Interactor, vtkCommand::RightButtonPressEvent, this, mslot);
  vtkConnect(d->Interactor, vtkCommand::MouseMoveEvent, this, mslot);
  vtkConnect(d->Interactor, vtkCommand::KeyPressEvent, this, kslot);
  vtkConnect(d->Interactor, vtkCommand::KeyReleaseEvent, this, kslot);

  d->BorderWidget->On();
  d->BorderWidget->ProcessEventsOff();
  d->BorderRepresentation->SetShowBorderToOff();
  d->QuickMode = false;
  d->setState(vtkVgRegionWidgetPrivate::PickFirst);
}

//-----------------------------------------------------------------------------
void vtkVgRegionWidget::beginQuick(const QSize& size)
{
  QTE_D(vtkVgRegionWidget);

  const char* const kslot = SLOT(viewKeyEvent(vtkObject*, unsigned long));
  const char* const mslot = SLOT(viewMouseEvent(vtkObject*, unsigned long));

  vtkConnect(d->Interactor, vtkCommand::LeftButtonPressEvent, this, mslot);
  vtkConnect(d->Interactor, vtkCommand::LeftButtonReleaseEvent, this, mslot);
  vtkConnect(d->Interactor, vtkCommand::RightButtonPressEvent, this, mslot);
  vtkConnect(d->Interactor, vtkCommand::MouseMoveEvent, this, mslot);
  vtkConnect(d->Interactor, vtkCommand::EnterEvent, this, mslot);
  vtkConnect(d->Interactor, vtkCommand::LeaveEvent, this, mslot);
  vtkConnect(d->Interactor, vtkCommand::KeyPressEvent, this, kslot);
  vtkConnect(d->Interactor, vtkCommand::KeyReleaseEvent, this, kslot);

  d->DropSize = size;
  d->BorderWidget->On();
  d->BorderWidget->ProcessEventsOff();
  d->BorderRepresentation->SetShowBorderToOff();
  d->QuickMode = true;
  d->setState(vtkVgRegionWidgetPrivate::Drop);
  d->BorderWidget->Render();
}

//-----------------------------------------------------------------------------
void vtkVgRegionWidget::viewKeyEvent(
  vtkObject* sender, unsigned long event)
{
  Q_UNUSED(sender);
  Q_UNUSED(event);

  QTE_D(vtkVgRegionWidget);

  if (d->Interactor->GetKeyCode() == 0) // modifier key pressed/released
    {
    const char* const slot = SLOT(viewMouseEvent(vtkObject*, unsigned long));

    vtkVgRegionWidgetPrivate::InteractionState const state = d->state();
    if (d->Interactor->GetShiftKey())
      {
      if (state == vtkVgRegionWidgetPrivate::Drop)
        {
        // Connect this to the interactor style to prevent it from handling
        // drags (e.g. rubber band zoom)
        vtkConnect(d->Interactor->GetInteractorStyle(),
                   vtkCommand::LeftButtonPressEvent,
                   this, slot);
        d->BorderRepresentation->SetVisibility(false);
        d->setState(vtkVgRegionWidgetPrivate::PickFirst);
        emit this->dropDrawStarted();
        }
      }
    else if (d->QuickMode)
      {
      switch (state)
        {
        case vtkVgRegionWidgetPrivate::PickFirst:
          // Disconnect this from the interactor style to allow it to handle
          // drags (e.g. rubber band zoom), since we do not need to block that
          // in drop mode
          vtkDisconnect(d->Interactor->GetInteractorStyle(),
                        vtkCommand::LeftButtonPressEvent,
                        this);
          d->setState(vtkVgRegionWidgetPrivate::Drop);
          d->BorderRepresentation->SetVisibility(d->IsWithinView);
          d->updateDropPosition();
          emit this->dropDrawCanceled();
        case vtkVgRegionWidgetPrivate::Drop:
        case vtkVgRegionWidgetPrivate::DragDrop:
          d->updateDropPosition();
          break;
        default:
          break;
        }
      }

    if (state == vtkVgRegionWidgetPrivate::PickSecond)
      {
      d->updateSecondCorner();
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgRegionWidget::viewMouseEvent(
  vtkObject* sender, unsigned long event)
{
  Q_UNUSED(sender);

  QTE_D(vtkVgRegionWidget);

  vtkVgRegionWidgetPrivate::InteractionState state = d->state();
  int pos[2];
  d->Interactor->GetEventPosition(pos);

  switch (event)
    {
    case vtkCommand::LeftButtonPressEvent:
      if (state == vtkVgRegionWidgetPrivate::PickFirst)
        {
        vtkConnect(d->Interactor, vtkCommand::LeftButtonPressEvent, this,
                   SLOT(viewMouseEvent(vtkObject*, unsigned long)));

        d->FirstPoint = QPoint(pos[0], pos[1]);
        d->BorderRepresentation->SetDisplayPoint1(pos);
        d->BorderRepresentation->SetDisplayPoint2(pos);
        d->BorderRepresentation->SetVisibility(true);
        d->setState(vtkVgRegionWidgetPrivate::DragDraw);
        QTimer::singleShot(700, this, SLOT(beginSecondPick()));
        }
      else if (state == vtkVgRegionWidgetPrivate::Drop)
        {
        d->FirstPoint = QPoint(pos[0], pos[1]);
        d->setState(vtkVgRegionWidgetPrivate::DragDrop);
        }
      break;
    case vtkCommand::MouseMoveEvent:
      if (state == vtkVgRegionWidgetPrivate::DragDrop)
        {
        d->SecondPoint = QPoint(pos[0], pos[1]);
        int dist = (d->FirstPoint - d->SecondPoint).manhattanLength();
        if (dist > 6)
          {
          // Suppress drop once a drag has started
          d->setState(vtkVgRegionWidgetPrivate::IgnoreDrop);
          d->BorderRepresentation->SetVisibility(false);
          d->BorderWidget->Render();
          }
        else
          {
          state = vtkVgRegionWidgetPrivate::Drop;
          }
        }
      if (state == vtkVgRegionWidgetPrivate::Drop)
        {
        d->BorderRepresentation->SetShowBorderToOn();
        d->updateDropPosition();
        }
      else
        {
        if (state == vtkVgRegionWidgetPrivate::DragDraw)
          {
          d->SecondPoint = QPoint(pos[0], pos[1]);
          int dist = (d->FirstPoint - d->SecondPoint).manhattanLength();
          if (dist > 6)
            {
            state = vtkVgRegionWidgetPrivate::PickSecond;
            d->setState(state);
            }
          }
        if (state == vtkVgRegionWidgetPrivate::PickSecond)
          {
          d->updateSecondCorner();
          }
        }
      break;
    case vtkCommand::EnterEvent:
      d->IsWithinView = true;
      if (state == vtkVgRegionWidgetPrivate::Drop ||
          state == vtkVgRegionWidgetPrivate::DragDrop)
        {
        d->BorderRepresentation->SetVisibility(true);
        d->BorderWidget->Render();
        }
      break;
    case vtkCommand::LeaveEvent:
      d->IsWithinView = false;
      if (state == vtkVgRegionWidgetPrivate::Drop ||
          state == vtkVgRegionWidgetPrivate::DragDrop)
        {
        d->BorderRepresentation->SetVisibility(false);
        d->BorderWidget->Render();
        }
      break;
    case vtkCommand::LeftButtonReleaseEvent:
      switch (state)
        {
        case vtkVgRegionWidgetPrivate::PickFirst:
        case vtkVgRegionWidgetPrivate::DragDraw:
          d->setState(vtkVgRegionWidgetPrivate::PickSecond);
          break;
        case vtkVgRegionWidgetPrivate::PickSecond:
          if (d->QuickMode)
            {
            emit this->dropDrawCompleted();
            emit this->completed();
            }
          else
            {
            // Done handling mouse events
            vtkDisconnect(d->Interactor->GetInteractorStyle(),
                          vtkCommand::NoEvent, this);
            vtkDisconnect(d->Interactor, vtkCommand::NoEvent, this);
            d->setState(vtkVgRegionWidgetPrivate::Manipulate);
            }
          break;
        case vtkVgRegionWidgetPrivate::Drop:
        case vtkVgRegionWidgetPrivate::DragDrop:
          emit this->completed();
          break;
        case vtkVgRegionWidgetPrivate::IgnoreDrop:
          d->setState(vtkVgRegionWidgetPrivate::Drop);
          d->BorderRepresentation->SetVisibility(d->IsWithinView);
          d->updateDropPosition();
          break;
        default: // Should never happen
          qWarning() << "in" << __FUNCTION__
                     << "at" << QString("%1:%2").arg(__FILE__).arg(__LINE__)
                     << "unhandled state" << state;
          break;
        }
      break;
    case vtkCommand::RightButtonPressEvent:
      emit (state == vtkVgRegionWidgetPrivate::Manipulate
            ? this->completed() : this->canceled());
    default:
      break;
    }
}

//-----------------------------------------------------------------------------
void vtkVgRegionWidget::beginSecondPick()
{
  QTE_D(vtkVgRegionWidget);

  if (d->state() == vtkVgRegionWidgetPrivate::DragDraw)
    {
    d->setState(vtkVgRegionWidgetPrivate::PickSecond);
    }
}

//END vtkVgRegionWidget
