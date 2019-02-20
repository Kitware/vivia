/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsFakeStreamSourceFactoryPlugin_h
#define __vsFakeStreamSourceFactoryPlugin_h

#include <QObject>

#include <vsSourceFactoryPlugin.h>

class vsFakeStreamSourceFactoryPlugin : public QObject,
                                        public vsSourceFactoryPlugin
{
  Q_OBJECT
  Q_INTERFACES(vsSourceFactoryInterface)
  Q_PLUGIN_METADATA(IID "org.visgui.vsSourceFactoryInterface")

public:
  vsFakeStreamSourceFactoryPlugin();
  virtual ~vsFakeStreamSourceFactoryPlugin();

  virtual QString identifier() const QTE_OVERRIDE;

  virtual void registerActions() QTE_OVERRIDE;

  virtual vsSourceFactory* createFactory() QTE_OVERRIDE;

protected:
  virtual void insertActions(
    qtPrioritizedMenuProxy& videoMenu, qtPrioritizedMenuProxy& trackMenu,
    qtPrioritizedMenuProxy& descriptorMenu) QTE_OVERRIDE;

private:
  QTE_DISABLE_COPY(vsFakeStreamSourceFactoryPlugin)
};

#endif
