// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsRandomAlertDescriptorPlugin_h
#define __vsRandomAlertDescriptorPlugin_h

#include <QObject>

#include <vsSourceFactoryPlugin.h>

class vsRandomAlertDescriptorPlugin : public QObject,
                                      public vsSourceFactoryPlugin
{
  Q_OBJECT
  Q_INTERFACES(vsSourceFactoryInterface)
  Q_PLUGIN_METADATA(IID "org.visgui.vsSourceFactoryInterface")

public:
  vsRandomAlertDescriptorPlugin();
  virtual ~vsRandomAlertDescriptorPlugin();

  virtual QString identifier() const QTE_OVERRIDE;

  virtual void registerActions() QTE_OVERRIDE;

  virtual vsSourceFactory* createFactory() QTE_OVERRIDE;

protected:
  virtual void insertActions(
    qtPrioritizedMenuProxy& videoMenu, qtPrioritizedMenuProxy& trackMenu,
    qtPrioritizedMenuProxy& descriptorMenu) QTE_OVERRIDE;

private:
  QTE_DISABLE_COPY(vsRandomAlertDescriptorPlugin)
};

#endif
