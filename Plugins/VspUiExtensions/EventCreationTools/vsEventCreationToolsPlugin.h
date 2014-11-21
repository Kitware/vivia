/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsEventCreationToolsPlugin_h
#define __vsEventCreationToolsPlugin_h

#include <QObject>

#include <qtGlobal.h>

#include <vsUiExtensionInterface.h>

class vsEventCreationToolsPlugin : public QObject,
                                   public vsUiExtensionInterface
{
  Q_OBJECT
  Q_INTERFACES(vsUiExtensionInterface)

public:
  vsEventCreationToolsPlugin();
  virtual ~vsEventCreationToolsPlugin();

  virtual void createInterface(vsMainWindow*, vsScene*) QTE_OVERRIDE;

private:
  QTE_DISABLE_COPY(vsEventCreationToolsPlugin)
};

#endif
