// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvQueryService_h
#define __vvQueryService_h

#include <QUrl>

#include <vgExport.h>

class QStringList;

class vvQueryServerDialog;
class vvQuerySession;

//-----------------------------------------------------------------------------
/// Methods for interfacing with the query service framework.
///
/// This namespace provides methods to query and interact with the query
/// service framework, which is a generic front-end for service provider
/// plugins. Plugin loading is done on first use (that is, the first time any
/// function in the namespace is called).
namespace vvQueryService
{
  /// Get list of supported schemes.
  ///
  /// This method returns a unified list of all URI schemes supported by all
  /// loaded service provider plugins.
  VV_IO_EXPORT QStringList supportedSchemes();

  /// Register server choosers with chooser dialog.
  ///
  /// This method registers server choosers for all loaded service provider
  /// plugins with the specified server chooser dialog.
  VV_IO_EXPORT void registerChoosers(vvQueryServerDialog*);

  /// Create session for URI.
  ///
  /// This method attempts to create a ::vvQuerySession instance for the
  /// specified URI, by searching all loaded service provider plugins for one
  /// which is able to handle the URI. If no provider can be found, a null
  /// pointer is returned.
  VV_IO_EXPORT vvQuerySession* createSession(const QUrl&);
};

#endif
