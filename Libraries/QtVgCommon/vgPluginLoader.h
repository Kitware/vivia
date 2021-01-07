// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgPluginLoader_h
#define __vgPluginLoader_h

#include <QObject>

#include <vgExport.h>

//-----------------------------------------------------------------------------
namespace vgPluginLoader
{
  /// Get list of plugin instance objects.
  ///
  /// \param interfaceId Name of the desired plugin interface, or \c nullptr to
  ///                    obtain all plugins.
  ///
  /// This method returns the list of all plugin instance objects that can be
  /// successfully loaded and (if \p interfaceId is not \c nullptr) implement
  /// the specified interface. If plugins have already been loaded, it returns
  /// quickly. Otherwise, plugins are loaded when the function is called.
  ///
  /// The results of an interface query are cached; the initial query for a
  /// given interface may be slightly slower due to the need to test each
  /// plugin to see if it implements the specified interface.
  ///
  /// Most users should use the template version of this function instead.
  QTVG_COMMON_EXPORT QObjectList plugins(const char* interfaceId = 0);

  /// Get list plugin instance objects implementing the specified template.
  ///
  /// \tparam Interface Desired plugin interface
  ///
  /// This method returns the list of all plugin interfaces of the specified
  /// type from all plugins that can be successfully loaded. If plugins have
  /// already been loaded, it returns quickly. Otherwise, plugins are loaded
  /// when the function is called.
  ///
  /// The results of an interface query are cached; the initial query for a
  /// given interface may be slightly slower due to the need to test each
  /// plugin to see if it implements the specified interface.
  template <typename Interface>
  inline QList<Interface*> pluginInterfaces();
};

//-----------------------------------------------------------------------------
template <typename Interface>
QList<Interface*> vgPluginLoader::pluginInterfaces()
{
  const char* const iid = qobject_interface_iid<Interface*>();
  const QObjectList objects = plugins(iid);

  QList<Interface*> result;
  result.reserve(objects.count());

  foreach (QObject* const plugin, objects)
    {
    result.append(reinterpret_cast<Interface*>(plugin->qt_metacast(iid)));
    }
  return result;
}

#endif
