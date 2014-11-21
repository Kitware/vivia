/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvQueryServiceInterface_h
#define __vvQueryServiceInterface_h

#include <QtPlugin>

class QStringList;
class QUrl;

class vvQueryServerDialog;
class vvQuerySession;

//-----------------------------------------------------------------------------
class vvQueryServiceInterface
{
public:
  virtual ~vvQueryServiceInterface() {}

  /// Get list of supported schemes.
  ///
  /// This method returns a list of all URI schemes supported by this provider
  /// instance.
  virtual QStringList supportedSchemes() const = 0;

  /// Register server choosers with chooser dialog.
  ///
  /// This method registers a provider's server chooser(s) with the specified
  /// server chooser dialog.
  virtual void registerChoosers(vvQueryServerDialog*) = 0;

  /// Create session for URI.
  ///
  /// This method creates a new ::vvQuerySession instance for the specified
  /// URI. In case of errors, a null pointer is returned.
  virtual vvQuerySession* createSession(const QUrl&) = 0;
};

Q_DECLARE_INTERFACE(vvQueryServiceInterface,
                    "org.visgui.vvQueryServiceInterface")

#endif
