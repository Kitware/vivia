// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsRulerPlugin_h
#define __vsRulerPlugin_h

#include <QObject>

#include <qtGlobal.h>

#include <vsUiExtensionInterface.h>

class vsRulerPlugin : public QObject, public vsUiExtensionInterface
{
  Q_OBJECT
  Q_INTERFACES(vsUiExtensionInterface)
  Q_PLUGIN_METADATA(IID "org.visgui.vsUiExtensionInterface")

public:
  vsRulerPlugin();
  virtual ~vsRulerPlugin();

  virtual void createInterface(vsMainWindow*, vsScene*) QTE_OVERRIDE;

private:
  QTE_DISABLE_COPY(vsRulerPlugin)
};

#endif
