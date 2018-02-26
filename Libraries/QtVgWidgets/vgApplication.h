/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgApplication_h
#define __vgApplication_h

#include <QApplication>

#include <qtGlobal.h>

#include <vgExport.h>

class QMenu;

class vgApplicationPrivate;

/// Base class for ViViA applications.
///
/// This class extends QApplication to provide some common properties that are
/// used by vgAboutDialog and vgUserManualAction.
class QTVG_WIDGETS_EXPORT vgApplication : public QApplication
{
  Q_OBJECT

  /// Application copyright year.
  ///
  /// This specifies the year (or years) for which the application copyright
  /// applies.
  ///
  /// \sa copyrightYear(), setCopyrightYear()
  Q_PROPERTY(QString copyrightYear READ copyrightYear WRITE setCopyrightYear)

  /// Application copyright organization.
  ///
  /// This specifies the individual(s) or organizations(s) which own the
  /// application copyright.
  ///
  /// \sa copyrightOrganization(), setCopyrightOrganization()
  Q_PROPERTY(QString copyrightOrganization READ copyrightOrganization
                                           WRITE setCopyrightOrganization)

  /// Location of application user manual.
  ///
  /// This specifies the file name or location of the application's user
  /// manual.
  ///
  /// \sa userManualLocation(), setUserManualLocation()
  Q_PROPERTY(QString userManualLocation READ userManualLocation
                                        WRITE setUserManualLocation)

public:
  enum HelpMenuEntry
  {
    UserManualAction = 0x1,
    AboutAction = 0x2,
    FullHelpMenu = 0xff
  };
  Q_DECLARE_FLAGS(HelpMenuEntries, HelpMenuEntry);

public:
  vgApplication(int& argc, char** argv);
  virtual ~vgApplication();

  static vgApplication* instance();

  /// Get copyright year.
  /// \sa copyrightYear, setCopyrightYear()
  static QString copyrightYear();

  /// Set copyright year.
  /// \sa copyrightYear, copyrightYear()
  static void setCopyrightYear(const QString&);

  /// Set copyright year.
  /// \sa copyrightYear, copyrightYear()
  static void setCopyrightYear(int);

  /// Get copyright organization.
  /// \sa copyrightOrganization, setCopyrightOrganization()
  static QString copyrightOrganization();

  /// Set copyright organization.
  /// \sa copyrightOrganization, copyrightOrganization()
  static void setCopyrightOrganization(const QString&);

  /// Set the copyright year and organization.
  ///
  /// This convenience method sets both copyrightYear and copyrightOrganization
  /// in a single call.
  void setCopyright(const QString& year, const QString& organization);

  /// \copydoc setCopyright(const QString&, const QString&)
  void setCopyright(int year, const QString& organization);

  /// Get location of application user manual.
  ///
  /// The returns the absolute path to the application's user manual.
  ///
  /// \sa userManualLocation, setUserManualLocation()
  static QString userManualLocation();

  /// Set location of application user manual.
  ///
  /// This method will resolve non-absolute paths so that the stored value is
  /// always a complete absolute path. If a name without path is given
  /// (recommended), the user manual is assumed to be in a well known relative
  /// location with respect to the location of the application executable.
  ///
  /// The result of specifying a relative path is not specified.
  ///
  /// \sa userManualLocation, userManualLocation()
  static void setUserManualLocation(const QString&);

  /// Create standard help menu actions.
  ///
  /// This creates the specified standard actions in the application's Help
  /// menu. This includes activation slots for the actions, i.e. the actions
  /// will function with no additional setup required.
  static void setupHelpMenu(QMenu*, HelpMenuEntries = FullHelpMenu);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgApplication)

private:
  QTE_DECLARE_PRIVATE(vgApplication)
  Q_DISABLE_COPY(vgApplication)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(vgApplication::HelpMenuEntries)

#endif
