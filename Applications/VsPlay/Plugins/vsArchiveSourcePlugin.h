/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsArchiveSourcePlugin_h
#define __vsArchiveSourcePlugin_h

#include <vsArchiveSourceInfo.h>

#include <vsSourceFactoryPlugin.h>

class vsArchiveSourcePluginPrivate;

class vsArchiveSourcePlugin : public vsSourceFactoryPlugin
{
public:
  virtual ~vsArchiveSourcePlugin();

  /// Parse CLI arguments for factory plugin.
  ///
  /// This method is called by the source factory framework to allow the plugin
  /// to process command line arguments. The default implementation will
  /// request creation of its subclass's source for any files passed by command
  /// line options registered with ::addArchiveOption.
  virtual QList<vsPendingFactoryAction> parseFactoryArguments(
    const qtCliArgs&) QTE_OVERRIDE;

  /// Create a new factory instance.
  ///
  /// This method will create a new ::vsArchiveSourceFactory instance with the
  /// same source type that was passed to this instance at creation.
  virtual vsSourceFactory* createFactory() QTE_OVERRIDE;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsArchiveSourcePlugin)

  /// Construct a generic archive plugin bound to the specified \p type.
  explicit vsArchiveSourcePlugin(vsArchiveSourceType type);

  /// Check if back-ends are available.
  ///
  /// This method tests if there are any loaded plugins able to handle the
  /// source type assigned to this instance. Subclasses should call this before
  /// registering command line options or creating menu actions, to avoid
  /// giving the appearance that a source of our type can be created when in
  /// fact there are no archive plugins available for the source type.
  ///
  /// \return \c true if there are loaded archive source plugins that support
  ///         our type, \c false otherwise.
  bool areBackendsAvailable();

  /// Add a CLI option to load an archive.
  ///
  /// This method registers a command line option to load an archive, using a
  /// standard format for the option. The option will have the specified
  /// \p shortName, and a long name of \p type suffixed with "-file". Note that
  /// \p type is also used in the option description, and should be the plural
  /// of the data type (e.g. "video", "tracks").
  void addArchiveOption(
    qtCliOptions& options, const QString& type, const QString& shortName);

private:
  QTE_DECLARE_PRIVATE(vsArchiveSourcePlugin)
  QTE_DISABLE_COPY(vsArchiveSourcePlugin)
};

#endif
