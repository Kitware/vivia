// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgFrameSpinBox.h"

#include <QDebug>
#include <QLineEdit>

QTE_IMPLEMENT_D_FUNC(vgFrameSpinBox)

//-----------------------------------------------------------------------------
class vgFrameSpinBoxPrivate
{
public:
  vgFrameSpinBoxPrivate() : nextDirection(vg::SeekUnspecified) {}

  // Bypass Qt API's 'const' modifier on QSpinBox valueFromText()/fixup()
  mutable vg::SeekMode nextDirection;
};

//-----------------------------------------------------------------------------
vgFrameSpinBox::vgFrameSpinBox(QWidget* parent) :
  QSpinBox(parent),
  d_ptr(new vgFrameSpinBoxPrivate)
{
  this->setAlignment(Qt::AlignRight);
  connect(this, SIGNAL(valueChanged(int)),
          this, SLOT(emitValueChangeWithDirection(int)));
}

//-----------------------------------------------------------------------------
vgFrameSpinBox::~vgFrameSpinBox()
{
}

//-----------------------------------------------------------------------------
void vgFrameSpinBox::setRange(int minimum, int maximum)
{
  QTE_D(vgFrameSpinBox);

  // Set formatting options and read-only flag
  if (maximum == minimum)
    {
    this->lineEdit()->setReadOnly(true);
    this->setSpecialValueText(minimum < 0 ? QString("(N/A)") : QString());
    this->setSuffix(QString());
    }
  else
    {
    this->lineEdit()->setReadOnly(this->isReadOnly());
    this->setSpecialValueText(QString());
    if (minimum <= 1)
      {
      this->setSuffix(QString(" / %1").arg(maximum));
      }
    else
      {
      this->setSuffix(QString(" / (%2 - %1)").arg(maximum).arg(minimum));
      }
    }

  // Update the range, and temporarily set seek direction in case the value
  // changes as a result
  d->nextDirection = vg::SeekNearest;
  QSpinBox::setRange(minimum, maximum);
  d->nextDirection = vg::SeekUnspecified;
}

//-----------------------------------------------------------------------------
void vgFrameSpinBox::stepBy(int steps)
{
  if (steps)
    {
    QTE_D(vgFrameSpinBox);
    d->nextDirection = (steps > 0 ? vg::SeekLowerBound : vg::SeekUpperBound);
    QAbstractSpinBox::stepBy(steps);
    }
}

//-----------------------------------------------------------------------------
int vgFrameSpinBox::valueFromText(const QString& text) const
{
  QTE_D_CONST(vgFrameSpinBox);
  d->nextDirection = vg::SeekNearest;
  return QSpinBox::valueFromText(text);
}

//-----------------------------------------------------------------------------
void vgFrameSpinBox::fixup(QString& str) const
{
  QTE_D_CONST(vgFrameSpinBox);
  d->nextDirection = vg::SeekNearest;
  QSpinBox::fixup(str);
}

//-----------------------------------------------------------------------------
void vgFrameSpinBox::emitValueChangeWithDirection(int newValue)
{
  QTE_D(vgFrameSpinBox);

  if (d->nextDirection == vg::SeekUnspecified)
    {
    qWarning() << "vgFrameSpinBox: valueChanged() was emitted"
                  " but direction is not set";
    }

  emit this->valueChanged(newValue, d->nextDirection);
  d->nextDirection = vg::SeekUnspecified;
}
