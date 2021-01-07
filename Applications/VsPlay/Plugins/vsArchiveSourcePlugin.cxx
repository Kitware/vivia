// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsArchiveSourcePlugin.h"

#include <qtCliArgs.h>

#include <vsFactoryAction.h>
#include <vsSourceService.h>

#include "vsArchiveSourceFactory.h"

//-----------------------------------------------------------------------------
class vsArchiveSourcePluginPrivate
{
public:
  vsArchiveSourcePluginPrivate(vsArchiveSourceType type) :
    Type(type), BackendsChecked(false) {}

  const vsArchiveSourceType Type;
  bool BackendsAvailable;
  bool BackendsChecked;
  QList<QString> CliOptions;
};

QTE_IMPLEMENT_D_FUNC(vsArchiveSourcePlugin)

//-----------------------------------------------------------------------------
vsArchiveSourcePlugin::vsArchiveSourcePlugin(vsArchiveSourceType type) :
  d_ptr(new vsArchiveSourcePluginPrivate(type))
{
}

//-----------------------------------------------------------------------------
vsArchiveSourcePlugin::~vsArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
bool vsArchiveSourcePlugin::areBackendsAvailable()
{
  QTE_D(vsArchiveSourcePlugin);
  if (!d->BackendsChecked)
    {
    d->BackendsAvailable =
      !vsSourceService::archivePluginInfo(d->Type).isEmpty();
    d->BackendsChecked = true;
    }
  return d->BackendsAvailable;
}

//-----------------------------------------------------------------------------
void vsArchiveSourcePlugin::addArchiveOption(
  qtCliOptions& options, const QString& type, const QString& shortName)
{
  QTE_D(vsArchiveSourcePlugin);

  options.add(type + "-file <file>",
              QString("Load %1 from archive 'file'").arg(type))
         .add(shortName, qtCliOption::Short);

  d->CliOptions.append(type + "-file");
}

//-----------------------------------------------------------------------------
QList<vsPendingFactoryAction>
vsArchiveSourcePlugin::parseFactoryArguments(const qtCliArgs& args)
{
  QTE_D_CONST(vsArchiveSourcePlugin);

  QList<vsPendingFactoryAction> requestedActions;
  foreach (const QString& option, d->CliOptions)
    {
    foreach (const QString& uri, args.values(option))
      {
      if (!uri.isEmpty())
        {
        vsPendingFactoryAction action;
        action.FactoryIdentifier = this->identifier();
        action.SourceUri = QUrl::fromLocalFile(uri);
        requestedActions.append(action);
        }
      }
    }
  return requestedActions;
}

//-----------------------------------------------------------------------------
vsSourceFactory* vsArchiveSourcePlugin::createFactory()
{
  QTE_D_CONST(vsArchiveSourcePlugin);
  return new vsArchiveSourceFactory(d->Type);
}
