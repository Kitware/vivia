// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef _pqInvokeMethodEventPlayer_h
#define _pqInvokeMethodEventPlayer_h

#include <pqWidgetEventPlayer.h>

#include <vgExport.h>

/**
Concrete implementation of pqWidgetEventPlayer that handles playback of the
invokeMethod command.

\sa pqEventPlayer
*/
class QT_TESTINGSUPPORT_EXPORT pqInvokeMethodEventPlayer :
  public pqWidgetEventPlayer
{
  Q_OBJECT

public:
  pqInvokeMethodEventPlayer(QObject* p = 0);

  virtual bool playEvent(QObject* Object, const QString& Command,
                         const QString& Arguments, bool& Error);

private:
  Q_DISABLE_COPY(pqInvokeMethodEventPlayer)
};

#endif
