// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsApplication.h"

#include <vgPluginLoader.h>

#include <vsSourceService.h>

#include "vsCore.h"
#include "vsMainWindow.h"
#include "vsUiExtensionInterface.h"

QTE_IMPLEMENT_D_FUNC(vsApplication)

//-----------------------------------------------------------------------------
class vsApplicationPrivate
{
public:
  vsCore* Core;
  QList<vsUiExtensionInterface*> UiExtensions;
  QPointer<vsMainWindow> FirstView;
  bool FirstViewAssigned;
};

//-----------------------------------------------------------------------------
vsApplication::vsApplication(int& argc, char** argv)
  : vgApplication(argc, argv), d_ptr(new vsApplicationPrivate)
{
  QTE_D(vsApplication);

  d->FirstViewAssigned = false;
}

//-----------------------------------------------------------------------------
vsApplication::~vsApplication()
{
}

//-----------------------------------------------------------------------------
vsApplication* vsApplication::instance()
{
  return qobject_cast<vsApplication*>(QCoreApplication::instance());
}

//-----------------------------------------------------------------------------
QList<vsUiExtensionInterface*> vsApplication::uiExtensions()
{
  return vsApplication::instance()->d_func()->UiExtensions;
}

//-----------------------------------------------------------------------------
vsMainWindow* vsApplication::firstView() const
{
  QTE_D_CONST(vsApplication);
  return d->FirstView.data();
}

//-----------------------------------------------------------------------------
vsMainWindow* vsApplication::newView(vsMainWindow* invokingView)
{
  QTE_D(vsApplication);

  // Create new view
  vsMainWindow* view = new vsMainWindow(d->Core, invokingView);
  view->show();

  // Mark window for clean-up on close
  view->setAttribute(Qt::WA_DeleteOnClose, true);

  // Ensure view is closed when we shut down (won't happen if we are shut down
  // via QCoreApplication::exit!)
  connect(this, SIGNAL(aboutToQuit()), view, SLOT(close()));

  // Assign as the first view, if not assigned
  if (!d->FirstViewAssigned)
    {
    d->FirstView = view;
    d->FirstViewAssigned = true;
    }

  return view;
}

//-----------------------------------------------------------------------------
void vsApplication::connectSource(const QString& identifier, const QUrl& uri)
{
  QTE_D(vsApplication);

  vsSourceFactoryPtr factory = vsSourceService::createFactory(identifier);
  if (factory && factory->initialize(uri))
    d->Core->addSources(factory);
}

//-----------------------------------------------------------------------------
void vsApplication::initialize(const qtCliArgs& args)
{
  QTE_D(vsApplication);
  d->Core = new vsCore(this);

  // Load UI extensions
  d->UiExtensions = vgPluginLoader::pluginInterfaces<vsUiExtensionInterface>();
  foreach (vsUiExtensionInterface* extension, d->UiExtensions)
    {
    // Register and initialize extension
    extension->initialize(d->Core);
    extension->parseExtensionArguments(args);
    }
}

//-----------------------------------------------------------------------------
void vsApplication::viewCloseEvent(vsMainWindow*, QCloseEvent*)
{
  int viewCount = 0;
  foreach (QWidget* tlw, qApp->topLevelWidgets())
    {
    if (qobject_cast<vsMainWindow*>(tlw))
      ++viewCount;
    }

  if (viewCount == 1)
    emit this->lastViewClosed();
}
