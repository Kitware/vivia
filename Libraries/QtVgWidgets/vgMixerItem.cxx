// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgMixerItem.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QtCore>

#include <qtDoubleSlider.h>
#include <qtScopedValueChange.h>

#include <cmath>

#include "vgMixerDrawer.h"
#include "vgMixerWidget.h"

QTE_IMPLEMENT_D_FUNC(vgMixerItem)

//BEGIN vgMixerItemPrivate

//-----------------------------------------------------------------------------
class vgMixerItemPrivate
{
public:
  void setWidgetRangeAndStep();
  void setWidgetValue(double value);

  vgMixerDrawer* parent_;

  QCheckBox* checkbox_;
  QCheckBox* checkboxInverted_;
  qtDoubleSlider* slider_;
  QDoubleSpinBox* spinbox_;

  int key_;
  double value_;
  double minimum_, maximum_, range_;
  double singleStep_, pageStep_;
};

//-----------------------------------------------------------------------------
void vgMixerItemPrivate::setWidgetRangeAndStep()
{
  qtScopedBlockSignals bsSlider(this->slider_), bsSpinBox(this->spinbox_);

  this->slider_->setRange(this->minimum_, this->maximum_);
  this->slider_->setSingleStep(this->singleStep_);
  this->slider_->setPageStep(this->pageStep_);

  this->spinbox_->setRange(this->minimum_, this->maximum_);
  this->spinbox_->setSingleStep(this->singleStep_);

  // Determine number of decimal places to display
  int precision = 0;
  double ss = this->singleStep_;
  while (!qFuzzyCompare(ss, floor(ss + 0.5)))
    {
    ++precision;
    ss *= 10;
    }

  this->spinbox_->setDecimals(precision);
}

//-----------------------------------------------------------------------------
void vgMixerItemPrivate::setWidgetValue(double value)
{
  qtScopedBlockSignals bsSlider(this->slider_), bsSpinBox(this->spinbox_);

  this->spinbox_->setValue(value);
  this->slider_->setValue(value);
}

//END vgMixerItemPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgMixerItem

//-----------------------------------------------------------------------------
vgMixerItem::vgMixerItem(int key, const QString& text, vgMixerDrawer* drawer) :
  qtDrawer(drawer),
  d_ptr(new vgMixerItemPrivate)
{
  QTE_D(vgMixerItem);

  d->parent_ = drawer;
  d->key_ = key;
  d->value_ = 0.0;

  d->checkbox_ = new QCheckBox(text.trimmed());
  d->checkboxInverted_ = new QCheckBox("Invert");
  d->slider_ = new qtDoubleSlider();
  d->spinbox_ = new QDoubleSpinBox();

  d->slider_->setMinimumSize(QSize(40, 4));

  this->setRange(0.0, 1.0, 0.01, 0.1);
  d->setWidgetValue(d->value_);

  d->checkbox_->setCheckState(Qt::Checked);
  d->checkboxInverted_->setCheckState(Qt::Unchecked);

  connect(d->slider_, SIGNAL(valueChanged(double)),
          this, SLOT(setValueFromSlider(double)));
  connect(d->spinbox_, SIGNAL(valueChanged(double)),
          this, SLOT(setValueFromSpinBox(double)));

  connect(d->checkbox_, SIGNAL(toggled(bool)),
          this, SLOT(emitStateChanged(bool)));
  connect(d->checkboxInverted_, SIGNAL(toggled(bool)),
          this, SLOT(emitInvertedChanged(bool)) );

  this->addWidget(d->checkbox_, 0);
  this->addWidget(d->slider_, 1);
  this->addWidget(d->spinbox_, 2);
   this->addWidget(d->checkboxInverted_, 3);
}

//-----------------------------------------------------------------------------
vgMixerItem::~vgMixerItem()
{
}

//-----------------------------------------------------------------------------
int vgMixerItem::key() const
{
  QTE_D_CONST(vgMixerItem);
  return d->key_;
}

//-----------------------------------------------------------------------------
bool vgMixerItem::state() const
{
  QTE_D_CONST(vgMixerItem);
  return d->checkbox_->isChecked();
}

//-----------------------------------------------------------------------------
bool vgMixerItem::isInverted() const
{
  QTE_D_CONST(vgMixerItem);
  return d->checkboxInverted_->isChecked();
}

//-----------------------------------------------------------------------------
double vgMixerItem::value() const
{
  QTE_D_CONST(vgMixerItem);
  return d->slider_->value();
}

//-----------------------------------------------------------------------------
double vgMixerItem::minimum() const
{
  QTE_D_CONST(vgMixerItem);
  return d->slider_->minimum();
}

//-----------------------------------------------------------------------------
double vgMixerItem::maximum() const
{
  QTE_D_CONST(vgMixerItem);
  return d->slider_->maximum();
}

//-----------------------------------------------------------------------------
double vgMixerItem::singleStep() const
{
  QTE_D_CONST(vgMixerItem);
  return d->slider_->singleStep();
}

//-----------------------------------------------------------------------------
double vgMixerItem::pageStep() const
{
  QTE_D_CONST(vgMixerItem);
  return d->slider_->pageStep();
}

//-----------------------------------------------------------------------------
vgMixerDrawer* vgMixerItem::parent()
{
  QTE_D(vgMixerItem);
  return d->parent_;
}

//-----------------------------------------------------------------------------
void vgMixerItem::setText(const QString& text)
{
  QTE_D(vgMixerItem);
  d->checkbox_->setText(text.trimmed());
}

//-----------------------------------------------------------------------------
void vgMixerItem::setInvertedText(const QString& text)
{
  QTE_D(vgMixerItem);
  d->checkboxInverted_->setText(text.trimmed());
}

//-----------------------------------------------------------------------------
void vgMixerItem::setMinimum(double minimum)
{
  QTE_D(vgMixerItem);
  this->setRange(minimum, d->maximum_, d->singleStep_, d->pageStep_);
}

//-----------------------------------------------------------------------------
void vgMixerItem::setMaximum(double maximum)
{
  QTE_D(vgMixerItem);
  this->setRange(d->minimum_, maximum, d->singleStep_, d->pageStep_);
}

//-----------------------------------------------------------------------------
void vgMixerItem::setSingleStep(double step)
{
  QTE_D(vgMixerItem);
  this->setRange(d->minimum_, d->maximum_, step, d->pageStep_);
}

//-----------------------------------------------------------------------------
void vgMixerItem::setPageStep(double step)
{
  QTE_D(vgMixerItem);
  this->setRange(d->minimum_, d->maximum_, d->singleStep_, step);
}

//-----------------------------------------------------------------------------
void vgMixerItem::setRange(double minimum, double maximum)
{
  QTE_D(vgMixerItem);
  this->setRange(minimum, maximum, d->singleStep_, d->pageStep_);
}

//-----------------------------------------------------------------------------
void vgMixerItem::setRange(
  double minimum, double maximum, double singleStep, double pageStep)
{
  QTE_D(vgMixerItem);

  // First, make sure our range is reasonable
  if (!(qIsFinite(minimum) && qIsFinite(maximum)))
    {
    return;
    }
  minimum = qMin(minimum, maximum);
  maximum = qMax(minimum, maximum);
  d->minimum_ = minimum;
  d->maximum_ = maximum;
  const double range = d->range_ = maximum - minimum;

  // Next make sure our steps are reasonable
  qIsFinite(pageStep) || (pageStep = range * 0.1);
  qIsFinite(singleStep) || (singleStep = pageStep * 0.1);
  pageStep = qMin(fabs(pageStep), range);
  singleStep = qMin(fabs(singleStep), pageStep);
  d->singleStep_ = singleStep;
  d->pageStep_ = pageStep;

  // Update the widgets
  d->setWidgetRangeAndStep();

  // Reset the widgets' displayed values, and check if the value changed
  const double value = qBound(minimum, d->value_, maximum);
  d->setWidgetValue(value);

  if (!qFuzzyCompare(value, d->value_))
    {
    // Value changed due to range change
    d->value_ = value;
    emit this->valueChanged(d->key_, value);
    }
}

//-----------------------------------------------------------------------------
void vgMixerItem::setState(bool v)
{
  QTE_D(vgMixerItem);

  if (d->checkbox_->isChecked() != v)
    {
    d->checkbox_->setChecked(v);
    emit this->stateChanged(d->key_, v);
    }
}

//-----------------------------------------------------------------------------
void vgMixerItem::setInverted(bool v)
{
  QTE_D(vgMixerItem);

  if (d->checkboxInverted_->isChecked() != v)
    {
    d->checkboxInverted_->setChecked(v);
    emit this->invertedChanged(d->key_, v);
    }
}

//-----------------------------------------------------------------------------
void vgMixerItem::setValue(double t)
{
  QTE_D(vgMixerItem);

  if (!qFuzzyCompare(d->value_, t))
    {
    d->value_ = t;
    d->setWidgetValue(t);
    emit this->valueChanged(d->key_, t);
    }
}

//-----------------------------------------------------------------------------
void vgMixerItem::setValueFromSlider(double newValue)
{
  QTE_D(vgMixerItem);

  qtScopedBlockSignals bs(d->spinbox_);
  d->spinbox_->setValue(newValue);

  if (!qFuzzyCompare(d->value_, newValue))
    {
    d->value_ = newValue;
    emit this->valueChanged(d->key_, newValue);
    }
}

//-----------------------------------------------------------------------------
void vgMixerItem::setValueFromSpinBox(double newValue)
{
  QTE_D(vgMixerItem);

  qtScopedBlockSignals bs(d->slider_);
  d->slider_->setValue(newValue);

  if (!qFuzzyCompare(d->value_, newValue))
    {
    d->value_ = newValue;
    emit this->valueChanged(d->key_, newValue);
    }
}

//-----------------------------------------------------------------------------
void vgMixerItem::emitStateChanged(bool state)
{
  QTE_D(vgMixerItem);
  emit this->stateChanged(d->key_, state);
}

//-----------------------------------------------------------------------------
void vgMixerItem::emitInvertedChanged(bool state)
{
  QTE_D(vgMixerItem);
  emit this->invertedChanged(d->key_, state);
}

//END vgMixerItem
