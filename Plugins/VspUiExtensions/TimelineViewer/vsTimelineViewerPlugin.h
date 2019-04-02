/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
