/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <limits>

#include <QSlider>
#include <QHBoxLayout>
#include <QLayoutItem>

#include <qtMath.h>

#include "vgFrameScrubber.h"
#include "vgFrameSpinBox.h"

#include "ctkRangeSlider.h"

QTE_IMPLEMENT_D_FUNC(vgFrameScrubber)

//-----------------------------------------------------------------------------
class vgFrameScrubberPrivate
{
public:
  vgFrameScrubberPrivate(vgFrameScrubber* parent);

  void updateFrameNumberRange(int minimum, int maximum);
  void updateTimeRange(double minimum, double maximum, double resolution);

  void setFrameNumber(int value);
  void setFrameNumbers(int minVal, int maxVal);
  void setTime(double value);
  void setTimes(double minVal, double maxVal);

  void setMinFrameNumber(int value);
  void setMinTime(double value);

  void setMinValue(int value,
                   bool keepWidth = false);

  void setValue(int value,
                bool keepWidth = false,
                bool forceSignal = false);

  void setValues(int minVal, int maxVal,
                 bool resized = false,
                 bool forceSignal = false);

  double timeFromSliderPosition(int value);

  void setIntervalModeEnabled(bool enabled);

protected:
  QTE_DECLARE_PUBLIC_PTR(vgFrameScrubber)

  void setFrameNumberMode();
  void setTimeMode();

  QHBoxLayout* layout_;
  ctkRangeSlider* slider_;
  vgFrameSpinBox* spinBox_;
  double timeScale_, timeOffset_;

private:
  QTE_DECLARE_PUBLIC(vgFrameScrubber)
};

//-----------------------------------------------------------------------------
vgFrameScrubberPrivate::vgFrameScrubberPrivate(vgFrameScrubber* parent) :
  q_ptr(parent), spinBox_(0), timeScale_(0.0), timeOffset_(0.0)
{
  QTE_Q(vgFrameScrubber);

  this->layout_ = new QHBoxLayout();
  this->slider_ = new ctkRangeSlider(Qt::Horizontal);
  this->layout_->addWidget(this->slider_);
  q->setLayout(this->layout_);

  q->connect(this->slider_, SIGNAL(valueChanged(int)),
             q, SLOT(updateValueFromSlider(int)));

  q->connect(this->slider_, SIGNAL(valuesChanged(int, int)),
             q, SLOT(updateValuesFromSlider(int, int)));
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::setFrameNumberMode()
{
  if (this->spinBox_)
    {
    return;
    }

  QTE_Q(vgFrameScrubber);

  this->spinBox_ = new vgFrameSpinBox();
  this->layout_->addWidget(this->spinBox_);

  q->connect(this->spinBox_, SIGNAL(valueChanged(int)),
             q, SLOT(updateValueFromSlider(int)));
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::setTimeMode()
{
  if (!this->spinBox_)
    {
    return;
    }

  QLayoutItem* item = this->layout_->takeAt(1);
  delete item;
  delete this->spinBox_;
  this->spinBox_ = 0;
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::updateFrameNumberRange(int minimum, int maximum)
{
  QTE_Q(vgFrameScrubber);

  const int oldMinVal = this->slider_->minimumValue();
  const int oldMaxVal = this->slider_->maximumValue();

  this->setFrameNumberMode();
  this->slider_->blockSignals(true);
  this->spinBox_->blockSignals(true);

  if (maximum < 0)
    {
    minimum = maximum = -1;
    }
  if (minimum > maximum)
    {
    minimum = maximum;
    }

  this->slider_->setEnabled(maximum > minimum);
  this->slider_->setRange(minimum, maximum);
  this->spinBox_->setRange(minimum, maximum);

  // Since signals are blocked, the ctkRangeSlider doesn't get the message that
  // the QAbstractSlider's range has changed, and thus doesn't 'fix' the
  // previous values to fit into the new range. So we need to make it happen
  // here, by re-setting the values and letting them get clamped appropriately.
  this->slider_->setValues(this->slider_->minimumValue(),
                           this->slider_->maximumValue());

  this->slider_->blockSignals(false);
  this->spinBox_->blockSignals(false);

  const int minVal = this->slider_->minimumValue();
  const int maxVal = this->slider_->maximumValue();

  bool minChanged = minVal != oldMinVal;
  bool maxChanged = maxVal != oldMaxVal;

  if (minChanged || maxChanged)
    {
    emit q->frameNumbersChanged(minVal, maxVal, false);
    if (minChanged)
      {
      emit q->minFrameNumberChanged(minVal);
      }
    if (maxChanged)
      {
      emit q->frameNumberChanged(maxVal);
      }
    }
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::updateTimeRange(
  double minimum, double maximum, double resolution)
{
  QTE_Q(vgFrameScrubber);

  const double oldMinTime =
    this->timeFromSliderPosition(this->slider_->minimumValue());

  const double oldMaxTime =
    this->timeFromSliderPosition(this->slider_->maximumValue());

  this->setTimeMode();
  this->slider_->blockSignals(true);

  if (minimum > maximum)
    {
    maximum = qQNaN();
    }

  if (!qIsFinite(minimum) || !qIsFinite(maximum))
    {
    this->slider_->setEnabled(false);
    this->slider_->setRange(0, 0);
    }
  else
    {
    this->timeOffset_ = minimum;
    this->slider_->setEnabled(true);

    static const int max_int = std::numeric_limits<int>::max();
    const double delta = maximum - minimum, ri = 1.0 / resolution;
    if (delta * ri >= (double)max_int)
      {
      this->timeScale_ = delta / (double)max_int;
      this->slider_->setRange(0, max_int);
      }
    else
      {
      this->timeScale_ = resolution;
      this->slider_->setRange(0, static_cast<int>(ceil(delta * ri)));
      }
    }

  // Since signals are blocked, the ctkRangeSlider doesn't get the message that
  // the QAbstractSlider's range has changed, and thus doesn't 'fix' the
  // previous values to fit into the new range. So we need to make it happen
  // here, by re-setting the values and letting them get clamped appropriately.
  this->slider_->setValues(this->slider_->minimumValue(),
                           this->slider_->maximumValue());

  this->slider_->blockSignals(false);

  double minTime = this->timeFromSliderPosition(this->slider_->minimumValue());
  double maxTime = this->timeFromSliderPosition(this->slider_->maximumValue());

  bool minChanged = minTime != oldMinTime;
  bool maxChanged = maxTime != oldMaxTime;

  if (minChanged || maxChanged)
    {
    emit q->timesChanged(minTime, maxTime, false);
    if (minChanged)
      {
      emit q->minTimeChanged(minTime);
      }
    if (maxChanged)
      {
      emit q->timeChanged(maxTime);
      }
    }
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::setFrameNumber(int value)
{
  if (!this->spinBox_)
    {
    return;
    }

  this->setValue(value);
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::setFrameNumbers(int minVal, int maxVal)
{
  if (!this->spinBox_)
    {
    return;
    }

  this->setValues(minVal, maxVal);
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::setTime(double value)
{
  if (this->spinBox_)
    {
    return;
    }

  static const int max_int = std::numeric_limits<int>::max();
  const double pos = (value - this->timeOffset_) / this->timeScale_;
  this->setValue(pos > max_int ? max_int : qRound(pos));
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::setTimes(double minVal, double maxVal)
{
  if (this->spinBox_)
    {
    return;
    }

  static const int max_int = std::numeric_limits<int>::max();
  const double pos1 = (minVal - this->timeOffset_) / this->timeScale_;
  const double pos2 = (maxVal - this->timeOffset_) / this->timeScale_;
  this->setValues(pos1 > max_int ? max_int : qRound(pos1),
                  pos2 > max_int ? max_int : qRound(pos2));
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::setMinFrameNumber(int value)
{
  if (!this->spinBox_)
    {
    return;
    }

  this->setMinValue(value);
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::setMinTime(double value)
{
  if (this->spinBox_)
    {
    return;
    }

  static const int max_int = std::numeric_limits<int>::max();
  const double pos = (value - this->timeOffset_) / this->timeScale_;
  this->setMinValue(pos > max_int ? max_int : qRound(pos));
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::setValue(int value,
                                      bool keepWidth,
                                      bool forceSignal)
{
  if (keepWidth)
    {
    int width = this->slider_->maximumValue() - this->slider_->minimumValue();
    this->setValues(value - width, value);
    }
  else
    {
    this->setValues(value, value, forceSignal);
    }
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::setMinValue(int value,
                                         bool keepWidth)
{
  if (keepWidth)
    {
    int width = this->slider_->maximumValue() - this->slider_->minimumValue();
    this->setValues(value, value + width);
    }
  else
    {
    this->setValues(value, this->slider_->maximumValue());
    }
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::setValues(int minVal,
                                       int maxVal,
                                       bool resized,
                                       bool forceSignal)
{
  int prevMin = this->slider_->minimumValue();
  int prevMax = this->slider_->maximumValue();
  this->slider_->blockSignals(true);
  this->slider_->setValues(minVal, maxVal);
  this->slider_->setValue(maxVal); // value == maximumValue
  this->slider_->blockSignals(false);

  bool minChanged = this->slider_->minimumValue() != prevMin;
  bool maxChanged = this->slider_->maximumValue() != prevMax;

  if (this->spinBox_)
    {
    this->spinBox_->blockSignals(true);
    this->spinBox_->setValue(maxVal);
    this->spinBox_->blockSignals(false);
    }

  if (forceSignal || minChanged || maxChanged)
    {
    QTE_Q(vgFrameScrubber);

    if (this->spinBox_)
      {
      emit q->frameNumbersChanged(minVal, maxVal, resized);
      if (minChanged || forceSignal)
        {
        emit q->minFrameNumberChanged(minVal);
        }
      if (maxChanged || forceSignal)
        {
        emit q->frameNumberChanged(maxVal);
        }
      }
    else
      {
      double minTime = this->timeFromSliderPosition(minVal);
      double maxTime = this->timeFromSliderPosition(maxVal);

      emit q->timesChanged(minTime, maxTime, resized);
      if (minChanged || forceSignal)
        {
        emit q->minTimeChanged(minTime);
        }
      if (maxChanged || forceSignal)
        {
        emit q->timeChanged(maxTime);
        }
      }
    }
}

//-----------------------------------------------------------------------------
double vgFrameScrubberPrivate::timeFromSliderPosition(int value)
{
  if (this->spinBox_)
    {
    return qQNaN();
    }

  return this->timeOffset_ + (this->timeScale_ * (double)value);
}

//-----------------------------------------------------------------------------
void vgFrameScrubberPrivate::setIntervalModeEnabled(bool enabled)
{
  this->slider_->setMinHandleShown(enabled);
}

//-----------------------------------------------------------------------------
vgFrameScrubber::vgFrameScrubber(QWidget* parent) :
  QWidget(parent),
  d_ptr(new vgFrameScrubberPrivate(this))
{
}

//-----------------------------------------------------------------------------
vgFrameScrubber::~vgFrameScrubber()
{
  QTE_D(vgFrameScrubber);
  delete d;
}

//-----------------------------------------------------------------------------
void vgFrameScrubber::setFrameNumberRange(int minimumFrameNumber,
                                          int maximumFrameNumber)
{
  QTE_D(vgFrameScrubber);
  d->updateFrameNumberRange(minimumFrameNumber, maximumFrameNumber);
}

//-----------------------------------------------------------------------------
void vgFrameScrubber::setFrameNumber(int value)
{
  QTE_D(vgFrameScrubber);
  d->setFrameNumber(value);
}

//-----------------------------------------------------------------------------
void vgFrameScrubber::setFrameNumbers(int minVal, int maxVal)
{
  QTE_D(vgFrameScrubber);
  d->setFrameNumbers(minVal, maxVal);
}

//-----------------------------------------------------------------------------
void vgFrameScrubber::setMinFrameNumber(int value)
{
  QTE_D(vgFrameScrubber);
  d->setMinFrameNumber(value);
}

//-----------------------------------------------------------------------------
void vgFrameScrubber::setTimeRange(double minimumTime, double maximumTime,
                                   double resolution)
{
  QTE_D(vgFrameScrubber);
  d->updateTimeRange(minimumTime, maximumTime, resolution);
}

//-----------------------------------------------------------------------------
void vgFrameScrubber::setTime(double value)
{
  QTE_D(vgFrameScrubber);
  d->setTime(value);
}

//-----------------------------------------------------------------------------
void vgFrameScrubber::setTimes(double minVal, double maxVal)
{
  QTE_D(vgFrameScrubber);
  d->setTimes(minVal, maxVal);
}

//-----------------------------------------------------------------------------
void vgFrameScrubber::setMinTime(double value)
{
  QTE_D(vgFrameScrubber);
  d->setMinTime(value);
}

//-----------------------------------------------------------------------------
void vgFrameScrubber::setIntervalModeEnabled(bool enabled)
{
  QTE_D(vgFrameScrubber);
  d->setIntervalModeEnabled(enabled);
}

//-----------------------------------------------------------------------------
void vgFrameScrubber::updateValueFromSlider(int value)
{
  QTE_D(vgFrameScrubber);
  d->setValue(value, true, true);
}

//-----------------------------------------------------------------------------
void vgFrameScrubber::updateValuesFromSlider(int minVal, int maxVal)
{
  QTE_D(vgFrameScrubber);
  d->setValues(minVal, maxVal, true, true);
}
