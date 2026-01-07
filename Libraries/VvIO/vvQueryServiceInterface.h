// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
