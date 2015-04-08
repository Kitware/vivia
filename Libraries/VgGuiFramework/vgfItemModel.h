/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgfItemModel_h
#define __vgfItemModel_h

#include <qtGlobal.h>

#include <vgExport.h>

#include <QSortFilterProxyModel>

class vgfItemModelPrivate;

/// Abstract implementation of an item model.
///
/// This class provides a base class for implementing item models from a
/// generic data model. It provides some common functionality for translating
/// low-level data types into data suitable for presentation, as well as common
/// handling for handling and manipulating item visibility states.
class VG_GUI_FRAMEWORK_EXPORT vgfItemModel : public QSortFilterProxyModel
{
  Q_OBJECT

  /// This property holds whether hidden items are shown in the item view.
  ///
  /// If this property is \c true, all items are by default shown in the item
  /// view. If this property is \c false, items that would not be visible in a
  /// scene view (item data for vgf::VisibilityRole is \c false) are also
  /// hidden in the item view.
  ///
  /// Derived item views may additionally hide items for other reasons.
  ///
  /// \sa areHiddenItemsShown(), setHiddenItemsShown()
  Q_PROPERTY(bool hiddenItemsShown READ areHiddenItemsShown
                                   WRITE setHiddenItemsShown)

public:
  explicit vgfItemModel(QObject* parent = 0);
  virtual ~vgfItemModel();

  /// Get whether hidden items are shown in the item view.
  /// \sa hiddenItemsShown, setHiddenItemsShown()
  virtual bool areHiddenItemsShown() const;

  /// Get data role mapping for column.
  ///
  /// This returns the data role mapping (i.e. the logical data role) for the
  /// specified column, or -1 if no mapping has been set.
  ///
  /// This is not used internally, but may be used by e.g. views in order to
  /// take appropriate actions when an item index is activated or edited. As
  /// such, subclasses should override this method if necessary to ensure that
  /// a suitable logical role is always returned whenever possible.
  virtual int roleForColumn(int column) const;

  // TODO helpers to get classification text and decoration

  // Reimplemented from QAbstractItemModel
  virtual QVariant data(const QModelIndex& index, int role) const QTE_OVERRIDE;
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const QTE_OVERRIDE;

  // Reimplemented from QSortFilterProxyModel
  virtual void sort(
    int column, Qt::SortOrder order = Qt::AscendingOrder) QTE_OVERRIDE;

public slots:
  /// Set whether hidden items are shown in the item view.
  /// \sa hiddenItemsShown, areHiddenItemsShown()
  virtual void setHiddenItemsShown(bool);

protected slots:
  /// Update sorting when data changes.
  ///
  /// This slot checks if the model has not been sorted, and if so, sorts it.
  /// This is done to work around a bug in QSortFilterProxyModel where items
  /// are not sorted after a data change if all items were initially hidden.
  ///
  /// This slot is called automatically when the proxy model's data changes
  /// (note: \em not the source model's data). Users should not normally need
  /// to call this slot themselves.
  void updateSort();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgfItemModel)

  /// Set data role mapping for column.
  ///
  /// This sets a data role mapping for the specified column. If a mapping
  /// exists for a column, calls to data(const QModelIndex&, int) and
  /// lessThan(const QModelIndex&, const QModelIndex&) will return or use the
  /// data for the specified logical data role.
  ///
  /// If \p role is negative, the mapping for \p column (if one was previously
  /// set) is removed.
  virtual void setRoleForColumn(int column, int role);

  /// Return representation for specified index and data role.
  ///
  /// This method returns presentation-ready data for the specified
  /// presentation role and data role.
  ///
  /// Supported presentation roles are:
  /// \li Qt::DisplayRole
  /// \li Qt::DecorationRole
  /// \li Qt::TextAlignmentRole
  /// \li Qt::ToolTipRole
  virtual QVariant data(const QModelIndex& sourceIndex,
                        int presentationRole, int dataRole) const;

  /// Compare data for two indices.
  ///
  /// This method performs a comparison of the \p role data of two proxy
  /// indices. If \p role is not a supported data role, the result is \c false.
  ///
  /// \return \c true if the left index's data is less than the right index's
  ///         data; otherwise \c false.
  virtual bool lessThan(const QModelIndex& left, const QModelIndex& right,
                        int role) const;

  // Reimplemented from QSortFilterProxyModel
  virtual bool lessThan(
    const QModelIndex& left, const QModelIndex& right) const;

  virtual bool filterAcceptsRow(
    int sourceRow, const QModelIndex& sourceParent) const QTE_OVERRIDE;
  virtual bool filterAcceptsColumn(
    int source_column, const QModelIndex& source_parent) const QTE_OVERRIDE;

private:
  QTE_DECLARE_PRIVATE(vgfItemModel)
  Q_DISABLE_COPY(vgfItemModel)
};

#endif
