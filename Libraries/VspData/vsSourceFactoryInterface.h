/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsSourceFactoryInterface_h
#define __vsSourceFactoryInterface_h

#include <QtPlugin>

#include <vgExport.h>

#include <qtPrioritizedMenuProxy.h>

class QSignalMapper;
class QString;

template <typename T> class QList;

class qtCliArgs;
class qtCliOptions;

struct vsPendingFactoryAction;
class vsSourceFactory;

//-----------------------------------------------------------------------------
/// Interface for a source factory plugin.
///
/// This class defines the public interface for a source factory plugin.
class VSP_DATA_EXPORT vsSourceFactoryInterface
{
public:
  virtual ~vsSourceFactoryInterface();

  /// Return the identifier of this source factory.
  ///
  /// This method returns an identifier for the source factory. The source
  /// service uses this identifier to choose the appropriate plugin when asked
  /// to create a factory. As such, implementations should take care to ensure
  /// that the identifier is unique.
  virtual QString identifier() const = 0;

  /// Return key for creating actions.
  ///
  /// This method returns a QSettings key suitable for uniquely identifying a
  /// single action associated with the plugin.
  virtual QString settingsKey() const;

  /// Register CLI options for factory plugin.
  ///
  /// This method is called to allow the source factory plugin to add any
  /// additional command line options it wishes to be available to the user.
  /// The most common example would be an option to connect to a source
  /// specified via the command line.
  ///
  /// The default implementation does nothing.
  virtual void registerFactoryCliOptions(qtCliOptions&);

  /// Parse CLI arguments for factory plugin.
  ///
  /// This method is called to allow the source factory plugin to process the
  /// command line arguments and take any necessary actions based on the same.
  ///
  /// The return value is a list of source connections that should be created
  /// based on the command line arguments, which may be empty.
  ///
  /// The default implementation does nothing and returns an empty list.
  virtual QList<vsPendingFactoryAction> parseFactoryArguments(const qtCliArgs&);

  /// Register actions.
  ///
  /// This method is called to register actions that will be used by the
  /// plugin. Implementations should create action factories for any actions
  /// they wish to define when this method is called.
  ///
  /// The default implementation does nothing.
  virtual void registerActions();

  /// Create actions.
  ///
  /// This method is called to request that the plugin create menu actions that
  /// the user can use to create an instance of the source. Implementations
  /// should create their action(s) and add them to the appropriate menu(s)
  /// when this method is called. The action's ::QAction::triggered signal
  /// should be connected to the \p signalMapper, using the plugin's
  /// ::identifier as the mapping key.
  ///
  /// The default implementation does nothing.
  ///
  /// \param parent A ::QObject suitable for use as the parent object of any
  ///               actions created.
  /// \param signalMapper ::QSignalMapper that should be triggered when the
  ///                     action is triggered.
  /// \param videoMenu Insertion placeholder for the application's 'Video'
  ///                  menu.
  /// \param trackMenu Insertion placeholder for the application's 'Tracks'
  ///                  menu.
  /// \param descriptorMenu Insertion placeholder for the application's
  ///                       'Descriptors' menu.
  virtual void createActions(
    QObject* parent, QSignalMapper* signalMapper,
    qtPrioritizedMenuProxy& videoMenu,
    qtPrioritizedMenuProxy& trackMenu,
    qtPrioritizedMenuProxy& descriptorMenu);

  /// Create a new factory instance.
  ///
  /// This method creates a new instance of the source factory plugin's factory
  /// object.
  ///
  /// The default implementation returns a null pointer.
  virtual vsSourceFactory* createFactory();

  /// Return name of display group for source factory actions.
  ///
  /// This method returns the name of the display group under which source
  /// factory plugins should register their actions with ::qtActionManager.
  static QString actionDisplayGroup();

  /// Set name of display group for source factory actions.
  ///
  /// This method sets the name that will be returned by ::actionGroup. The
  /// name is stored as a property on the ::QCoreApplication instance.
  /// Applications should call this method prior to calling
  /// ::vsSourceService::registerActions if they wish to change the default
  /// name for the action display group.
  static void setActionDisplayGroup(const QString&);
};

Q_DECLARE_INTERFACE(vsSourceFactoryInterface,
                    "org.visgui.vsSourceFactoryInterface")

#endif
