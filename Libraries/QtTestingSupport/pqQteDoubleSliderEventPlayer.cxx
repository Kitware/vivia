/*=========================================================================

   Program: ParaView
   Module:    pqQteDoubleSliderEventPlayer.cxx

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

#include "pqQteDoubleSliderEventPlayer.h"

#include "pqEventDispatcher.h"

#include <QApplication>
#include <QtDebug>

#include <qtDoubleSlider.h>

//-----------------------------------------------------------------------------
pqQteDoubleSliderEventPlayer::pqQteDoubleSliderEventPlayer(QObject* p)
  : pqWidgetEventPlayer(p)
{
}

//-----------------------------------------------------------------------------
bool pqQteDoubleSliderEventPlayer::playEvent(QObject* Object, const QString& Command, const QString& Arguments, bool& Error)
{
  double value = 0.0;

  if (Command == "set_double")
    {
    value = Arguments.toDouble();
    }
  else if (Command == "set_xdouble")
    {
    QByteArray data = QByteArray::fromHex(Arguments.toAscii());
    QDataStream stream(data);
    stream >> value;
    }
  else
    {
    return false;
    }

  if (qtDoubleSlider* const object = qobject_cast<qtDoubleSlider*>(Object))
    {
    object->setValue(value);
    return true;
    }

  if (Command == "set_double")
    {
    // let pqAbstractDoubleEventPlayer handle it if possible.
    return false;
    }

  qCritical() << "calling set_xdouble on unhandled type" << Object;
  Error = true;
  return true;
}
