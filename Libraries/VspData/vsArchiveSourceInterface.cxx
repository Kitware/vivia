/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsSourceFactoryInterface.h"

#include <QApplication>
#include <QVariant>

#include "vsFactoryAction.h"

namespace { const char* keyActionDisplayGroup = "vsData_ActionDisplayGroup"; }

//-----------------------------------------------------------------------------
vsSourceFactoryInterface::~vsSourceFactoryInterface()
{
}

//-----------------------------------------------------------------------------
QString vsSourceFactoryInterface::settingsKey() const
{
  return QString("SourceFactory/Create%1").arg(this->identifier());
}

//-----------------------------------------------------------------------------
void vsSourceFactoryInterface::registerFactoryCliOptions(qtCliOptions& options)
{
  Q_UNUSED(options);
}

//-----------------------------------------------------------------------------
QList<vsPendingFactoryAction> vsSourceFactoryInterface::parseFactoryArguments(
  const qtCliArgs& args)
{
  Q_UNUSED(args);
  return QList<vsPendingFactoryAction>();
}

//-----------------------------------------------------------------------------
void vsSourceFactoryInterface::registerActions()
{
}

//-----------------------------------------------------------------------------
void vsSourceFactoryInterface::createActions(
  QObject* parent, QSignalMapper* signalMapper,
  qtPrioritizedMenuProxy& videoMenu,
  qtPrioritizedMenuProxy& trackMenu,
  qtPrioritizedMenuProxy& descriptorMenu)
{
  Q_UNUSED(parent);
  Q_UNUSED(signalMapper);
  Q_UNUSED(videoMenu);
  Q_UNUSED(trackMenu);
  Q_UNUSED(descriptorMenu);
}

//-----------------------------------------------------------------------------
vsSourceFactory* vsSourceFactoryInterface::createFactory()
{
  return 0;
}

//-----------------------------------------------------------------------------
QString vsSourceFactoryInterface::actionDisplayGroup()
{
  QVariant value = qApp->property(keyActionDisplayGroup);
  return (value.isNull() ? QString("Sources") : value.toString());
}

//-----------------------------------------------------------------------------
void vsSourceFactoryInterface::setActionDisplayGroup(const QString& value)
{
  qApp->setProperty(keyActionDisplayGroup, value);
}
