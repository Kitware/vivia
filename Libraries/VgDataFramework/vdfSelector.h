// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfSelector_h
#define __vdfSelector_h

#include <qtGlobal.h>

#include <vgExport.h>

#include <QObject>

class vdfSelectorType;

/// Selector type handle.
///
/// This class is a wrapper over a pointer to a \c const QMetaObject. Besides
/// providing a convenient and informative type name, it is required for
/// correct generation of the Shiboken Python bindings, as the container
/// wrapping otherwise cannot wrap a container which contains \c const
/// pointers.
class VG_DATA_FRAMEWORK_EXPORT vdfSelectorType
{
public:
  /*implicit*/ vdfSelectorType(const QMetaObject* = 0);
  ~vdfSelectorType();

  vdfSelectorType& operator=(const QMetaObject*);

  operator const QMetaObject*() const;

  const QMetaObject& operator*() const;
  const QMetaObject& operator->() const;

protected:
  const QMetaObject* d_ptr;
};

/// Abstract base class for selectors.
///
/// This class defines a basic interface for selectors. Since most of the
/// meaningful interface for a selector is dependent on specific data types on
/// which the selector operates, the main purpose of this interface is to
/// ensure a mechanism for copying selectors.
///
/// \sa \ref vdfSelectors
class VG_DATA_FRAMEWORK_EXPORT vdfSelector : protected QObject
{
  Q_OBJECT

public:
  vdfSelector();
  ~vdfSelector();

  /// Create a functionally equivalent copy of this selector.
  ///
  /// This method shall return a new vdfSelector having the same derived class
  /// as this instance, and the same internal data, such that the new selector
  /// will perform the same selection as the existing instance.
  ///
  /// This is used to implement copy-on-write semantics for vdfSelectorSet,
  /// e.g. when a consumer wishes to change a selector that is shared with a
  /// pending asynchronous update request.
  virtual vdfSelector* clone() const = 0;

  /// Get the type of this selector.
  vdfSelectorType type() const;

private:
  QTE_DISABLE_COPY(vdfSelector)
};

extern VG_DATA_FRAMEWORK_EXPORT uint qHash(const vdfSelectorType&);

#endif
