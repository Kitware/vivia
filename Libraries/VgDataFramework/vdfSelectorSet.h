// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfSelectorSet_h
#define __vdfSelectorSet_h

#include <qtGlobal.h>

#include <vgExport.h>

#include <QSharedDataPointer>

template <typename T> class QList;

class vdfSelector;
class vdfSelectorType;

class vdfSelectorSetData;

/// Collection of selectors
///
/// This class provides a container for passing and managing selectors, which
/// are used to request data from nodes.
///
/// \sa \ref vdfSelectors
class VG_DATA_FRAMEWORK_EXPORT vdfSelectorSet
{
public:
  vdfSelectorSet();
  ~vdfSelectorSet();

  vdfSelectorSet(const vdfSelectorSet&);
  vdfSelectorSet& operator=(const vdfSelectorSet&);

  /// Add a selector to this set.
  ///
  /// This method adds the specified selector to the selector set. If the set
  /// already contains a selector of the same type, it is replaced.
  ///
  /// The set takes ownership of the selector. The caller should consider the
  /// selector to be invalid after it is inserted and should not attempt to
  /// use it again.
  void insert(const vdfSelector* selector);

  /// Remove any selector of the specified type.
  void remove(const vdfSelectorType&);

  /// Get selectors in this set.
  QList<const vdfSelector*> selectors() const;

protected:
  QTE_DECLARE_SHARED_PTR(vdfSelectorSet)

private:
  QTE_DECLARE_SHARED(vdfSelectorSet)
};

#endif
