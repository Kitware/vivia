/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfSourceService_h
#define __vdfSourceService_h

#include <vgExport.h>

template <typename T> class QList;
class QUrl;

struct vdfArchivePluginInfo;
class vdfDataSource;

//-----------------------------------------------------------------------------
/// Methods for interfacing with the data sources plugin framework.
///
/// This namespace provides methods to query and interact with data source
/// plugins. Plugin loading is done on first use (that is, the first time any
/// function in the namespace is called).
namespace vdfSourceService
{
  // TODO:
  // Eventually this should more closely mirror vsSourceService (e.g. handling
  // argument / action registration and arbitrary plugins). For now, we only
  // care about archive sources.

  /// Get information on archive plugins.
  ///
  /// This method returns a ::vdfArchivePluginInfo structure for all loaded
  /// archive plugins servicing the specified \p type.
  VG_DATA_FRAMEWORK_EXPORT QList<vdfArchivePluginInfo> archivePluginInfo();

  /// Create archive source factory.
  ///
  /// This method attempts to create an archive source factory of the specified
  /// \p type from the specified \p archive. This is done by asking all loaded
  /// archive source plugins to create a static source factory from the
  /// specified \p archive. If no plugin is able to process the \p archive,
  /// a null pointer is returned.
  VG_DATA_FRAMEWORK_EXPORT vdfDataSource* createArchiveSource(
    const QUrl& archive);
}

#endif
