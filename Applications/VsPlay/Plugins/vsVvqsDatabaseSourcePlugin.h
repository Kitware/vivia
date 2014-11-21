/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsVvqsDatabaseSourcePlugin_h
#define __vsVvqsDatabaseSourcePlugin_h

#include <QObject>

#include <vsSourceFactoryPlugin.h>

class vsVvqsDatabaseSourcePluginPrivate;

class vsVvqsDatabaseSourcePlugin : public QObject, public vsSourceFactoryPlugin
{
  Q_OBJECT
  Q_INTERFACES(vsSourceFactoryInterface)

public:
  vsVvqsDatabaseSourcePlugin();
  virtual ~vsVvqsDatabaseSourcePlugin();

  virtual QString identifier() const QTE_OVERRIDE;

  virtual void registerFactoryCliOptions(qtCliOptions&) QTE_OVERRIDE;
  virtual QList<vsPendingFactoryAction> parseFactoryArguments(
    const qtCliArgs&) QTE_OVERRIDE;

  virtual void registerActions() QTE_OVERRIDE;

  virtual vsSourceFactory* createFactory() QTE_OVERRIDE;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsVvqsDatabaseSourcePlugin)

  virtual void insertActions(
    qtPrioritizedMenuProxy& videoMenu, qtPrioritizedMenuProxy& trackMenu,
    qtPrioritizedMenuProxy& descriptorMenu) QTE_OVERRIDE;

private:
  QTE_DECLARE_PRIVATE(vsVvqsDatabaseSourcePlugin)
  QTE_DISABLE_COPY(vsVvqsDatabaseSourcePlugin)
};

#endif
