/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsSourceFactoryPlugin.h"

#include <QDebug>
#include <QHash>
#include <QSignalMapper>

#include <qtActionFactory.h>
#include <qtActionManager.h>

//-----------------------------------------------------------------------------
class vsSourceFactoryPluginPrivate
{
public:
  vsSourceFactoryPluginPrivate() : ActionParent(0), SignalMapper(0) {}

  QHash<int, QAction*> Actions;
  QHash<int, qtActionFactory*> ActionFactories;

  QObject* ActionParent;
  QSignalMapper* SignalMapper;
};

QTE_IMPLEMENT_D_FUNC(vsSourceFactoryPlugin)

//-----------------------------------------------------------------------------
vsSourceFactoryPlugin::vsSourceFactoryPlugin() :
  d_ptr(new vsSourceFactoryPluginPrivate)
{
}

//-----------------------------------------------------------------------------
vsSourceFactoryPlugin::~vsSourceFactoryPlugin()
{
}

//-----------------------------------------------------------------------------
qtActionFactory* vsSourceFactoryPlugin::registerAction(
  int key, const QString& displayText, const QString& iconName,
  const QString& shortcutPortableString, const QString& toolTipText)
{
  QTE_D(vsSourceFactoryPlugin);

  // Check that we haven't already registered an action with this key
  if (d->ActionFactories.contains(key))
    {
    qWarning() << "vsSourceFactoryPlugin::registerAction:"
                  " ignoring request to register action"
                  " with already registered key" << key;
    return 0;
    }

  const QString& displayGroup = vsSourceFactoryInterface::actionDisplayGroup();
  const QKeySequence shortcut(shortcutPortableString,
                              QKeySequence::PortableText);

  // Create the action factory
  qtActionFactory* actionFactory =
    qtAm->registerAction(this->settingsKey(), displayText, displayGroup,
                         iconName, shortcut);
  actionFactory->setToolTip(toolTipText);

  // Insert into private map and return the new factory
  d->ActionFactories.insert(key, actionFactory);
  return actionFactory;
}

//-----------------------------------------------------------------------------
void vsSourceFactoryPlugin::createActions(
  QObject* parent, QSignalMapper* signalMapper,
  qtPrioritizedMenuProxy& videoMenu,
  qtPrioritizedMenuProxy& trackMenu,
  qtPrioritizedMenuProxy& descriptorMenu)
{
  QTE_D(vsSourceFactoryPlugin);

  d->ActionParent = parent;
  d->SignalMapper = signalMapper;

  this->insertActions(videoMenu, trackMenu, descriptorMenu);
}

//-----------------------------------------------------------------------------
QAction* vsSourceFactoryPlugin::action(int key)
{
  QTE_D(vsSourceFactoryPlugin);

  // Has the action been created yet?
  if (!d->Actions.contains(key))
    {
    if (!d->ActionFactories.contains(key))
      {
      // Oops; looks like this action was not registered
      return 0;
      }

    // Create action, and connect to signal mapper
    QAction* action = d->ActionFactories[key]->createAction(d->ActionParent);
    d->SignalMapper->setMapping(action, this->identifier());
    d->SignalMapper->connect(action, SIGNAL(triggered()), SLOT(map()));
    d->Actions.insert(key, action);

    // Return new action (saves an extra has look-up)
    return action;
    }

  // Return existing action
  return d->Actions[key];
}

//-----------------------------------------------------------------------------
void vsSourceFactoryPlugin::insertActions(
  qtPrioritizedMenuProxy&, qtPrioritizedMenuProxy&, qtPrioritizedMenuProxy&)
{
}
