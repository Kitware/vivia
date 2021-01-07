// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsVvqsDatabaseSourcePlugin.h"
#include "moc_vsVvqsDatabaseSourcePlugin.cpp"

#include <QSignalMapper>
#include <QtPlugin>

#include <qtActionFactory.h>
#include <qtActionManager.h>
#include <qtCliArgs.h>

#include <vvQueryService.h>

#include <vsFactoryAction.h>

#include "vsVvqsDatabaseFactory.h"

namespace { static const int keyLoadAction = 0; }

//-----------------------------------------------------------------------------
class vsVvqsDatabaseSourcePluginPrivate
{
public:
  vsVvqsDatabaseSourcePluginPrivate() : BackendsChecked(false) {}

  bool areBackendsAvailable();

  bool BackendsAvailable;
  bool BackendsChecked;
};

QTE_IMPLEMENT_D_FUNC(vsVvqsDatabaseSourcePlugin)

//-----------------------------------------------------------------------------
bool vsVvqsDatabaseSourcePluginPrivate::areBackendsAvailable()
{
  if (!this->BackendsChecked)
    {
    this->BackendsAvailable = !vvQueryService::supportedSchemes().isEmpty();
    this->BackendsChecked = true;
    }
  return this->BackendsAvailable;
}

//-----------------------------------------------------------------------------
vsVvqsDatabaseSourcePlugin::vsVvqsDatabaseSourcePlugin() :
  d_ptr(new vsVvqsDatabaseSourcePluginPrivate)
{
}

//-----------------------------------------------------------------------------
vsVvqsDatabaseSourcePlugin::~vsVvqsDatabaseSourcePlugin()
{
}

//-----------------------------------------------------------------------------
QString vsVvqsDatabaseSourcePlugin::identifier() const
{
  return "VvqsDatabase";
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseSourcePlugin::registerFactoryCliOptions(
  qtCliOptions& options)
{
  QTE_D(vsVvqsDatabaseSourcePlugin);

  if (d->areBackendsAvailable())
    {
    options.add("query-database <uri>",
                "Issue database query defined by 'uri' to load archived data")
           .add("qd", qtCliOption::Short);
    }
}

//-----------------------------------------------------------------------------
QList<vsPendingFactoryAction>
vsVvqsDatabaseSourcePlugin::parseFactoryArguments(const qtCliArgs& args)
{
  QTE_D(vsVvqsDatabaseSourcePlugin);

  QList<vsPendingFactoryAction> requestedActions;
  if (d->areBackendsAvailable())
    {
    foreach (const QString& uri, args.values("query-database"))
      {
      if (!uri.isEmpty())
        {
        vsPendingFactoryAction action;
        action.FactoryIdentifier = this->identifier();
        action.SourceUri = QUrl::fromUserInput(uri);
        requestedActions.append(action);
        }
      }
    }
  return requestedActions;
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseSourcePlugin::registerActions()
{
  QTE_D(vsVvqsDatabaseSourcePlugin);

  if (d->areBackendsAvailable())
    {
    this->registerAction(
      keyLoadAction, "Load from &Database", "load-database", "Ctrl+O, B",
      "Load tracks and descriptors from a database");
    }
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseSourcePlugin::insertActions(
  qtPrioritizedMenuProxy& videoMenu, qtPrioritizedMenuProxy& trackMenu,
  qtPrioritizedMenuProxy& descriptorMenu)
{
  QTE_D(vsVvqsDatabaseSourcePlugin);

  if (d->areBackendsAvailable())
    {
    Q_UNUSED(videoMenu);
    trackMenu.insertAction(this->action(keyLoadAction), 200);
    descriptorMenu.insertAction(this->action(keyLoadAction), 200);
    }
}

//-----------------------------------------------------------------------------
vsSourceFactory* vsVvqsDatabaseSourcePlugin::createFactory()
{
  return new vsVvqsDatabaseFactory;
}
