/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsUiExtensionInterface_h
#define __vsUiExtensionInterface_h

#include <QtPlugin>

#include <qtGlobal.h>

#include <vgExport.h>

class vsCore;
class vsScene;
class vsMainWindow;
class vsMenuPlaceholder;
class vsToolBarPlaceholder;

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
