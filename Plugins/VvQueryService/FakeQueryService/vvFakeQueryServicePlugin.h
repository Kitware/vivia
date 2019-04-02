/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
