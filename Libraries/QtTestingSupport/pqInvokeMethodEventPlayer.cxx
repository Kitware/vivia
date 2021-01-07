// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "pqInvokeMethodEventPlayer.h"

#include <QDebug>
#include <QStringList>

#include <pqEventDispatcher.h>

namespace
{

struct IgnoreBlocking
{
  IgnoreBlocking()
    {
    pqEventDispatcher::setIgnoreBlocking(true);
    }
  ~IgnoreBlocking()
    {
    pqEventDispatcher::setIgnoreBlocking(false);
    }
};

}

//-----------------------------------------------------------------------------
pqInvokeMethodEventPlayer::pqInvokeMethodEventPlayer(QObject* p)
  : pqWidgetEventPlayer(p)
{
}

//-----------------------------------------------------------------------------
bool pqInvokeMethodEventPlayer::playEvent(
  QObject* Object, const QString& Command,
  const QString& Arguments, bool& Error)
{
  if (Command == "invokeMethod")
    {
    // Get split argument list and prepare argument object list
    QStringList strargs = Arguments.split(",");
    QGenericArgument qargs[10];

    // Extract and convert arguments
    for (int n = 0, i = 1, k = strargs.count(); i < k && n < 10; ++n, ++i)
      {
      // Get argument type and value strings
      const QString type = strargs[i];
      if (++i >= k)
        {
        qCritical() << "wrong number of arguments to invokeMethod";
        Error = true;
        return false;
        }
      const QString value = strargs[i];

      // Parse argument value
      if (type == "string")
        {
        qargs[n] = Q_ARG(QString, value);
        }
      else if (type == "int")
        {
        qargs[n] = Q_ARG(int, value.toInt());
        }
      else if (type == "long")
        {
        qargs[n] = Q_ARG(long, value.toLong());
        }
      else if (type == "long long" || type == "longlong")
        {
        qargs[n] = Q_ARG(long long, value.toLongLong());
        }
      else if (type == "float")
        {
        qargs[n] = Q_ARG(float, value.toFloat());
        }
      else if (type == "double")
        {
        qargs[n] = Q_ARG(double, value.toDouble());
        }
      else
        {
        qCritical() << "invokeMethod given unhandled argument type" << type;
        Error = true;
        return false;
        }
      }

    // Prevent QtTesting from interleaving playback of other events with this
    // one. If the invokeMethod call happens to create a modal dialog this will
    // need to be revisited.
    IgnoreBlocking ib;

    // Invoke method
    if (!QMetaObject::invokeMethod(
           Object, qPrintable(strargs[0]), Qt::AutoConnection,
           qargs[0], qargs[1], qargs[2], qargs[3], qargs[4],
           qargs[5], qargs[6], qargs[7], qargs[8], qargs[9]))
      {
      qCritical() << "calling invokeMethod" << strargs
                  << "on object" << Object << "failed";
      Error = true;
      return false;
      }
    return true;
    }

  return false;
}
