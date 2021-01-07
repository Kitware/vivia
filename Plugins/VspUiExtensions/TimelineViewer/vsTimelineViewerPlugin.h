// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsTimelineViewerPlugin_h
#define __vsTimelineViewerPlugin_h

#include <vsUiExtensionInterface.h>

#include <qtGlobal.h>

#include <QObject>

class vsTimelineViewerPlugin : public QObject, public vsUiExtensionInterface
{
  Q_OBJECT
  Q_INTERFACES(vsUiExtensionInterface)
  Q_PLUGIN_METADATA(IID "org.visgui.vsUiExtensionInterface")

public:
  vsTimelineViewerPlugin();
  virtual ~vsTimelineViewerPlugin();

  // vsUiExtensionInterface interface
  virtual void initialize(vsCore* core) QTE_OVERRIDE;
  virtual void createInterface(vsMainWindow*, vsScene*) QTE_OVERRIDE;

private:
  QTE_DISABLE_COPY(vsTimelineViewerPlugin)
};

#endif
