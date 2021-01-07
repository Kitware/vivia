// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsReportGeneratorPlugin_h
#define __vsReportGeneratorPlugin_h

#include <QObject>

#include <qtGlobal.h>

#include <vsUiExtensionInterface.h>

class vsReportGeneratorPlugin : public QObject, public vsUiExtensionInterface
{
  Q_OBJECT
  Q_INTERFACES(vsUiExtensionInterface)
  Q_PLUGIN_METADATA(IID "org.visgui.vsUiExtensionInterface")

public:
  vsReportGeneratorPlugin();
  virtual ~vsReportGeneratorPlugin();

  virtual void createInterface(vsMainWindow*, vsScene*) QTE_OVERRIDE;

private:
  QTE_DISABLE_COPY(vsReportGeneratorPlugin)
};

#endif
