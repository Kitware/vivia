/*=========================================================================

   Program: ParaView
   Module:    pqFileDialogEventPlayer.cxx

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

#include "pqFileDialogEventPlayer.h"

#include "pqEventDispatcher.h"
#include "pqCoreTestUtility.h"

#include <vtksys/SystemTools.hxx>

#include <QFileDialog>
#include <QApplication>
#include <QtDebug>

//-----------------------------------------------------------------------------
pqFileDialogEventPlayer::pqFileDialogEventPlayer(pqCoreTestUtility* t,
                                                 QObject* p)
  : pqWidgetEventPlayer(p), TestUtility(t)
{
}

//-----------------------------------------------------------------------------
bool pqFileDialogEventPlayer::playEvent(QObject* Object, const QString& Command,
                                        const QString& Arguments, bool& Error)
{
  // Handle playback for pqFileDialog and all its children ...
  QFileDialog* object = 0;
  for (QObject* o = Object; o; o = o->parent())
    {
    object = qobject_cast<QFileDialog*>(o);
    if (object)
      {
      break;
      }
    }
  if (!object)
    {
    return false;
    }

  QString fileString = Arguments;
  QString dataEnv = this->TestUtility->GetDataRootEnvironment();
  QString dataDirectory = this->TestUtility->GetDataRoot();

  if (!this->expandEnvToken(fileString, dataEnv, dataDirectory))
    {
    qCritical() << "You must set the " << dataEnv
                << " environment variable to play-back file selections.";
    Error = true;
    return true;
    }

  QString testTempEnv = this->TestUtility->GetTestTempRootEnvironment();
  QString testTempDirectory = this->TestUtility->GetTestTempRoot();

  if (!this->expandEnvToken(fileString, testTempEnv, testTempDirectory))
    {
    qCritical() << "You must set the " << testTempEnv
                << " environment variable to play-back file selections.";
    Error = true;
    return true;
    }

  if (Command == "filesSelected")
    {
    // For some reason the dialog must be hidden for the selectFile call to work...
    object->setVisible(false);
    object->selectFile(fileString);
    object->setConfirmOverwrite(false);

    connect(this, SIGNAL(accept()), object, SLOT(accept()));
    emit accept();

    if (object && object->result() == QDialog::Accepted)
      {
      pqEventDispatcher::processEventsAndWait(0);
      return true;
      }

    qCritical() << "Dialog couldn't accept " << fileString;
    Error = true;
    return true;
    }

  if (Command == "cancelled")
    {
    object->reject();
    return true;
    }
  if (Command == "remove")
    {
    vtksys::SystemTools::RemoveFile(qPrintable(fileString));
    return true;
    }

  qCritical() << "Unknown QFileDialog command: " << Object << " " << Command << " " << Arguments;
  Error = true;
  return true;
}

//-----------------------------------------------------------------------------
bool pqFileDialogEventPlayer::expandEnvToken(QString& path,
                                             QString token,
                                             const QString& expansion)
{
  if (path.contains(token) && expansion.isEmpty())
    {
    return false;
    }

  path.replace(token.prepend('$'),
               QDir::cleanPath(QDir::fromNativeSeparators(expansion)));
  return true;
}
