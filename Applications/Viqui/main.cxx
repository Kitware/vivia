/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <QtCore>
#include <QApplication>
#include <QList>
#include <QMainWindow>
#include <QScopedPointer>

#include <qtCliArgs.h>
#include <qtMap.h>
#include <qtStlUtil.h>

#include <vgCheckArg.h>

#include <vvQuery.h>
#include <vvQueryResult.h>
#include <vvQueryService.h>
#include <vvQuerySession.h>

#ifdef ENABLE_QTTESTING
#include <pqCoreTestUtility.h>
#endif

#include "vqApplication.h"
#include "vqPredefinedQueryCache.h"
#include "vqVersion.h"

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

  // Parse arguments
  args.parseOrDie();

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
  QApplication app(args.qtArgc(), args.qtArgv());

  app.setProperty("COPY_YEAR", VIQUI_COPY_YEAR);
  app.setProperty("COPY_ORGANIZATION", "Kitware, Inc.");

  // Register metatypes
  QTE_REGISTER_METATYPE(vvProcessingRequest);
  QTE_REGISTER_METATYPE(vvQueryInstance);
  QTE_REGISTER_METATYPE(QList<vvDescriptor>);
  QTE_REGISTER_METATYPE(QList<vvTrack>);
  QTE_REGISTER_METATYPE(vvQueryResult);
  QTE_REGISTER_METATYPE(vvIqr::ScoringClassifiers);
  QTE_REGISTER_METATYPE(const qtCliArgs*);

  vqApplication mainWindow(args.value("ui") == "analyst" ?
                           vqApplication::UI_Analyst :
                           vqApplication::UI_Engineering);
  mainWindow.show();

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

  return app.exec();
}
