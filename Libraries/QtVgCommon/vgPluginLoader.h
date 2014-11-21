/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgPluginLoader_h
#define __vgPluginLoader_h

#include <QObject>

#include <vgExport.h>

//-----------------------------------------------------------------------------
namespace vgPluginLoader
{
  /// Get list of all plugin instance objects.
  ///
  /// This method returns the list of all plugin instance objects that can be
  /// successfully loaded. If plugins have already been loaded, it returns
  /// quickly. Otherwise, plugins are loaded when the function is called.
  QTVG_COMMON_EXPORT QObjectList plugins();
};

#endif
