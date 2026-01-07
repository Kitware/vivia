// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsSourceFactoryPlugin_h
#define __vsSourceFactoryPlugin_h

#include <qtGlobal.h>

#include <vgExport.h>

#include <vsSourceFactoryInterface.h>

class qtActionFactory;

class vsSourceFactoryPluginPrivate;

//-----------------------------------------------------------------------------
/// An abstract source factory plugin that supports actions.
///
/// This class provides an abstract base class for a source factory plugin
/// that registers and creates actions. Source factory plugins can derive from
/// this class to take advantage of its action management functions to simplify
/// creation and registration of UI actions.
class VSP_SOURCEUTIL_EXPORT vsSourceFactoryPlugin
  : public vsSourceFactoryInterface
{
public:
  virtual ~vsSourceFactoryPlugin();

  /// Create actions.
  ///
  /// This method prepares the instance to create registered actions, and calls
  /// ::insertActions to allow subclasses to insert actions into the
  /// appropriate menus. Subclasses overriding this method must call this base
  /// class implementation before calling ::action.
  virtual void createActions(
    QObject* parent, QSignalMapper* signalMapper,
    qtPrioritizedMenuProxy& videoMenu, qtPrioritizedMenuProxy& trackMenu,
    qtPrioritizedMenuProxy& descriptorMenu) QTE_OVERRIDE;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsSourceFactoryPlugin)

  vsSourceFactoryPlugin();

  /// Register an action.
  ///
  /// This method registers an action with the specified \p key, and returns
  /// the resulting action factory. The action uses the standard settings key
  /// and display group.
  qtActionFactory* registerAction(
    int key, const QString& displayText, const QString& iconName,
    const QString& shortcutPortableString, const QString& toolTipText);

  /// Retrieve an action.
  ///
  /// This method retrieves a ::QAction instance of a previously registered
  /// action, creating a new instance if necessary. The action is already
  /// parented, and already connected to the signal mapper passed to
  /// ::createActions.
  ///
  /// \par Caution:
  /// This method should not be called outside of ::insertActions.
  QAction* action(int key);

  /// Insert actions into menus.
  ///
  /// This method is called to request that the plugin insert menu actions that
  /// the user can use to create an instance of the source. Registered actions
  /// should be retrieved via ::action.
  ///
  /// The default implementation does nothing.
  ///
  /// \param videoMenu Insertion placeholder for the application's 'Video'
  ///                  menu.
  /// \param trackMenu Insertion placeholder for the application's 'Tracks'
  ///                  menu.
  /// \param descriptorMenu Insertion placeholder for the application's
  ///                       'Descriptors' menu.
  virtual void insertActions(
    qtPrioritizedMenuProxy& videoMenu, qtPrioritizedMenuProxy& trackMenu,
    qtPrioritizedMenuProxy& descriptorMenu);

private:
  QTE_DECLARE_PRIVATE(vsSourceFactoryPlugin)
  QTE_DISABLE_COPY(vsSourceFactoryPlugin)
};

#endif
