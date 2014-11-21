/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
