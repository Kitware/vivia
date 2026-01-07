// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsPowerPointGeneratorPlugin_h
#define __vsPowerPointGeneratorPlugin_h

#include <QObject>

#include <qtGlobal.h>

#include <vsUiExtensionInterface.h>

class vsPowerPointGeneratorPlugin : public QObject,
                                    public vsUiExtensionInterface
{
  Q_OBJECT
  Q_INTERFACES(vsUiExtensionInterface)
  Q_PLUGIN_METADATA(IID "org.visgui.vsUiExtensionInterface")

public:
  vsPowerPointGeneratorPlugin();
  virtual ~vsPowerPointGeneratorPlugin();

  virtual void createInterface(vsMainWindow*, vsScene*) QTE_OVERRIDE;

private:
  QTE_DISABLE_COPY(vsPowerPointGeneratorPlugin)
};

#endif
