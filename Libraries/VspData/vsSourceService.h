// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsSourceService_h
#define __vsSourceService_h

#include <vgExport.h>

#include <qtPrioritizedMenuProxy.h>

#include "vsArchiveSourceInfo.h"
#include "vsSourceFactory.h"

class QSignalMapper;

class qtCliArgs;
class qtCliOptions;

struct vsPendingFactoryAction;

//-----------------------------------------------------------------------------
/// Methods for interfacing with the data sources plugin framework.
///
/// This namespace provides methods to query and interact with data source
/// plugins. Plugin loading is done on first use (that is, the first time any
/// function in the namespace is called).
namespace vsSourceService
{
  /// Register CLI options for plugins.
  ///
  /// This method registers command line options for all loaded data source
  /// plugins. This allows plugins to define command line options for their
  /// use, most often to allow source factory plugins to create options for
  /// connecting sources on start-up via the command line.
  VSP_DATA_EXPORT void registerCliOptions(qtCliOptions&);

  /// Parse CLI arguments for plugins.
  ///
  /// This method sends the parsed command line arguments to all loaded plugins
  /// for processing. It is up to specific plugins what, if anything, they need
  /// to do with the arguments.
  ///
  /// The return value is a list of source connections that were requested by
  /// source factory plugins based on the command line arguments, which may be
  /// empty.
  VSP_DATA_EXPORT QList<vsPendingFactoryAction> parseArguments(
    const qtCliArgs&);

  /// Register actions for source factory plugins.
  ///
  /// This method registers actions for all loaded source factory plugins.
  /// This should be done as part of the application's start-up, but must not
  /// be done until \em after the QCoreApplication object has been created.
  VSP_DATA_EXPORT void registerActions();

  /// Create actions for source factory plugins.
  ///
  /// This method creates actions for all loaded source factory plugins.
  VSP_DATA_EXPORT void createActions(
    QObject* parent, QSignalMapper* signalMapper,
    qtPrioritizedMenuProxy& videoMenu,
    qtPrioritizedMenuProxy& trackMenu,
    qtPrioritizedMenuProxy& descriptorMenu);

  /// Create source factory.
  ///
  /// This method creates a source factory of the specified \p kind.
  VSP_DATA_EXPORT vsSourceFactoryPtr createFactory(const QString& kind);

  /// Get information on archive plugins.
  ///
  /// This method returns a ::vsArchivePluginInfo structure for all loaded
  /// archive plugins servicing the specified \p type.
  VSP_DATA_EXPORT QList<vsArchivePluginInfo> archivePluginInfo(
    vsArchiveSourceType type);

  /// Create archive source factory.
  ///
  /// This method attempts to create an archive source factory of the specified
  /// \p type from the specified \p archive. This is done by asking all loaded
  /// archive source plugins to create a static source factory from the
  /// specified \p archive. If no plugin is able to process the \p archive,
  /// a null pointer is returned.
  VSP_DATA_EXPORT vsSimpleSourceFactoryPtr createArchiveSource(
    vsArchiveSourceType type, const QUrl& archive);
}

#endif
