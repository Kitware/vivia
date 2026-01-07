// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsDescriptorArchiveSourcePlugin_h
#define __vsDescriptorArchiveSourcePlugin_h

#include <QObject>

#include "vsArchiveSourcePlugin.h"

class vsDescriptorArchiveSourcePlugin : public QObject,
                                        public vsArchiveSourcePlugin
{
  Q_OBJECT
  Q_INTERFACES(vsSourceFactoryInterface)
  Q_PLUGIN_METADATA(IID "org.visgui.vsSourceFactoryInterface")

public:
  vsDescriptorArchiveSourcePlugin();
  virtual ~vsDescriptorArchiveSourcePlugin();

  virtual QString identifier() const QTE_OVERRIDE;

  virtual void registerFactoryCliOptions(qtCliOptions&) QTE_OVERRIDE;
  virtual void registerActions() QTE_OVERRIDE;

protected:
  virtual void insertActions(
    qtPrioritizedMenuProxy& videoMenu, qtPrioritizedMenuProxy& trackMenu,
    qtPrioritizedMenuProxy& descriptorMenu) QTE_OVERRIDE;

private:
  QTE_DISABLE_COPY(vsDescriptorArchiveSourcePlugin)
};

#endif
