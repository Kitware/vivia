/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// QT includes
#include <QApplication>
#include <QCleanlooksStyle>
#include <QSettings>

// QT Extensions includes
#include <qtCliArgs.h>

// QT Testing Support includes
#ifdef ENABLE_QTTESTING
#include <pqCoreTestUtility.h>
#endif

// VisGUI includes
#include "vpApplication.h"
#include "vpView.h"

#include "vpVersion.h"

extern int qInitResources_icons();

int main(int argc, char** argv)
{
  // Force ini-style configuration files on Windows for easy editing.
  QSettings::setDefaultFormat(QSettings::IniFormat);

  QApplication::setApplicationName("VisGUI View");
  QApplication::setOrganizationName("Kitware");
  QApplication::setOrganizationDomain("kitware.com");
  QApplication::setApplicationVersion(VPVIEW_VERSION_STR);

  // Set up command line options
  qtCliArgs args(argc, argv);

#ifdef ENABLE_QTTESTING
  pqCoreTestUtility::AddCommandLineOptions(args);
#endif

  qtCliOptions options;
  options.add("project <file>").add("p", "Load project 'file'");
  options.add("streaming").add("s", "Enable streaming mode");
  options.add("import-config <file>", "Append settings from 'file'"
              " to the application's configuration space");
  args.addOptions(options);

  vgApplication::addCommandLineOptions(args);

  // Parse arguments
  args.parseOrDie();
  vgApplication::parseCommandLine(args);

  // Create application instance and set copyright information
  vpApplication app(args.qtArgc(), args.qtArgv());
  app.setCopyright(VPVIEW_COPY_YEAR, "Kitware, Inc.");

  // Import configuration file, if requested
  foreach (QString config, args.values("import-config"))
    {
    QSettings loadSettings(config, QSettings::IniFormat);
    QSettings saveSettings;

    foreach (QString key, loadSettings.allKeys())
      saveSettings.setValue(key, loadSettings.value(key));
    saveSettings.sync();
    }

  // Create and show the main window
  vpView myView;
  myView.show();
  myView.initialize(&args);

  return app.exec();
}
