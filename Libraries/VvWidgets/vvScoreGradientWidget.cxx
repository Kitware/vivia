// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvScoreGradientWidget.h"
#include "ui_vvScoreGradientWidget.h"

QTE_IMPLEMENT_D_FUNC(vvScoreGradientWidget)

//-----------------------------------------------------------------------------
class vvScoreGradientWidgetPrivate
{
public:
  Ui::vvScoreGradientWidget UI;
  vvScoreGradient gradient;
};

//-----------------------------------------------------------------------------
vvScoreGradientWidget::vvScoreGradientWidget(QWidget* parent) :
  QWidget(parent),
  d_ptr(new vvScoreGradientWidgetPrivate)
{
  QTE_REGISTER_METATYPE(vvScoreGradient);

  QTE_D(vvScoreGradientWidget);
  d->UI.setupUi(this);

  d->UI.stops->setStops(this->stops());
  d->UI.gradient->setGradient(this->gradient());

  connect(d->UI.interpolationMode, SIGNAL(currentIndexChanged(int)),
          this, SLOT(updatePreview()));
  connect(d->UI.stops, SIGNAL(stopsChanged(vvScoreGradient)),
          this, SLOT(setStops(vvScoreGradient)));

  // FIXME: adding/removing stops not implemented yet
  d->UI.addStop->setEnabled(false);
  d->UI.removeStop->setEnabled(false);
}

//-----------------------------------------------------------------------------
vvScoreGradientWidget::~vvScoreGradientWidget()
{
}

//-----------------------------------------------------------------------------
qtGradient vvScoreGradientWidget::gradient() const
{
  return this->stops().gradient(this->interpolationMode());
}

//-----------------------------------------------------------------------------
vvScoreGradient vvScoreGradientWidget::stops() const
{
  QTE_D_CONST(vvScoreGradientWidget);
  return d->gradient;
}

//-----------------------------------------------------------------------------
void vvScoreGradientWidget::setStops(vvScoreGradient newStops)
{
  QTE_D(vvScoreGradientWidget);
  if (d->gradient != newStops)
    {
    d->gradient = newStops;
    d->UI.stops->setStops(newStops);
    d->UI.gradient->setGradient(this->gradient());
    emit this->stopsChanged(newStops);
    }
}

//-----------------------------------------------------------------------------
qtGradient::InterpolationMode vvScoreGradientWidget::interpolationMode() const
{
  QTE_D_CONST(vvScoreGradientWidget);
  switch (d->UI.interpolationMode->currentIndex())
    {
    case 1:
      return qtGradient::InterpolateLinear;
    case 2:
      return qtGradient::InterpolateCubic;
    default:
      return qtGradient::InterpolateDiscrete;
    }
}

//-----------------------------------------------------------------------------
void vvScoreGradientWidget::setInterpolationMode(
  qtGradient::InterpolationMode mode)
{
  QTE_D(vvScoreGradientWidget);
  switch (mode & qtGradient::InterpolateFunctionMask)
    {
    case qtGradient::InterpolateLinear:
      d->UI.interpolationMode->setCurrentIndex(1);
      break;
    case qtGradient::InterpolateCubic:
      d->UI.interpolationMode->setCurrentIndex(2);
      break;
    default:
      d->UI.interpolationMode->setCurrentIndex(0);
      break;
    }
}

//-----------------------------------------------------------------------------
void vvScoreGradientWidget::updatePreview()
{
  QTE_D(vvScoreGradientWidget);
  d->UI.gradient->setGradient(this->gradient());
}
