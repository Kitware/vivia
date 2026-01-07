// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
