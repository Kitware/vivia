// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfArchiveSourceInterface_h
#define __vdfArchiveSourceInterface_h

#include <QtPlugin>

#include "vdfArchiveSourceInfo.h"

class QUrl;

class vdfDataSource;

//-----------------------------------------------------------------------------
/// Interface for an archive source plugin.
///
/// This class defines the public interface for an archive source plugin.
class vdfArchiveSourceInterface
{
public:
  /// Mode in which source creation should operate.
  enum SourceCreateMode
    {
    /// Quick archive validation.
    ///
    /// This mode specifies that the plugin should make a quick, educated guess
    /// if it supports the specified archive, and fail otherwise. Usually this
    /// means simply checking if the archive is a file name matching a well
    /// known pattern supported by the plugin. This is meant to prevent a
    /// plugin from trying overly hard to process an archive it does not
    /// support in order to allow other plugins a chance to accept the archive.
    /// A quick attempt should be followed by a thorough attempt before the
    /// archive is accepted.
    ///
    /// This mode is used first by ::vdfSourceService::createArchiveSource.
    QuickTest,
    /// Thorough archive validation.
    ///
    /// This mode specifies that the plugin should make a thorough attempt to
    /// process the archive, rather than relying on quick checks. This is meant
    /// to overcome problems such as an incorrectly formatted file name.
    ///
    /// This mode is used by ::vdfSourceService::createArchiveSource after all
    /// plugins have been asked to process the archive in ::QuickTest mode, but
    /// none accepted the archive.
    ThoroughTest
    };

  virtual ~vdfArchiveSourceInterface() {}

  /// Get plugin information.
  ///
  /// This method returns a ::vdfArchivePluginInfo structure describing the
  /// plugin (in particular, what file formats the plugin supports).
  virtual vdfArchivePluginInfo archivePluginInfo() const = 0;

  /// Create data source, if the archive is supported.
  ///
  /// This method requests that the plugin create sources for the specified
  /// \p archive. The plugin should test if the \p archive is of a type that it
  /// supports, and if not, return null.
  virtual vdfDataSource* createArchiveSource(
    const QUrl& archive, SourceCreateMode) = 0;
};

Q_DECLARE_INTERFACE(vdfArchiveSourceInterface,
                    "org.visgui.framework.data.archiveSourceInterface")

#endif
