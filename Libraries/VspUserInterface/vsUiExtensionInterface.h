// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsUiExtensionInterface_h
#define __vsUiExtensionInterface_h

#include <QtPlugin>

#include <qtGlobal.h>

#include <vgExport.h>

class qtCliArgs;
class qtCliOptions;

class vsCore;
class vsMainWindow;
class vsScene;

class vsUiExtensionInterfacePrivate;

//-----------------------------------------------------------------------------
class VSP_USERINTERFACE_EXPORT vsUiExtensionInterface
{
public:
  virtual ~vsUiExtensionInterface();

  /// Initialize the extension.
  ///
  /// This method is used to perform any initialization of the plugin that
  /// cannot be performed in the constructor (e.g. because it needs the vsCore
  /// instance or otherwise depends on the application being already fully
  /// initialized).
  ///
  /// \param core Pointer to the application vsCore instance.
  ///
  /// The default implementation merely stores the pointer to the vsCore
  /// instance; it should be called by subclasses that wish to use core().
  virtual void initialize(vsCore* core);

  /// Create user interface for new main window.
  ///
  /// This method is called to request that the plugin create its user
  /// interface elements for a new main window. Implementations should create
  /// their menu actions, tool bar actions/widgets, dock windows, etc. and add
  /// them to the main user interface.
  ///
  /// \param window New window object to which the extension interface should
  ///               be added.
  /// \param scene Scene associated with the window.
  ///
  /// The default implementation does nothing.
  virtual void createInterface(vsMainWindow* window, vsScene* scene);

  /// Register CLI options for extension plugin.
  ///
  /// This method is called to allow the UI extension plugin to add any
  /// additional command line options it wishes to be available to the user.
  ///
  /// The default implementation does nothing.
  virtual void registerExtensionCliOptions(qtCliOptions&);

  /// Parse CLI arguments for extension plugin.
  ///
  /// This method is called to allow the UI extension plugin to process the
  /// command line arguments and take any necessary actions based on the same.
  ///
  /// \note This method is called once, prior to the creation of per-window
  ///       interfaces. As a result, plugins may need to cache argument values
  ///       for later use, e.g. in createInterface().
  ///
  /// The default implementation does nothing.
  virtual void parseExtensionArguments(const qtCliArgs&);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsUiExtensionInterface)

  vsUiExtensionInterface();

  /// Return pointer to the application vsCore instance, as passed to
  /// initialize().
  vsCore* core() const;

private:
  QTE_DECLARE_PRIVATE(vsUiExtensionInterface)
  QTE_DISABLE_COPY(vsUiExtensionInterface)
};

Q_DECLARE_INTERFACE(vsUiExtensionInterface,
                    "org.visgui.vsUiExtensionInterface")

#endif
