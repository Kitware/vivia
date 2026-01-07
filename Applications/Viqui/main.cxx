// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <QtCore>
#include <QList>
#include <QMainWindow>
#include <QScopedPointer>

#ifndef _WIN32
#include <signal.h>
#include <unistd.h>
#endif

#include <qtCliArgs.h>
#include <qtMap.h>
#include <qtStlUtil.h>

#include <vgCheckArg.h>

#include <vgApplication.h>

#include <vvQuery.h>
#include <vvQueryResult.h>
#include <vvQueryService.h>
#include <vvQuerySession.h>

#include <vital/plugin_loader/plugin_manager.h>

#ifdef ENABLE_QTTESTING
#include <pqCoreTestUtility.h>
#endif

#include "vqApplication.h"
#include "vqPredefinedQueryCache.h"
#include "vqVersion.h"

#ifndef _WIN32
//-----------------------------------------------------------------------------
// Signal handler for graceful shutdown on Ctrl+C
static void signalHandler(int signum)
{
  Q_UNUSED(signum);
  // Request application quit - this is safe to call from a signal handler
  // as it just posts an event to the event queue
  if (QCoreApplication::instance())
    {
    QCoreApplication::quit();
    }
}

//-----------------------------------------------------------------------------
static void setupSignalHandlers()
{
  struct sigaction sa;
  sa.sa_handler = signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sigaction(SIGINT, &sa, nullptr);
  sigaction(SIGTERM, &sa, nullptr);
}
#endif

namespace // anonymous
{

//-----------------------------------------------------------------------------
void parseConfigFile(const QString& filename, bool replace, bool organization)
{
  CHECK_ARG(QFileInfo(filename).exists());

  QSettings loadSettings(filename, QSettings::IniFormat);

  QScopedPointer<QSettings> saveSettings(organization
    ? new QSettings(QApplication::organizationName())
    : new QSettings);

  if (replace)
    saveSettings->clear();

  foreach (QString key, loadSettings.allKeys())
    {
    const QVariant value = loadSettings.value(key);
    saveSettings->setValue(key, value);
    }
  saveSettings->sync();
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
#ifndef _WIN32
  // Set up signal handlers for graceful shutdown on Ctrl+C
  setupSignalHandlers();
#endif

  // Set application information
  QApplication::setApplicationName("VisGUI Query Interface");
  QApplication::setOrganizationName("Kitware");
  QApplication::setOrganizationDomain("kitware.com");
  QApplication::setApplicationVersion(VIQUI_VERSION_STR);

  // Set up command line options
  qtCliArgs args(argc, argv);

#ifdef ENABLE_QTTESTING
  pqCoreTestUtility::AddCommandLineOptions(args);
#endif

  qtCliOptions options;
  options.add("ui <mode>", "Set user interface mode (analyst, engineering)");
  options.add("add-layer <uri>", "Add context layer from 'uri' on startup");
  options.add("import-config <file>", "Append settings from 'file'"
              " to the application's configuration space");
  options.add("import-config-org <file>", "Append settings from 'file'"
              " to the organization's configuration space");
  options.add("replace-config <file>", "Replace settings in the"
              " application's configuration space with those from 'file'");
  options.add("replace-config-org <file>", "Replace settings in the"
              " organization's configuration space with those from 'file'");
  args.addOptions(options);

  vgApplication::addCommandLineOptions(args);

  // Parse arguments
  args.parseOrDie();
  vgApplication::parseCommandLine(args);

  // Handle various config-related options
  qtUtil::map(args.values("import-config"),
              &parseConfigFile, false, false);
  qtUtil::map(args.values("import-config-org"),
              &parseConfigFile, false, true);
  qtUtil::map(args.values("replace-config"),
              &parseConfigFile, true, false);
  qtUtil::map(args.values("replace-config-org"),
              &parseConfigFile, true, true);

  // Create application instance and set copyright information
  vgApplication app(args.qtArgc(), args.qtArgv());
  app.setCopyright(VIQUI_COPY_YEAR, "Kitware, Inc.");

  // Register metatypes
  QTE_REGISTER_METATYPE(vvProcessingRequest);
  QTE_REGISTER_METATYPE(vvQueryInstance);
  QTE_REGISTER_METATYPE(QList<vvDescriptor>);
  QTE_REGISTER_METATYPE(QList<vvTrack>);
  QTE_REGISTER_METATYPE(vvQueryResult);
  QTE_REGISTER_METATYPE(vvIqr::ScoringClassifiers);
  QTE_REGISTER_METATYPE(const qtCliArgs*);

  // Pre-load KWIVER plugins to avoid delay when starting queries
  kwiver::vital::plugin_manager::instance().load_all_plugins();

  vqApplication mainWindow(args.value("ui") == "analyst" ?
                           vqApplication::UI_Analyst :
                           vqApplication::UI_Engineering);
  mainWindow.showMaximized();

  // Connect aboutToQuit to ensure proper cleanup when Ctrl+C or SIGTERM is
  // received. QCoreApplication::quit() (called from signal handler) only
  // causes the event loop to exit but doesn't trigger closeEvent() on windows.
  QObject::connect(&app, SIGNAL(aboutToQuit()), &mainWindow, SLOT(close()));

  // Pre-load plans for pre-defined query formulation
  vqPredefinedQueryCache::reload();

  // Load requested layer(s)
  foreach (const QString& layer, args.values("add-layer"))
    {
    mainWindow.addLayer(QUrl::fromUserInput(layer));
    }

#ifdef ENABLE_QTTESTING
  // If given testing arguments, run tests on the main window; invoke by queued
  // connection so this doesn't execute until the event loop is up and running
  QMetaObject::invokeMethod(&mainWindow, "initializeTesting",
                            Qt::QueuedConnection,
                            Q_ARG(const qtCliArgs*, &args));
#endif

  vtkObject::GlobalWarningDisplayOff();

  return app.exec();
}
