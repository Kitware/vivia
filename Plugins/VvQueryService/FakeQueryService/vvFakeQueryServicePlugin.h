// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvFakeQueryServicePlugin_h
#define __vvFakeQueryServicePlugin_h

#include <vvQueryServiceInterface.h>

class vvFakeQueryServicePlugin : public QObject, public vvQueryServiceInterface
{
  Q_OBJECT
  Q_INTERFACES(vvQueryServiceInterface)
  Q_PLUGIN_METADATA(IID "org.visgui.vvQueryServiceInterface")

public:
  vvFakeQueryServicePlugin();
  ~vvFakeQueryServicePlugin();

  virtual QStringList supportedSchemes() const;
  virtual void registerChoosers(vvQueryServerDialog*);
  virtual vvQuerySession* createSession(const QUrl&);
};

#endif
