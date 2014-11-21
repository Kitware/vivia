/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsSourceFactory_h
#define __vsSourceFactory_h

#include <vgExport.h>

#include "vsSimpleSourceFactory.h"

class QUrl;
class QWidget;

/// Standard source factory.
///
/// This class implements a full source factory, with support for
/// initialization. These are closer to "true" factories, in that they require
/// no additional information to create, and can be initialized later to set up
/// the actual sources.
class VSP_DATA_EXPORT vsSourceFactory : public vsSimpleSourceFactory
{
public:
  virtual ~vsSourceFactory();

  /// Request interactive factory initialization.
  ///
  /// This method attempts to interactively initialize the source factory,
  /// usually by prompting the user to select a source (e.g. a file or URI).
  /// The factory should use \p dialogParent as the window parent for any
  /// dialogs it creates.
  ///
  /// \return \c true if the factory was initialized, \c false otherwise.
  virtual bool initialize(QWidget* dialogParent) = 0;

  /// Request factory initialization from URI.
  ///
  /// This method attempts to initialize the source factory from the specified
  /// URI in a non-interactive manner. This is typically used to initialize a
  /// factory from a source specified via the command line.
  ///
  /// \return \c true if the factory was initialized, \c false otherwise.
  virtual bool initialize(const QUrl& uri) = 0;

protected:
  vsSourceFactory();

  /// Display a warning.
  ///
  /// This method displays a warning, using a dialog box in interactive mode
  /// (if \p dialogParent is non-null), and ::qWarning otherwise.
  ///
  /// \param dialogTitle Title to give the dialog box, if a dialog box is used.
  /// \param message Warning message to display.
  void warn(QWidget* dialogParent, const QString& dialogTitle,
            const QString& message);

private:
  QTE_DISABLE_COPY(vsSourceFactory)
};

typedef QSharedPointer<vsSourceFactory> vsSourceFactoryPtr;

#endif
