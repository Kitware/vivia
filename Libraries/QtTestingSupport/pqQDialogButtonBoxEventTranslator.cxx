/*=========================================================================

   Program: ParaView
   Module:    pqQDialogButtonBoxEventTranslator.cxx

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

#include "pqQDialogButtonBoxEventTranslator.h"

#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QEvent>
#include <QPushButton>

//-----------------------------------------------------------------------------
static int ButtonStandardButtonId(QAbstractButton* Button, QDialogButtonBox* Box)
{
  QPushButton* const button = qobject_cast<QPushButton*>(Button);
  if (button)
    {
    if (Box->button(QDialogButtonBox::Ok) == button)
      {
      return QDialogButtonBox::Ok;
      }
    if (Box->button(QDialogButtonBox::Open) == button)
      {
      return QDialogButtonBox::Open;
      }
    if (Box->button(QDialogButtonBox::Save) == button)
      {
      return QDialogButtonBox::Save;
      }
    if (Box->button(QDialogButtonBox::Cancel) == button)
      {
      return QDialogButtonBox::Cancel;
      }
    if (Box->button(QDialogButtonBox::Close) == button)
      {
      return QDialogButtonBox::Close;
      }
    if (Box->button(QDialogButtonBox::Discard) == button)
      {
      return QDialogButtonBox::Discard;
      }
    if (Box->button(QDialogButtonBox::Apply) == button)
      {
      return QDialogButtonBox::Apply;
      }
    if (Box->button(QDialogButtonBox::Reset) == button)
      {
      return QDialogButtonBox::Reset;
      }
    if (Box->button(QDialogButtonBox::RestoreDefaults) == button)
      {
      return QDialogButtonBox::RestoreDefaults;
      }
    if (Box->button(QDialogButtonBox::Help) == button)
      {
      return QDialogButtonBox::Help;
      }
    if (Box->button(QDialogButtonBox::SaveAll) == button)
      {
      return QDialogButtonBox::SaveAll;
      }
    if (Box->button(QDialogButtonBox::Yes) == button)
      {
      return QDialogButtonBox::Yes;
      }
    if (Box->button(QDialogButtonBox::YesToAll) == button)
      {
      return QDialogButtonBox::YesToAll;
      }
    if (Box->button(QDialogButtonBox::No) == button)
      {
      return QDialogButtonBox::No;
      }
    if (Box->button(QDialogButtonBox::NoToAll) == button)
      {
      return QDialogButtonBox::NoToAll;
      }
    if (Box->button(QDialogButtonBox::Abort) == button)
      {
      return QDialogButtonBox::Abort;
      }
    if (Box->button(QDialogButtonBox::Retry) == button)
      {
      return QDialogButtonBox::Retry;
      }
    if (Box->button(QDialogButtonBox::Ignore) == button)
      {
      return QDialogButtonBox::Ignore;
      }
    }
  return QDialogButtonBox::NoButton;
}

//-----------------------------------------------------------------------------
pqQDialogButtonBoxEventTranslator::pqQDialogButtonBoxEventTranslator(QObject* p)
  : pqWidgetEventTranslator(p),
    CurrentObject(0)
{
}

//-----------------------------------------------------------------------------
bool pqQDialogButtonBoxEventTranslator::translateEvent(QObject* Object, QEvent* Event, bool& /*Error*/)
{
  QAbstractButton* const object = qobject_cast<QAbstractButton*>(Object);
  if (!object || !qobject_cast<QDialogButtonBox*>(object->parent()))
    {
    return false;
    }

  switch (Event->type())
    {
    case QEvent::Enter:
      this->CurrentObject = object;
      connect(object, SIGNAL(pressed()),
              this, SLOT(onActivate()));
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
void pqQDialogButtonBoxEventTranslator::onActivate()
{
  QDialogButtonBox* const parent =
    qobject_cast<QDialogButtonBox*>(this->CurrentObject->parent());
  // Determine if this is a standard button
  int id = ButtonStandardButtonId(this->CurrentObject, parent);
  if (id)
    {
    emit recordEvent(parent, "activate", QString("0x%1").arg(id, 0, 16));
    return;
    }

  // Not a standard button; emit a regular "activate"
  emit recordEvent(this->CurrentObject, "activate", QString());
}
