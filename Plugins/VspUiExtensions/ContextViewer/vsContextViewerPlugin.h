/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsContextViewerPlugin_h
#define __vsContextViewerPlugin_h

#include <vsUiExtensionInterface.h>

#include <qtGlobal.h>

#include <QObject>

class vsContextViewerPlugin : public QObject, public vsUiExtensionInterface
{
  Q_OBJECT
  Q_INTERFACES(vsUiExtensionInterface)

public:
  vsContextViewerPlugin();
  virtual ~vsContextViewerPlugin();

  // vsUiExtensionInterface interface
  virtual void initialize(vsCore* core) QTE_OVERRIDE;
  virtual void createInterface(vsMainWindow*, vsScene*) QTE_OVERRIDE;
  virtual void registerExtensionCliOptions(qtCliOptions&) QTE_OVERRIDE;
  virtual void parseExtensionArguments(const qtCliArgs&) QTE_OVERRIDE;

private:
  QTE_DISABLE_COPY(vsContextViewerPlugin)

  QString ContextUri;
};

#endif
