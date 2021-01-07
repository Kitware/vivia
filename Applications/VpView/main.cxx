// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// QT includes
#include <QApplication>
#include <QSettings>

// vxl includes
#include <vil/vil_image_view.h>
#include <vgl/algo/vgl_h_matrix_2d.h>

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

  QApplication::setApplicationName("WAMI Viewer");
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
  args.addOptions(options);

  vgApplication::addCommandLineOptions(args);

  // Parse arguments
  args.parseOrDie();
  vgApplication::parseCommandLine(args);

  // Create application instance and set copyright information
  vpApplication app(args.qtArgc(), args.qtArgv());
  app.setCopyright(VPVIEW_COPY_YEAR, "Kitware, Inc.");

  // Register metatypes
  QTE_REGISTER_METATYPE(vil_image_view<vxl_byte>);
  QTE_REGISTER_METATYPE(vil_image_view<double>);
  QTE_REGISTER_METATYPE(std::string);
  QTE_REGISTER_METATYPE(std::vector<vgl_h_matrix_2d<double> >);

  vpView myView;
  myView.show();
  myView.initialize(&args);

  return app.exec();
}
