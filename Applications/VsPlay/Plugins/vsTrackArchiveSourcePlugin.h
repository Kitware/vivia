/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackArchiveSourcePlugin_h
#define __vsTrackArchiveSourcePlugin_h

#include <QObject>

#include "vsArchiveSourcePlugin.h"

class vsTrackArchiveSourcePlugin : public QObject, public vsArchiveSourcePlugin
{
  Q_OBJECT
  Q_INTERFACES(vsSourceFactoryInterface)

public:
  vsTrackArchiveSourcePlugin();
  virtual ~vsTrackArchiveSourcePlugin();

  virtual QString identifier() const QTE_OVERRIDE;

  virtual void registerFactoryCliOptions(qtCliOptions&) QTE_OVERRIDE;
  virtual void registerActions() QTE_OVERRIDE;

protected:
  virtual void insertActions(
    qtPrioritizedMenuProxy& videoMenu, qtPrioritizedMenuProxy& trackMenu,
    qtPrioritizedMenuProxy& descriptorMenu) QTE_OVERRIDE;

private:
  QTE_DISABLE_COPY(vsTrackArchiveSourcePlugin)
};

#endif
