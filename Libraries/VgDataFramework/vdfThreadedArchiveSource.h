// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfThreadedArchiveSource_h
#define __vdfThreadedArchiveSource_h

#include "vdfDataSource.h"

class QUrl;

class vdfThreadedArchiveSourcePrivate;

/// Base class for asynchronous archive data sources.
///
/// This class provides a utility base class for archive data sources that
/// facilitates non-blocking archive access by providing a separate thread in
/// which the archive processing is executed, via the pure virtual
/// processArchive() method.
///
/// Additionally, the class provides default status management and a default
/// mechanism() which are appropriate for most archive data sources.
class VG_DATA_FRAMEWORK_EXPORT vdfThreadedArchiveSource : public vdfDataSource
{
public:
  /// Construct archive source using specified URI.
  explicit vdfThreadedArchiveSource(const QUrl& uri, QObject* parent);
  virtual ~vdfThreadedArchiveSource();

  /// \copydoc vdfDataSource::type
  virtual QString type() const QTE_OVERRIDE;

  /// \copybrief vdfDataSource::mechanism
  ///
  /// This returns an enumeration value describing, in very broad terms, the
  /// means via which the data source's data originates. The default
  /// implementation returns vdfDataSource::Archive.
  virtual Mechanism mechanism() const QTE_OVERRIDE;

  /// \copydoc vdfDataSource::start
  virtual void start() QTE_OVERRIDE;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vdfThreadedArchiveSource)

  /// Execute data source logic.
  ///
  /// This method is called from a separate thread once the archive started.
  /// The default implementation executes standard status changes and calls
  /// processArchive(). Subclasses may override this in order to modify the
  /// default status handling.
  virtual void run();

  /// Process archive.
  ///
  /// This method is called from a separate thread once the archive is started
  /// in order to process the specified archive. This is a pure virtual which
  /// must be implemented by subclasses.
  ///
  /// \param uri URI which identifies the archive to be processed. The value
  ///            passed to this method is the return value of the uri() method.
  virtual bool processArchive(const QUrl& uri) = 0;

  /// Get archive URI.
  ///
  /// \return The archive URI which was passed to the constructor.
  virtual QUrl uri() const;

private:
  QTE_DECLARE_PRIVATE(vdfThreadedArchiveSource)
  QTE_DISABLE_COPY(vdfThreadedArchiveSource)
};

#endif
