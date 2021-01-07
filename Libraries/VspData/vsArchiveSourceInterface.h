// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsArchiveSourceInterface_h
#define __vsArchiveSourceInterface_h

#include <QtPlugin>

#include "vsArchiveSourceInfo.h"

class QUrl;

class vsSimpleSourceFactory;

//-----------------------------------------------------------------------------
/// Interface for an archive source plugin.
///
/// This class defines the public interface for an archive source plugin.
class vsArchiveSourceInterface
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
    /// This mode is used first by ::vsSourceService::createArchiveSource.
    QuickTest,
    /// Thorough archive validation.
    ///
    /// This mode specifies that the plugin should make a thorough attempt to
    /// process the archive, rather than relying on quick checks. This is meant
    /// to overcome problems such as an incorrectly formatted file name.
    ///
    /// This mode is used by ::vsSourceService::createArchiveSource after all
    /// plugins have been asked to process the archive in ::QuickTest mode, but
    /// none accepted the archive.
    ThoroughTest
    };

  virtual ~vsArchiveSourceInterface() {}

  /// Get set of archive types supported by plugin.
  ///
  /// This method returns the set of archive types that the plugin services.
  virtual vsArchiveSourceTypes archiveTypes() const = 0;

  /// Get plugin information.
  ///
  /// This method returns a ::vsArchivePluginInfo structure describing the
  /// plugin (in particular, what file formats the plugin supports). The
  /// \p type of source is provided as a hint to allow plugins supporting more
  /// than one source type to return information specific to the \p type being
  /// queried.
  virtual vsArchivePluginInfo archivePluginInfo(
    vsArchiveSourceType type) const = 0;

  /// Create data sources, if the archive is supported.
  ///
  /// This method requests that the plugin create sources for the specified
  /// \p archive. The plugin should test if the \p archive is of a type that it
  /// supports, and if not, return null. The assumed \p type of archive is
  /// provided as a hint in case the plugin supports multiple source types and
  /// needs to know what type the user asked to open for proper behavior.
  virtual vsSimpleSourceFactory* createArchiveSource(
    vsArchiveSourceType type, const QUrl& archive, SourceCreateMode) = 0;
};

Q_DECLARE_INTERFACE(vsArchiveSourceInterface,
                    "org.visgui.vsArchiveSourceInterface")

#endif
