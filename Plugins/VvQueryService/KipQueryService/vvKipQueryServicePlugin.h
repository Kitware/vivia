/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvKipQueryServicePlugin_h
#define __vvKipQueryServicePlugin_h

#include <vvQueryServiceInterface.h>

class vvKipQueryServicePlugin : public QObject, public vvQueryServiceInterface
{
  Q_OBJECT
  Q_INTERFACES(vvQueryServiceInterface)

public:
  vvKipQueryServicePlugin();
  ~vvKipQueryServicePlugin();

  virtual QStringList supportedSchemes() const;
  virtual void registerChoosers(vvQueryServerDialog*);
  virtual vvQuerySession* createSession(const QUrl&);
};

#endif
