/*=========================================================================

   Program: ParaView
   Module:    pqQteDoubleSliderEventTranslator.cxx

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#include "pqQteDoubleSliderEventTranslator.h"

#include <QEvent>

#include <qtDoubleSlider.h>

//-----------------------------------------------------------------------------
pqQteDoubleSliderEventTranslator::pqQteDoubleSliderEventTranslator(QObject* p)
  : pqWidgetEventTranslator(p),
    CurrentObject(0)
{
}

//-----------------------------------------------------------------------------
bool pqQteDoubleSliderEventTranslator::translateEvent(QObject* Object, QEvent* Event, bool& /*Error*/)
{
  qtDoubleSlider* const object = qobject_cast<qtDoubleSlider*>(Object);
  if (!object)
    {
    return false;
    }

  switch (Event->type())
    {
    case QEvent::Enter:
      this->CurrentObject = Object;
      connect(object, SIGNAL(valueChanged(double)),
              this, SLOT(onValueChanged(double)));
      break;
    case QEvent::Leave:
      disconnect(Object, 0, this, 0);
      this->CurrentObject = 0;
      break;
    default:
      break;
    }

  return true;
}

//-----------------------------------------------------------------------------
void pqQteDoubleSliderEventTranslator::onValueChanged(double Value)
{
  QString value = QString::number(Value);
  if (qFuzzyCompare(value.toDouble(), Value))
    {
    emit recordEvent(this->CurrentObject, "set_double", value);
    }
  else
    {
    // QString::number() is not precise enough... use a raw representation of
    // the value.
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << Value;
    emit recordEvent(this->CurrentObject, "set_xdouble",
                     QString::fromAscii(data.toHex()));
    }
}
