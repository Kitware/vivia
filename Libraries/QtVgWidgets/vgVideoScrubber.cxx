// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgVideoScrubber.h"

#include <QApplication>
#include <QResizeEvent>
#include <QSlider>
#include <QWheelEvent>

#include <qtDoubleSlider.h>
#include <qtMath.h>
#include <qtScopedValueChange.h>

QTE_IMPLEMENT_D_FUNC(vgVideoScrubber)

//BEGIN vgVideoScrubberPrivate

//-----------------------------------------------------------------------------
class vgVideoScrubberPrivate : public QObject
{
public:
  vgVideoScrubberPrivate(vgVideoScrubber* q);

  void useTimeSlider();
  void useFrameSlider();

  void setValue(vgTimeStamp, vg::SeekMode);

  virtual bool eventFilter(QObject*, QEvent*);

  QScopedPointer<QSlider> frameSlider;
  QScopedPointer<qtDoubleSlider> timeSlider;

  vgTimeStamp min, max, val, sStep, wStep, pStep;
  double wAccum;
  bool wAccumNoClear;

  vg::SeekMode seekDirection;

protected:
  QTE_DECLARE_PUBLIC_PTR(vgVideoScrubber)

private:
  QTE_DECLARE_PUBLIC(vgVideoScrubber)
};

//-----------------------------------------------------------------------------
vgVideoScrubberPrivate::vgVideoScrubberPrivate(vgVideoScrubber* q) :
  wAccum(0.0),
  wAccumNoClear(false),
  q_ptr(q)
{
}

//-----------------------------------------------------------------------------
void vgVideoScrubberPrivate::useTimeSlider()
{
  QTE_Q(vgVideoScrubber);

  this->frameSlider.reset();

  if (!this->timeSlider)
    {
    this->timeSlider.reset(new qtDoubleSlider(q));
    this->timeSlider->setGeometry(QRect(QPoint(0, 0), q->size()));
    this->timeSlider->installEventFilter(this);

    this->timeSlider->setRange(this->min.Time, this->max.Time);
    this->timeSlider->setValue(this->val.Time);
    this->timeSlider->setSingleStep(this->sStep.Time);
    this->timeSlider->setPageStep(this->pStep.Time);

    q->connect(this->timeSlider.data(), SIGNAL(valueChanged(double)),
               q, SLOT(updateValue()));
    q->connect(this->timeSlider.data(), SIGNAL(actionTriggered(int)),
               q, SLOT(setDirectionFromSliderAction(int)));
    }
}

//-----------------------------------------------------------------------------
void vgVideoScrubberPrivate::useFrameSlider()
{
  QTE_Q(vgVideoScrubber);

  this->timeSlider.reset();

  if (!this->frameSlider)
    {
    this->frameSlider.reset(new QSlider(q));
    this->frameSlider->setGeometry(QRect(QPoint(0, 0), q->size()));
    this->frameSlider->installEventFilter(this);

    this->frameSlider->setRange(this->min.FrameNumber, this->max.FrameNumber);
    this->frameSlider->setValue(this->val.FrameNumber);
    this->frameSlider->setSingleStep(this->sStep.FrameNumber);
    this->frameSlider->setPageStep(this->pStep.FrameNumber);

    q->connect(this->frameSlider.data(), SIGNAL(valueChanged(int)),
               q, SLOT(updateValue()));
    q->connect(this->frameSlider.data(), SIGNAL(actionTriggered(int)),
               q, SLOT(setDirectionFromSliderAction(int)));
    }
}

//-----------------------------------------------------------------------------
void vgVideoScrubberPrivate::setValue(vgTimeStamp ts, vg::SeekMode sd)
{
  QTE_Q(vgVideoScrubber);

  this->val = ts;
  if (!this->wAccumNoClear)
    {
    this->wAccum = 0.0;
    }

  emit q->valueChanged(ts, sd);
}

//-----------------------------------------------------------------------------
bool vgVideoScrubberPrivate::eventFilter(QObject*, QEvent* e)
{
  if (e->type() == QEvent::Wheel)
    {
    QTE_Q(vgVideoScrubber);
    q->wheelEvent(static_cast<QWheelEvent*>(e));
    return true;
    }
  return false;
}

//END vgVideoScrubberPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgVideoScrubber

//-----------------------------------------------------------------------------
vgVideoScrubber::vgVideoScrubber(QWidget* parent) :
  QWidget(parent),
  d_ptr(new vgVideoScrubberPrivate(this))
{
  QTE_D(vgVideoScrubber);
  d->sStep = vgTimeStamp(1e-4, 1);
  d->pStep = vgTimeStamp(1.0, 10);
  this->setRange(vgTimeStamp(0u), vgTimeStamp(0u));
}

//-----------------------------------------------------------------------------
vgVideoScrubber::~vgVideoScrubber()
{
}

//-----------------------------------------------------------------------------
vgTimeStamp vgVideoScrubber::minimum() const
{
  QTE_D_CONST(vgVideoScrubber);
  return d->min;
}

//-----------------------------------------------------------------------------
vgTimeStamp vgVideoScrubber::maximum() const
{
  QTE_D_CONST(vgVideoScrubber);
  return d->max;
}

//-----------------------------------------------------------------------------
vgTimeStamp vgVideoScrubber::singleStep() const
{
  QTE_D_CONST(vgVideoScrubber);
  return d->sStep;
}

//-----------------------------------------------------------------------------
vgTimeStamp vgVideoScrubber::wheelStep() const
{
  QTE_D_CONST(vgVideoScrubber);
  return d->wStep;
}

//-----------------------------------------------------------------------------
vgTimeStamp vgVideoScrubber::pageStep() const
{
  QTE_D_CONST(vgVideoScrubber);
  return d->pStep;
}

//-----------------------------------------------------------------------------
vgTimeStamp vgVideoScrubber::value() const
{
  QTE_D_CONST(vgVideoScrubber);
  return d->val;
}

//-----------------------------------------------------------------------------
bool vgVideoScrubber::isRangeEmpty() const
{
  QTE_D_CONST(vgVideoScrubber);

  if (!(d->min.IsValid() && d->max.IsValid()))
    {
    return true;
    }

  const vgTimeStamp range = d->max - d->min;
  if (!range.IsValid())
    {
    return true;
    }

  return !((range.HasTime() && range.Time > 0) ||
           (range.HasFrameNumber() && range.FrameNumber > 0));
}

//-----------------------------------------------------------------------------
void vgVideoScrubber::setMinimum(vgTimeStamp min)
{
  this->setRange(min, this->maximum());
}

//-----------------------------------------------------------------------------
void vgVideoScrubber::setMaximum(vgTimeStamp max)
{
  this->setRange(this->minimum(), max);
}

//-----------------------------------------------------------------------------
void vgVideoScrubber::setRange(vgTimeStamp min, vgTimeStamp max)
{
  QTE_D(vgVideoScrubber);

  min = qMin(min, max);
  max = qMax(min, max);

  if (min != d->min || max != d->max)
    {
    // Update internal range
    d->min = min;
    d->max = max;

    // Make sure we are using the appropriate widget, and update its range
    if (min.HasTime() && max.HasTime())
      {
      d->useTimeSlider();
      d->timeSlider->setRange(min.Time, max.Time);
      }
    else if (min.HasFrameNumber() && max.HasFrameNumber())
      {
      d->useFrameSlider();
      d->frameSlider->setRange(min.FrameNumber, max.FrameNumber);
      }

    emit this->rangeChanged(min, max);
    }
}

//-----------------------------------------------------------------------------
void vgVideoScrubber::setSingleStep(vgTimeStamp step)
{
  QTE_D(vgVideoScrubber);

  step.HasTime() && (d->sStep.Time = step.Time);
  step.HasFrameNumber() && (d->sStep.FrameNumber = step.FrameNumber);

  (d->timeSlider
   ? d->timeSlider->setSingleStep(d->sStep.Time)
   : d->frameSlider->setSingleStep(d->sStep.FrameNumber));
}

//-----------------------------------------------------------------------------
void vgVideoScrubber::setWheelStep(vgTimeStamp step)
{
  QTE_D(vgVideoScrubber);

  // Set regardless; unlike single/page step, wheel step can be set to 'none'
  d->wStep = step;
}

//-----------------------------------------------------------------------------
void vgVideoScrubber::setPageStep(vgTimeStamp step)
{
  QTE_D(vgVideoScrubber);

  step.HasTime() && (d->pStep.Time = step.Time);
  step.HasFrameNumber() && (d->pStep.FrameNumber = step.FrameNumber);

  d->timeSlider
  ? d->timeSlider->setPageStep(d->pStep.Time)
  : d->frameSlider->setPageStep(d->pStep.FrameNumber);
}

//-----------------------------------------------------------------------------
void vgVideoScrubber::setValue(vgTimeStamp val, vg::SeekMode mode)
{
  QTE_D(vgVideoScrubber);
  if (d->timeSlider && val.HasTime())
    {
    qtScopedBlockSignals bs(d->timeSlider.data());
    d->timeSlider->setValue(val.Time);
    d->setValue(val, mode);
    }
  else if (d->frameSlider && val.HasFrameNumber())
    {
    qtScopedBlockSignals bs(d->frameSlider.data());
    d->frameSlider->setValue(val.FrameNumber);
    d->setValue(val, mode);
    }
}

//-----------------------------------------------------------------------------
void vgVideoScrubber::setDirectionFromSliderAction(int action)
{
  QTE_D(vgVideoScrubber);

  switch (action)
    {
    case QAbstractSlider::SliderSingleStepAdd:
    case QAbstractSlider::SliderPageStepAdd:
      d->seekDirection = vg::SeekLowerBound;
      break;
    case QAbstractSlider::SliderSingleStepSub:
    case QAbstractSlider::SliderPageStepSub:
      d->seekDirection = vg::SeekUpperBound;
      break;
    default:
      d->seekDirection = vg::SeekNearest;
      break;
    }
}

//-----------------------------------------------------------------------------
void vgVideoScrubber::updateValue()
{
  QTE_D(vgVideoScrubber);
  vgTimeStamp ts = (d->timeSlider
                    ? vgTimeStamp::fromTime(d->timeSlider->value())
                    : vgTimeStamp::fromFrameNumber(d->frameSlider->value()));
  d->setValue(ts, d->seekDirection);
}

//-----------------------------------------------------------------------------
void vgVideoScrubber::wheelEvent(QWheelEvent* e)
{
  if (!e->delta())
    {
    return;
    }

  QTE_D(vgVideoScrubber);

  // Mark event as handled
  e->accept();

  // Calculate amount by which to change the value
  const double delta = d->wAccum + (e->delta() / 120.0);
  vgTimeStamp step;
  if (e->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier))
    {
    step = d->pStep;
    }
  else
    {
    if (d->timeSlider ? d->wStep.HasTime() : d->wStep.HasFrameNumber())
      {
      step = d->wStep;
      }
    else
      {
      const unsigned int wsl = QApplication::wheelScrollLines();
      step.FrameNumber = wsl * d->sStep.FrameNumber;
      step.Time = wsl * d->sStep.Time;
      }
    }

  // Calculate new position
  vgTimeStamp ts;
  vg::SeekMode sd = (delta > 0 ? vg::SeekLowerBound : vg::SeekUpperBound);
  if (d->timeSlider)
    {
    // Calculate new time
    const double t =
      qBound(d->min.Time, d->max.Time, d->val.Time + (delta * step.Time));
    ts = vgTimeStamp::fromTime(t);

    // Time is continuous; no need to accumulate small changes until we have
    // enough to produce a change
    d->wAccum = 0.0;
    }
  else
    {
    // Accumulate partial frames so we will eventually generate a whole frame
    // step, in case of many events with very small delta
    d->wAccum += delta * step.FrameNumber;
    const double sf = floor(d->wAccum);
    d->wAccum -= sf;
    qint64 stepFrames = qRound64(sf);
    if (!stepFrames)
      {
      // We haven't accumulated a full frame yet, so there is nothing to do
      return;
      }

    // Calculate new frame number
    const qint64 min = d->min.FrameNumber, max = d->max.FrameNumber;
    qint64 frame = d->val.FrameNumber + stepFrames;
    if (frame < min)
      {
      // Clear accumulator if at lower bound
      d->wAccum = 0.0;
      frame = min;
      }
    else if (sf + frame > max)
      {
      // Clear accumulator if at upper bound
      d->wAccum = 0.0;
      frame = max;
      }
    ts = vgTimeStamp::fromFrameNumber(static_cast<unsigned int>(frame));
    }

  // Set new value
  qtScopedValueChange<bool> bca(d->wAccumNoClear, true);
  this->setValue(ts, sd);
}

//-----------------------------------------------------------------------------
void vgVideoScrubber::resizeEvent(QResizeEvent* e)
{
  QTE_D(vgVideoScrubber);
  if (d->timeSlider)
    {
    d->timeSlider->resize(e->size());
    }
  else
    {
    d->frameSlider->resize(e->size());
    }
  e->accept();
}

//END vgVideoScrubber
