/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsVersion.h"

#include <vsApplication.h>
#include <vsMainWindow.h>
#include <vsUiExtensionInterface.h>

#include <vsContour.h>
#include <vsDataSource.h>
#include <vsDescriptor.h>
#include <vsDescriptorInput.h>
#include <vsEvent.h>
#include <vsEventInfo.h>
#include <vsFactoryAction.h>
#include <vsSourceService.h>
#include <vsTrackClassifier.h>
#include <vsTrackData.h>
#include <vsTrackId.h>
#include <vsTrackState.h>

#include <vvQueryInstance.h>
#include <vvQueryService.h>

#include <vvQueryResult.h>

#include <vgVideoSourceRequestor.h>

#include <vtkVgVideoFrameMetaData.h>

#include <vgImage.h>

#include <vgPluginLoader.h>

#include <vgMatrix.h>

#ifdef ENABLE_QTTESTING
#include <pqCoreTestUtility.h>
#endif

#include <qtCliArgs.h>
#include <qtUtil.h>

#include <QtCore>
#include <QAbstractAnimation>
#include <QApplication>
#include <QDir>
#include <QHostAddress>
#include <QMainWindow>
#include <QSettings>

// Register built-in data source plugins
Q_IMPORT_PLUGIN(vsDescriptorArchiveSourcePlugin)
Q_IMPORT_PLUGIN(vsTrackArchiveSourcePlugin)
Q_IMPORT_PLUGIN(vsVideoArchiveSourcePlugin)
Q_IMPORT_PLUGIN(vsVvqsDatabaseSourcePlugin)

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  // Set application information
  QApplication::setApplicationName("Video Stream Player");
  QApplication::setOrganizationName("Kitware");
  QApplication::setOrganizationDomain("kitware.com");
  QApplication::setApplicationVersion(VSPLAY_VERSION_STR);

  // Set up command line options
  qtCliArgs args(argc, argv);

#ifdef ENABLE_QTTESTING
  pqCoreTestUtility::AddCommandLineOptions(args);
#endif

  qtCliOptions options;

  vsSourceService::registerCliOptions(options);
  foreach (vsUiExtensionInterface* const extension,
           vgPluginLoader::pluginInterfaces<vsUiExtensionInterface>())
    {
    extension->registerExtensionCliOptions(options);
    }

  options.add("filters-file <file>", "Load filter settings from 'file'")
         .add("ff", qtCliOption::Short);
  options.add("seek <time>",
              "Attempt to seek to 'time' once video is loaded");
  options.add("import-config <file>", "Append settings from 'file'"
              " to the application's configuration space");
  options.add("import-config-org <file>", "Append settings from 'file'"
              " to the organization's configuration space");
  options.add("mask-image <file>", "Mask image to overlay on video");
  args.addOptions(options);

  vgApplication::addCommandLineOptions(args);

  // Parse arguments
  args.parseOrDie();
  vgApplication::parseCommandLine(args);

  // Create application instance and set copyright information
  vsApplication app(args.qtArgc(), args.qtArgv());

  app.setCopyright(VSPLAY_COPY_YEAR, "Kitware, Inc.");
  app.setUserManualLocation("vsPlay.html");

  // Register metatypes
  QTE_REGISTER_METATYPE(QHostAddress);
  QTE_REGISTER_METATYPE(vgImage);
  QTE_REGISTER_METATYPE(vgMatrix4d);
  QTE_REGISTER_METATYPE(vgTimeStamp);
  QTE_REGISTER_METATYPE(vtkVgTimeStamp);
  QTE_REGISTER_METATYPE(vtkVgTimeStamp*);
  QTE_REGISTER_METATYPE(vgVideoSeekRequest);
  QTE_REGISTER_METATYPE(vgVideoSourceRequestor*);
  QTE_REGISTER_METATYPE(vtkVgVideoFrame*);
  QTE_REGISTER_METATYPE(vtkVgVideoFrameMetaData);
  QTE_REGISTER_METATYPE(QList<vtkVgVideoFrameMetaData>);
  QTE_REGISTER_METATYPE(vsDataSource::Status);
  QTE_REGISTER_METATYPE(vsContour);
  QTE_REGISTER_METATYPE(vsTrackObjectClassifier);
  QTE_REGISTER_METATYPE(vvDescriptor);
  QTE_REGISTER_METATYPE(vsDescriptor);
  QTE_REGISTER_METATYPE(vsDescriptorList);
  QTE_REGISTER_METATYPE(vsTrackData);
  QTE_REGISTER_METATYPE(vvTrackState);
  QTE_REGISTER_METATYPE(QList<vvTrackState>);
  QTE_REGISTER_METATYPE(vvQueryInstance);
  QTE_REGISTER_METATYPE(vvQueryResult);
  QTE_REGISTER_METATYPE(vtkIdType);
  QTE_REGISTER_METATYPE(vsTrackId);
  QTE_REGISTER_METATYPE(vsEvent);
  QTE_REGISTER_METATYPE(vsEventInfo);
  QTE_REGISTER_METATYPE(vsEventInfo::Group);
  QTE_REGISTER_METATYPE(vsDescriptorInputPtr);
  QTE_REGISTER_METATYPE(const qtCliArgs*);

  // Register actions for source factory plugins
  vsSourceService::registerActions();

  // Import configuration file, if requested
  foreach (QString config, args.values("import-config"))
    {
    QSettings loadSettings(config, QSettings::IniFormat);
    QSettings saveSettings;

    foreach (QString key, loadSettings.allKeys())
      saveSettings.setValue(key, loadSettings.value(key));
    saveSettings.sync();
    }

  foreach (QString config, args.values("import-config-org"))
    {
    QSettings loadSettings(config, QSettings::IniFormat);
    QSettings saveSettings(QApplication::organizationName());

    foreach (QString key, loadSettings.allKeys())
      saveSettings.setValue(key, loadSettings.value(key));
    saveSettings.sync();
    }

  // Parse plugin-specific command line options and get list of sources to be
  // created based on the same
  QList<vsPendingFactoryAction> sourceActions =
    vsSourceService::parseArguments(args);

  // Initialize application (i.e. creates vsCore) and create first view
  app.initialize(args);
  app.newView();

  // Connect sources specified on command line
  foreach (vsPendingFactoryAction sourceAction, sourceActions)
    {
    app.connectSource(sourceAction.FactoryIdentifier, sourceAction.SourceUri);
    }

  vsMainWindow* view = app.firstView();
  if (view)
    {
    const QString filtersFile = args.value("filters-file");
    if (!filtersFile.isEmpty())
      {
      // Load filter settings specified on command line
      view->loadFilterSettings(filtersFile);
      }

    const QString maskFile = args.value("mask-image");
    if (!maskFile.isEmpty())
      {
      // Load image mask
      view->loadMaskImage(maskFile);
      }

    bool okay;
    double seekTime = args.value("seek").toDouble(&okay);
    if (okay)
      {
      // Request pending seek
      view->setPendingSeek(vtkVgTimeStamp(seekTime));
      }

#ifdef ENABLE_QTTESTING
    // If given testing arguments, run tests on the initial view; invoke by
    // queued connection so this doesn't execute until the event loop is up and
    // running
    QMetaObject::invokeMethod(view, "initializeTesting", Qt::QueuedConnection,
                              Q_ARG(const qtCliArgs*, &args));
#endif
    }

  return app.exec();
}
